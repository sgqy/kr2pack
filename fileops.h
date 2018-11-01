#ifndef __FILEOPS_H__
#define __FILEOPS_H__

#include <stdint.h>
void *fop_map_file_ro (const char *fn, uintptr_t *size);
void fop_unmap_file (void *p, uintptr_t size);

#endif // __FILEOPS_H__
