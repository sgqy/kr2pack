#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <ftw.h>
#include <unistd.h>
#include <fcntl.h>

#include <zlib.h>

#include "fileops.h"
#include "get_adler.h"
#include "u8u16str.h"
#include "zip_and_write.h"

#include "xp3.h"

// file list to save info
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

// process pack and every file

int out_fd = 0;

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
	node->z_size = zip_and_write(out_fd, node->solid_buf, node->size);

	add(node);
	printf("[+] %s\n", fpath);

	return 0;
}



int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("%s <dir>\n    output: <dir>.xp3\n", argv[0]);
		return 1;
	}

	// copy a package file name
	char out_fn[300];
	for(int i = 0; i < strlen(argv[1]); ++i)
	{
		if(strchr("./\\?!@#$%%^&*()-=+`~|\"\';:<>", argv[1][i]) != 0)
		{
			out_fn[i] = '_';
		}
		else
		{
			out_fn[i] = argv[1][i];
		}
	}
	strcat(out_fn, ".xp3");

	// create file
	out_fd = open(out_fn, O_CREAT | O_TRUNC | O_RDWR, 0664);
	if(out_fd < 0)
	{
		printf("[-] failed to create %s\n", out_fn);
		return 1;
	}

	// write header
	struct xp3_hdr hdr;
	memcpy(hdr.magic, "\x58\x50\x33\x0D\x0A\x20\x0A\x1A\x8B\x67\x01", 0xb);
	write(out_fd, &hdr, sizeof(hdr));

	// compress the files
	nftw(argv[1], fworker, 20, 0);

	// generate file table
	

	// clean up
	close(out_fd);

	return 0;
}
