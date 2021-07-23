CC=gcc
CFLAGS=-Wall

all:
	gcc `sdl2-config --cflags --libs` -Wall main.c -o chip8emu

clean:
	rm -f chip8emu
