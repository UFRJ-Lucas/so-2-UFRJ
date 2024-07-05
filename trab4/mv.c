#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

// Função que move (ou renomeia) um arquivo de uma localização para outra
int do_move(const char *source, const char *dest)
{
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

// Função principal que processa os argumentos da linha de comando e chama `do_move` conforme necessário
int main(int argc, char *argv[])
{
    // Verifica se o número de argumentos é menor que 3. Se for, imprime a mensagem de uso e sai com falha.
    if (argc < 3)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Move a single file to a destination: %s <source> <destination>\n", argv[0]);
        fprintf(stderr, "  Move multiple files to a destination: %s <source1> <source2> ... <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int is_dest_dir = 0;               // Variável para verificar se o destino é um diretório
    const char *source = argv[1];      // Primeiro argumento é a fonte (arquivo a ser movido)
    const char *dest = argv[argc - 1]; // Último argumento é o destino
    struct stat dest_stat;             // Estrutura para armazenar informações sobre o destino

    // Obtém as informações do destino e verifica se ele é um diretório
    if (stat(dest, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        is_dest_dir = 1;
    }

    // Renomeando um arquivo ou diretório
    if (argc == 3 && !is_dest_dir)
    {
        if (do_move(source, dest) != 0)
        {
            fprintf(stderr, "Failed to move '%s' to '%s'\n", source, dest);
            exit(EXIT_FAILURE);
        }
    }

    // Movendo um ou vários arquivos ou diretórios
    else
    {
        for (int i = 1; i < argc - 1; ++i)
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
            if (do_move(current_source, new_dest) != 0)
            {
                fprintf(stderr, "Failed to move '%s' to '%s'\n", current_source, new_dest);
                exit(EXIT_FAILURE);
            }
        }
    }

    return EXIT_SUCCESS;
}
