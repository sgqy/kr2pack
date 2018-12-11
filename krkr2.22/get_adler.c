#include <zlib.h>

#include "get_adler.h"

uint32_t get_adler32(const void *buf, const uintptr_t size)
{
	uLong          ret    = 1;
	const uint8_t  cutbit = 31;
	const uint32_t cutsz  = 1 << cutbit;
	uint32_t       count  = size >> cutbit;
	for (uint32_t i = 0; i < count; ++i)
	{
		ret = adler32(ret, buf + i * cutsz, cutsz);
	}
	uintptr_t rsz = size - count * cutsz;
	ret           = adler32(ret, buf + count * cutsz, rsz);
	return (ret & 0xFFFFFFFF);
}
