#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int do_move(const char *source, const char *dest) {
    if (rename(source, dest) == 0) { // Move (and rename) the file to another
        return 0; // Success
    } else {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return -1; // Error
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        // Moving only one file to the destination for now
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *source = argv[1];
    const char *dest = argv[2];

    if (do_move(source, dest) != 0) {
        fprintf(stderr, "Failed to move '%s' to '%s'\n", source, dest);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}