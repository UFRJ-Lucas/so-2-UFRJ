#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

    /* Verifica se o parametro de tempo foi passado corretamente */
    if (argc == 1 || atol(argv[1]) <= 0) {
        printf("Insira o intervalo de tempo (segundos) entre os logs registrados.\n%s [<n>]\n", argv[0]);
        return 1;
    }

    int n = atol(argv[1]);

    /* Abre o arquivo de log */

    /* Pega os processos ZOMBIEs na maquina */

    printf("Hello World for %d seconds\n", n);
}