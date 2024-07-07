#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>

#define TRUE 1
#define FALSE 0

// Função que move (ou renomeia) um arquivo de uma localização para outra
int do_move(const char *source, const char *dest, int interactive, int overwrite, int verbose, int backup)
{
    struct stat buffer;
    int file_exists = stat(dest, &buffer) == 0; // Verifica se o arquivo de destino já existe
    int backup_created = FALSE;

    if (file_exists)
    {
        if (interactive)
        {
            // Destino já existe, perguntar ao usuário
            char response;
            printf("mv: overwrite '%s'? ", dest);
            response = getchar();
            while (getchar() != '\n'); // Limpar o buffer de entrada
            if (response != 'y' && response != 'Y')
            {
                return 0; // Não sobrescrever, retornar sucesso
            }
        }
        else if (!overwrite)
        {
            // Não permitir sobrescrever arquivo já existente
            return 0; // Não sobrescrever, retornar sucesso
        }

        if (backup)
        {
            // Criar uma cópia de segurança do arquivo de destino
            char backup_filename[strlen(dest) + 2]; // espaço para o nome do arquivo e o sufixo `~`
            sprintf(backup_filename, "%s~", dest);
            if (rename(dest, backup_filename) != 0)
            {
                fprintf(stderr, "Error creating backup of '%s': %s\n", dest, strerror(errno));
                return -1;
            }
            backup_created = TRUE;
        }
    }

    if (rename(source, dest) == 0) // Tenta renomear (ou mover) o arquivo
    {
        if (verbose)
        {   // Exibir detalhes
            if (backup_created) 
            {
                printf("renamed '%s' -> '%s' (backup: '%s~')\n", source, dest, dest);
            }
            else
            {
                printf("renamed '%s' -> '%s'\n", source, dest);
            }
        }
        return 0; // Retorna 0 se bem-sucedido
    }
    else
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1; // Retorna -1 em caso de erro
    }
}

void usage()
{
    // Função para imprimir a mensagem de uso e sair com falha
    fprintf(stderr, "Usage: ./mv [OPTION]... SOURCE DEST\n");
    fprintf(stderr, "Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\n");
    fprintf(stderr, "  -b                           make a backup of each existing destination file\n");
    fprintf(stderr, "  -f                           do not prompt before overwriting\n");
    fprintf(stderr, "  -i                           prompt before overwrite\n");
    fprintf(stderr, "  -n                           do not overwrite an existing file\n");
    fprintf(stderr, "If you specify more than one of -i, -f, -n, only the final one takes effect.\n");
    fprintf(stderr, "  -v                           explain what is being done\n");
    fprintf(stderr, "  -h                           display this help and exit\n");
    exit(EXIT_FAILURE);
}

// Função principal que processa os argumentos da linha de comando e chama `do_move` conforme necessário
int main(int argc, char *argv[])
{
    // Variáveis para controlar as opções de linha de comando
    int overwrite = TRUE;
    int interactive = FALSE;
    int verbose = FALSE;
    int backup = FALSE;
    int opt;

    // Processar as opções de linha de comando
    while ((opt = getopt(argc, argv, "ifnvbh")) != -1) {
        switch (opt) {
            case 'f':
                overwrite = TRUE;
                interactive = FALSE;
                break;
            case 'i':
                overwrite = FALSE;
                interactive = TRUE;
                break;
            case 'n':
                overwrite = FALSE;
                interactive = FALSE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 'b':
                backup = TRUE;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Try 'mv -h' for more information.\n");
                exit(EXIT_FAILURE);
        }
    }

    // Ajusta os argumentos restantes, necessário pois senão o getopt deixa os argumentos de opção no meio da lista de arquivos
    argc -= optind;
    argv += optind;

    // Verifica se o número de argumentos é menor que 2. Se for, imprime a mensagem de uso e sai com falha.
    if (argc < 2)
    {
        if (argc == 1)
        {
            fprintf(stderr, "mv: missing destination file operand after %s\n", argv[0]);
            fprintf(stderr, "Try 'mv -h' for more information.\n");
        }
        else
        {
            fprintf(stderr, "mv: missing file operand\n");
            fprintf(stderr, "Try 'mv -h' for more information.\n");
        }
    }

    int is_dest_dir = 0;               // Variável para verificar se o destino é um diretório
    const char *source = argv[0];      // Primeiro argumento é a fonte (arquivo a ser movido)
    const char *dest = argv[argc - 1]; // Último argumento é o destino
    struct stat dest_stat;             // Estrutura para armazenar informações sobre o destino

    // Obtém as informações do destino e verifica se ele é um diretório
    if (stat(dest, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        is_dest_dir = 1;
    }

    // Renomeando um arquivo ou diretório
    if (argc == 2 && !is_dest_dir)
    {
        if (do_move(source, dest, interactive, overwrite, verbose, backup) != 0)
        {
            fprintf(stderr, "Failed to move '%s' to '%s'\n", source, dest);
            exit(EXIT_FAILURE);
        }
    }

    // Movendo um ou vários arquivos ou diretórios
    else
    {
        for (int i = 0; i < argc - 1; ++i)
        {
            const char *current_source = argv[i];          // Fonte atual
            char *filename = strrchr(current_source, '/'); // encontra a última ocorrência de `/` no caminho
            if (filename == NULL)
            {
                filename = (char *)current_source; // Se não houver `/`, `filename` é o próprio `current_source`
            }
            else
            {
                filename++; // Se houver `/`, `filename` aponta para o caractere imediatamente após a última `/`, que é o nome do arquivo ou diretório
            }

            // new_dest é alocado para ter espaço suficiente para o caminho completo (dest) mais o nome do arquivo ou diretório (filename) e um `/` adicional
            char new_dest[strlen(dest) + strlen(filename) + 2];

            // constrói o caminho completo concatenando o diretório de destino (dest), uma `/` e o nome do arquivo ou diretório (filename)
            sprintf(new_dest, "%s/%s", dest, filename);
            if (do_move(current_source, new_dest, interactive, overwrite, verbose, backup) != 0)
            {
                fprintf(stderr, "Failed to move '%s' to '%s'\n", current_source, new_dest);
                exit(EXIT_FAILURE);
            }
        }
    }

    return EXIT_SUCCESS;
}
