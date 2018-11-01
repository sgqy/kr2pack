#CFLAGS = -O0 -g -Wall -rdynamic -D_XOPEN_SOURCE=500
CFLAGS = -O2 -Wall -D_XOPEN_SOURCE=500

kr2packer: main.o fileops.o get_adler.o u8u16str.o unicode.o zip_and_write.o
	gcc -o $@ $^ -lz

clean:
	rm -f *.o kr2packer
