#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define _8B unsigned char
#define _16B unsigned short

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320


SDL_Window *window;


bool init_graphics(){
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("SDL Failed to initalise: %s\n", SDL_GetError());
		return false;	
	}
	
	window = SDL_CreateWindow("chip8emu",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				WINDOW_WIDTH,
				WINDOW_HEIGHT,
				0);

	if(window == NULL){
		printf("SDL window failed to initalise: %s\n", SDL_GetError());
		return false;
	}
	
	

	return true;
}

void destroy_graphics(){
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main()
{
	_8B	memory[4096];	 	
	_16B 	opcode;			
	_8B	V[16]; 		// registers, only 0x0 - 0xE are used, 0xF is used as a flag
	_16B 	I; 		// address register
	_16B 	PC;
	
	_16B	stack[16];
	_8B 	SP;		// stack pointer, points to top of stack
	
	_8B 	gbuff[64*32]; 	// graphical buffer needs to be interpreted by SDL	
	
	_8B 	fontset[80] =
	{ 
  	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  	0x20, 0x60, 0x20, 0x20, 0x70, // 1
  	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
 	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
 	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
 	 0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	}; 
	
	// move fontset into interpreter part of the memory
	for(int i = 0; i < 80; i++){
		memory[i] = fontset[i];	
	}
 
	if(!init_graphics())
		return 1;
	int c;	
	c = getchar();	

	destroy_graphics();	
	return 0;
}
