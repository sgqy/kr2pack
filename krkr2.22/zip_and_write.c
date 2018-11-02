
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <zlib.h>

#include "zip_and_write.h"

uintptr_t zip_and_write(int fd, const void *buf, const uintptr_t size)
{
	char *    zip   = malloc(size);
	uintptr_t zsize = size;
	int       res   = compress2((Bytef *)zip, &zsize, buf, size, 9);
	if (res != Z_OK) {
		printf("[-] zlib fail: %d\n", res);
		exit(1);
	}
	write(fd, zip, zsize);
	free(zip);
	return zsize;
}
