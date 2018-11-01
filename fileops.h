#include <stdint.h>
void *fop_map_file_ro (const char *fn, uintptr_t *size);
void fop_unmap_file (void *p, uintptr_t size);