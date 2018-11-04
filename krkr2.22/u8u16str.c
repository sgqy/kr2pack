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

int u16u8str(const uint16_t *u16, uint8_t *u8)
{
	int ret = 0;
	while (*u16) {
		if (*u16 < 0xD800 || *u16 > 0xDBFF) {
			int a = ucs2_to_utf8(*u16, u8);
			if (a < 0) {
				return ret;
			}
			u8 += a;
			u16 += 1;
			ret += a;
		} else {
			if (*(u16 + 1) == 0) {
				return ret;
			}
			int a = surrogate_to_utf8(*u16, *(u16 + 1), u8);
			if (a < 0) {
				return ret;
			}
			u8 += a;
			u16 += 2;
			ret += a;
		}
	}
	return ret;
}
