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
int do_move(const char *source, const char *dest, int interactive, int overwrite)
{
    struct stat buffer;
    int file_exists = stat(dest, &buffer) == 0; // Verifica se o arquivo de destino já existe

    if (file_exists){
        if (interactive) {
            // Destino já existe, perguntar ao usuário
            char response;
            printf("overwrite '%s'? (y/n [n]) ", dest);
            response = getchar();
            while (getchar() != '\n'); // Limpar o buffer de entrada
            if (response != 'y' && response != 'Y') {
                return 0; // Não sobrescrever, retornar sucesso
            }
        } else if (!overwrite) {
            // Não permitir sobrescrever arquivo já existente
            return 0; // Não sobrescrever, retornar sucesso
        }
    }

    if (rename(source, dest) == 0) // Tenta renomear (ou mover) o arquivo
    {
        return 0; // Retorna 0 se bem-sucedido
    }
    else
    {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1; // Retorna -1 em caso de erro
    }
}

void usage(char *func_name)
{
    // Função para imprimir a mensagem de uso e sair com falha
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  Move a single file to a destination: %s <source> <destination>\n", func_name);
    fprintf(stderr, "  Move multiple files to a destination: %s <source1> <source2> ... <destination>\n", func_name);
    exit(EXIT_FAILURE);
}

// Função principal que processa os argumentos da linha de comando e chama `do_move` conforme necessário
int main(int argc, char *argv[])
{
    // Variáveis para controlar as opções de linha de comando
    int overwrite = TRUE;
    int interactive = FALSE;
    int opt;

    // Processar as opções de linha de comando
    while ((opt = getopt(argc, argv, "ifn")) != -1) {
        switch (opt) {
            case 'f':
                overwrite = TRUE;
                break;
            case 'i':
                interactive = TRUE;
                break;
            case 'n':
                overwrite = FALSE;
                break;
            default:
                usage(argv[0]);
        }
    }

    // Ajusta os argumentos restantes, necessário pois senão o getopt deixa os argumentos de opção no meio da lista de arquivos
    argc -= optind;
    argv += optind;

    // Verifica se o número de argumentos é menor que 3. Se for, imprime a mensagem de uso e sai com falha.
    if (argc < 2)
    {
        usage(argv[0]);
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
        if (do_move(source, dest, interactive, overwrite) != 0)
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

            //constrói o caminho completo concatenando o diretório de destino (dest), uma `/` e o nome do arquivo ou diretório (filename)
            sprintf(new_dest, "%s/%s", dest, filename);
            if (do_move(current_source, new_dest, interactive, overwrite) != 0)
            {
                fprintf(stderr, "Failed to move '%s' to '%s'\n", current_source, new_dest);
                exit(EXIT_FAILURE);
            }
        }
    }

    return EXIT_SUCCESS;
}
