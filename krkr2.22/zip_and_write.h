#ifndef __ZIPnWR_H__
#define __ZIPnWR_H__

#include <stdint.h>
uintptr_t zip_and_write(int fd, const void *buf, const uintptr_t size);

#endif // __ZIPnWR_H__