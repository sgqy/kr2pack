#ifndef __GET_ADLER_H__
#define __GET_ADLER_H__

#include <stdint.h>

// adler32 64bit size extention
uint32_t get_adler32(const void *buf, const uintptr_t size);

#endif // __GET_ADLER_H__
