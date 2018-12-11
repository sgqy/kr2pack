#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "fileops.h"
#include "get_adler.h"

#include "xp3.h"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("%s <xp3>\n", argv[0]);
		return -1;
	}

	uintptr_t psz;
	char *    p = fop_map_file_ro(argv[1], &psz);

	if (!p)
	{
		printf("[-] error %s\n", argv[1]);
		return 12;
	}

	const char *const              set = p;
	const struct xp3_hdr *const    hdr = (struct xp3_hdr *)p;
	const struct xp3_ft_hdr *const ft  = (struct xp3_ft_hdr *)(p + hdr->ft_offset);

	if (ft->is_zipped != 1)
	{
		printf("[-] unsupported %s\n", argv[1]);
		exit(11);
	}

	uLongf ul  = ft->plain_sz + 262144;
	Bytef *un  = malloc(ul);
	int    rel = uncompress(un, &ul, (Bytef *)(ft + 1), ft->zip_sz);
	if (rel != Z_OK)
	{
		printf("[-] file table zlib: %d\n", rel);
		exit(10);
	}

	for (char *t = (char *)un; t < (char *)un + ft->plain_sz; /**/)
	{
		struct entry_hdr *ehdr  = t;
		t                       = ehdr + 1;
		struct entry_info *info = t;
		t += 0xc + info->this_sz;
		struct entry_segm *segm = t;
		t += 0xc + segm->this_sz;
		struct entry_adlr *adlr = t;
		t += 0xc + adlr->this_sz;

		uint16_t u16path[300] = {0};
		memcpy(u16path, info->path, info->path_sz * 2);
		uint8_t path[300] = {0};
		u16u8str(u16path, path);

		if (info->zip_sz != segm->zip_sz || info->plain_sz != segm->plain_sz)
		{
			printf("[-] info err %s\n", path);
			exit(9);
		}

		uLongf dsz  = segm->plain_sz + 262144;
		Bytef *data = malloc(dsz);
		int    r    = uncompress(data, &dsz, set + segm->offset, segm->zip_sz);
		if (r != Z_OK)
		{
			printf("[-] zip %s, zlib: %d\n", path, r);
			exit(8);
		}
		if (dsz != segm->plain_sz)
		{
			printf("[-] %ld but %ld size %s\n", segm->plain_sz, dsz, path);
			exit(7);
		}
		uint32_t hash = get_adler32(data, dsz);
		if (hash != adlr->hash)
		{
			printf("[-] [%08x] but [%08x] hash %s\n", adlr->hash, hash, path);
			exit(6);
		}
		printf("[+] [%08x]->[%08x] %s\n", adlr->hash, hash, path);
		free(data);
	}

	exit(0);
}
