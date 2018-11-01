// 64-bit only
// sizeof(long): 8 byte

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "fileops.h"

void *fop_map_file_ro (const char *fn, uintptr_t *sz)
{
    int fd = 0;
    uintptr_t size = 0;
    void *p = 0;

    fd = open (fn, O_RDONLY);

    if (fd < 0)
    {
        goto end;
    }

    size = (uintptr_t)lseek (fd, 0, SEEK_END);

    if (size < 0)
    {
        size = 0;
        goto end;
    }

    p = mmap (0, size, PROT_READ, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED)
    {
        p = 0;
        goto end;
    }

end:

    if (fd >= 0)
    {
        close (fd);
    }

    *sz = size;
    return p;
}

void fop_unmap_file (void *p, uintptr_t size)
{
    munmap (p, size);
}