#include "unicode.h"

#include "u8u16str.h"

int u8u16str(const uint8_t *u8, uint16_t *u16)
{
	int ret = 0;
	while (*u8) {
		int32_t ucs2 = utf8_to_ucs2(u8, &u8);
		if (ucs2 > 0xffff) {
			int32_t hi, lo;
			unicode_to_surrogates(ucs2, &hi, &lo);
			u16[ret++] = hi;
			u16[ret++] = lo;
		} else {
			u16[ret++] = ucs2;
		}
	}
	return ret;
}
