#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define TRUE 1
#define BUFFER_SIZE 100
#define SEPARATOR "========================\n"

void finish_log(int sig) {
    /* Finaliza o arquivo de log */
    FILE *log = fopen("log.txt", "a");
    if (log == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo de log.\n");
        exit(1);
    }

    fprintf(log, "Log finalizado.\n");

    exit(0);
}

int main(int argc, char **argv) {

    char buffer[BUFFER_SIZE];
    int n;

    /* Verifica se o parametro de tempo foi passado corretamente */
    if (argc == 1 || (n = atol(argv[1])) <= 0) {
        printf("Insira o intervalo de tempo (segundos) entre os logs registrados.\n%s [<n>]\n", argv[0]);
        return 1;
    }

    /* Abre o arquivo de log */
    FILE *log = fopen("log.txt", "w");
    if (log == NULL) {
        printf("Erro ao abrir o arquivo de log.\n");
        return 1;
    }

    /* Finalização com sinal SIGTERM */
    signal(SIGTERM, finish_log);

    /* Imprime header no arquivo de log */
    fprintf(log, "STAT PPID PID COMMAND\n");
    fprintf(log, SEPARATOR);

    while(TRUE) {

        /* Limpa o buffer */
        buffer[0] = '\0';

        /* Abre o pipe para o comando ps */
        FILE *pipe = popen("ps -eo stat,ppid,pid,comm | grep '^Z'", "r");
        if (pipe == NULL) {
            printf("Erro ao abrir o pipe.\n");
            return 1;
        }
        
        /* Escreve no arquivo de log */
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            fprintf(log, "%s", buffer);
        }

        /* Fecha o pipe */
        pclose(pipe);

        fprintf(log, SEPARATOR);

        /* Força a escrita no arquivo durante o laço */
        fflush(log); 

        /* Espera pelo intervalo definido */
        sleep(n);
    }
}