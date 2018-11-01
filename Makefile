CFLAGS = -O0 -g -rdynamic -D_XOPEN_SOURCE=501

kr2packer: main.o fileops.o get_adler.o u8u16str.o unicode.o
	gcc -o $@ $^ -lz

clean:
	rm -f *.o kr2packer
