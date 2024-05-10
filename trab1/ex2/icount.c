#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

/* variaveis globais de contador e tipo */
int inode_count;
int inode_type;

void increment_inode_count() {
    inode_count++;
}

void reset_inode_count() {
    inode_count = 0;
}

void set_inode_type(int type) {
    inode_type = type;
}

int walk_dir(const char *path, void (*func) (const char *)) {
    DIR *dirp;
    struct dirent *dp;
    char *p, *full_path;
    int len;

    /* abre o diretório */
    if ((dirp = opendir(path)) == NULL) return -1;
    len = strlen(path);

    /* aloca uma área na qual, garantidamente, o caminho caberá */
    if ((full_path = malloc(len + NAME_MAX + 2)) == NULL) {
        closedir(dirp);
        return -1;
    }
    
    /* copia o prefixo e acrescenta a ‘/’ ao final */
    memcpy(full_path, path, len);
    p = full_path + len; *p++ = '/'; /* deixa “p” no lugar certo! */
    
    while ((dp = readdir(dirp)) != NULL) {
        /* ignora as entradas “.” e “..” */
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

        strcpy(p, dp->d_name);

        /* "full_path" armazena o caminho */
        (*func) (full_path);
    }

    free(full_path);

    closedir(dirp);

    return 0;
}

void count_inodes(const char *fpath) {
    struct stat sb;

    if (stat(fpath, &sb) == -1) {
        printf("ERROR: %s\n", strerror(errno));
        return;
    } else if ((sb.st_mode & S_IFMT) == inode_type) {
        increment_inode_count();
    }
}

void show_arg_count(const char *argpath) {
    if (walk_dir(argpath, count_inodes) == -1) {
        printf("ERROR: %s\n", strerror(errno));
        return;
    }
    printf("Total de inodes em '%s': %d\n", argpath, inode_count);
    reset_inode_count();
}

int main(int argc, char *argv[]) {
    int opt;
    int mutex = 0; /* para verificar que as opcoes sao mutamente exclusivas */
    char *path;

    /* inicializa os valores padrao das variaveis globais */
    reset_inode_count();
    set_inode_type(S_IFREG);

    /* verifica qual opcao foi passada */
    while ((opt = getopt(argc, argv, "rdlbc")) != -1) {
        if (mutex) {
            printf("Opções são mutuamente exclusivas\n");
            printf("Uso: %s [-r | -d | -l | -b | -c] [<dir> ...]\n", argv[0]);
            return -1;
        }
        mutex = 1;
        switch (opt) {
            case 'r':
                break;
            case 'd':
                set_inode_type(S_IFDIR);
                break;
            case 'l':
                set_inode_type(S_IFLNK);
                break;
            case 'b':
                set_inode_type(S_IFBLK);
                break;
            case 'c':
                set_inode_type(S_IFCHR);
                break;
            default:
                printf("Uso: %s [-r | -d | -l | -b | -c] [<dir> ...]\n", argv[0]);
                return -1;
        }
    }

    /* verifica os argumentos passados. Se nenhum foi passado, usa o diretório atual */
    if (optind == argc) {
        path = ".";
        show_arg_count(path);
    } else {
        while (optind < argc) {
            path = argv[optind++];
            show_arg_count(path);
        }
    }
    
    return 0;
}