CC=gcc
CFLAGS=-Wall

all:
	gcc -Wall main.c -o chip8emu

clean:
	rm -f chip8emu
