all:
	./make.zynq stl2raw

CFLAGS+=-O3
stl2raw.x86: stl2raw.c
	cc stl2raw.c -o stl2raw.x86


