#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define _8B unsigned char
#define _16B unsigned short

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 320


SDL_Window *window;


// inits graphics, returs true if it was succesful
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
	_16B 	opcode;		// opcode that is currently being executed
	_8B	V[16]; 		// registers, only 0x0 - 0xE are used, 0xF is used as a flag
	_16B 	I; 		// memory register
	_16B 	PC;
	
	_16B	stack[16];
	_8B 	SP;		// stack pointer, points to top of stack
	
			 
	if(!init_graphics())
		return 1;
	int c;	
	c = getchar();	

	destroy_graphics();	
	return 0;
}
