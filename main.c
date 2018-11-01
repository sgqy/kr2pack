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
	// the order is inversed, do not write package in fworker
	node->next = last;
	last = node;
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
	const char *omit = strchr(fpath, '/') + 1;
	node->u16path_len = u8u16str(omit, node->u16path);
	node->size = sb->st_size;
	node->solid_buf = fop_map_file_ro_with_size(fpath, sb->st_size);
	node->adlr = get_adler32(node->solid_buf, node->size);
	node->z_size = node->size; // init

	add(node);
	printf("[+] %s\n", omit);

	return 0;
}

void mkpack()
{
	// 1. get file table offset
	uint64_t ft_loc = sizeof(struct xp3_hdr);
	int count = 0;
	struct file_list *n = last;
	while(n)
	{
		++count;
		n->z_size = zip_and_write(out_fd, n->solid_buf, n->size);
		ft_loc += n->z_size;		
		n = n->next;
	}

	// 2. prepare ft buff
	char *p = malloc(sizeof(struct xp3_ft_hdr) + count * (
		sizeof(struct entry_hdr) + sizeof(struct entry_info)
		+ sizeof(struct entry_segm) + sizeof(struct entry_adlr)));
	struct xp3_ft_hdr *ft = (struct xp3_ft_hdr*)p;
	char *beg = p = (char *)(ft + 1);

	n = last;
	uint64_t g_off = sizeof(struct xp3_hdr);
	while(n)
	{
		struct entry_adlr adlr = {
			.magic = mag_adlr,
			.this_sz = 0x04,
			.hash = n->adlr,
		};
		struct entry_segm segm = {
			.magic = mag_segm,
			.this_sz = 0x1c,
			.is_zipped = 1,
			.plain_sz = n->size,
			.zip_sz = n->z_size,
			.offset = g_off,
		};
		struct entry_info info = {
			.magic = mag_info,
			.is_enc = 0,
			.plain_sz = n->size,
			.zip_sz = n->z_size,
			.path_sz = n->u16path_len,
		};
		memcpy(info.path, n->u16path, n->u16path_len * 2);
		info.this_sz = 0x16 + info.path_sz * 2;
		struct entry_hdr file = {
			.magic = mag_file,
			.entry_sz = adlr.this_sz + segm.this_sz + info.this_sz + 3 * 0xc,
		};

		memcpy(p, &file, sizeof(file));
		p += sizeof(file);
		memcpy(p, &info, info.this_sz + 0xc);
		p += info.this_sz + 0xc;
		memcpy(p, &segm, segm.this_sz + 0xc);
		p += segm.this_sz + 0xc;
		memcpy(p, &adlr, adlr.this_sz + 0xc);
		p += adlr.this_sz + 0xc;

		g_off += n->z_size;
		n = n->next;
	}
	
	ft->is_zipped = 1;
	ft->plain_sz = p - beg;
	write(out_fd, ft, sizeof(struct xp3_ft_hdr));

	ft->zip_sz = zip_and_write(out_fd, ft + 1, ft->plain_sz);

	lseek(out_fd, 0xb, SEEK_SET);
	write(out_fd, &ft_loc, sizeof(ft_loc));

	lseek(out_fd, ft_loc + 1, SEEK_SET);
	write(out_fd, &(ft->zip_sz), sizeof(ft->zip_sz));

	free(ft);
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

	// get info of files
	nftw(argv[1], fworker, 20, 0);

	// make the package
	mkpack();

	// clean up
	close(out_fd);

	return 0;
}
