#ifndef __XP3_H__
#define __XP3_H__

#include <stdint.h>

#pragma pack(1) // gcc support

// xp3 archive

struct xp3_hdr
{
	uint8_t  magic[0xb]; // 5850330D0A200A1A8B6701
	uint64_t ft_offset;  // file table absolute offset
};

struct xp3_ft_hdr
{
	uint8_t  is_zipped; // 0 or 1
	uint64_t zip_sz;
	uint64_t plain_sz;
};

// xp3 file table entries

#define mag_file (*(uint32_t *)"File")
#define mag_info (*(uint32_t *)"info")
#define mag_segm (*(uint32_t *)"segm")
#define mag_adlr (*(uint32_t *)"adlr")

struct entry_hdr
{
	uint32_t magic;    // "File"
	uint64_t entry_sz; // info.sz + segm.sz + adlr.sz
};

struct entry_info
{
	uint32_t magic;   // "info"
	uint64_t this_sz; // size of below member
	uint32_t is_enc;  // 0(no) or 1<<31(yes)
	uint64_t plain_sz;
	uint64_t zip_sz;
	uint16_t path_sz;
	uint16_t path[256]; // UTF-16 LE, path separator: '/'
};

struct entry_segm
{
	uint32_t magic;     // "segm"
	uint64_t this_sz;   // size of below member: 0x1c
	uint32_t is_zipped; // 0 or 1
	uint64_t offset;    // absolute offset
	uint64_t plain_sz;
	uint64_t zip_sz;
};

struct entry_adlr
{
	uint32_t magic;   // "adlr"
	uint64_t this_sz; // size of below member: 0x04
	uint32_t hash;
};

#pragma pack()

#endif //  __XP3_H__
