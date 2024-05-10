#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int isopen(int fd) {
	struct stat sb;

	//printf("ERRNO: %s\n", strerror(errno));

	int value = fstat(fd, &sb);
	//printf("VALUE: %d\n", value);

	if (value == -1) {
		//printf("ERRNO: %s\n", strerror(errno));

		if (errno == EBADF) return 0;
	}

	return 1;
}
