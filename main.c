#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <ftw.h>

#include <zlib.h>

#include "fileops.h"
#include "get_adler.h"
#include "u8u16str.h"

#include "xp3.h"

struct file_list
{
	char path[256];
	uint16_t u16path_len;
	uint16_t u16path[256];
	uintptr_t size;
	char *solid_buf;
	uint32_t adlr;
	uintptr_t z_size;
	struct file_list *next;
};

struct file_list *last = 0;

void add(struct file_list *node)
{
	node->next = last;
	last = node;
}

void pnodes()
{
	struct file_list *n = last;
	while(n)
	{
		printf("[+] [%p] [%08x] %ld\t%d\t%s\n",
			n->solid_buf, n->adlr, n->size, n->u16path_len, n->path);
		n = n->next;
	}
}

int fworker(const char *fpath, const struct stat *sb,int typeflag, struct FTW *ftwbuf)
{
	if(typeflag != FTW_F)
	{
		return 0;
	}
	struct file_list *node = malloc(sizeof(struct file_list));
	strcpy(node->path, fpath);
	node->u16path_len = u8u16str(node->path, node->u16path);
	node->size = sb->st_size;
	node->solid_buf = fop_map_file_ro_with_size(fpath, sb->st_size);
	node->adlr = get_adler32(node->solid_buf, node->size);
	node->z_size = 0;

	add(node);
	//printf("[+] %s\n", fpath);

	return 0;
}

int main(int argc, char **argv)
{

	nftw(argv[1], fworker, 20, 0);

	pnodes();

	return 0;
}
