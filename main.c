#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define _1B 	bool
#define _8B 	unsigned char
#define _16B 	unsigned short

#define WINDOW_WIDTH 	640
#define WINDOW_HEIGHT 	320


SDL_Window *window;
SDL_Renderer *renderer;


	
_1B 	gbuff[64*32]; 

_8B 	delay_timer;
_8B	sound_timer;

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

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL){
		printf("SDL renderer failed to initialise: %s\n", SDL_GetError());
	}

	SDL_RenderSetLogicalSize(renderer, 64, 32);

	SDL_SetRenderDrawColor(renderer, 0,0,0,0);
	SDL_RenderClear(renderer);		
	

	return true;
}

void destroy_graphics(){
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void render_gbuff(){
	int pixel;
	for(int i = 0; i < 64; i++){
		for(int j = 0; j < 32; j++){
			pixel = gbuff[i+j*64] * 255;	
			SDL_SetRenderDrawColor(renderer, pixel, pixel, pixel, pixel);
			SDL_RenderDrawPoint(renderer, i, j); 
		}
	}
	SDL_RenderPresent(renderer);
}



Uint32 last = 0;
void decrement_timers(){
	Uint32 current = SDL_GetTicks();
	if(current-last >= 1000/60){		
		delay_timer -= (delay_timer > 0);
		sound_timer -= (sound_timer > 0);
		last = current;
	}
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
	
	// load fontset into interpreter part of the memory
	memcpy(memory, fontset, 80);
 
	if(!init_graphics())
		return 1;


	PC = 0x200;
	opcode = 0;
	memset(V, 0, 16);
	SP = 0;
	I = 0;


	SDL_Event event;
	for(;;){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_QUIT:
				return 0;
			}		
		}		
		
		
		decrement_timers();		
	}


	destroy_graphics();	
	return 0;
}
