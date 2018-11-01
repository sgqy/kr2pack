CFLAGS = -O0 -g -rdynamic

kr2packer: main.o fileops.o get_adler.o
	gcc -o $@ $^ -lz

clean:
	rm -f *.o kr2packer
