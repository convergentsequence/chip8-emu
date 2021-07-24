#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <SDL2/SDL.h>

#define _1B 	bool
#define _8B 	unsigned char
#define _16B 	unsigned short

#define WINDOW_WIDTH 	640
#define WINDOW_HEIGHT 	320

#define GBUFF_SIZE 32*64

#define VERBOSE_OPCODES 1

SDL_Window *window;
SDL_Renderer *renderer;


	
_1B 	gbuff[GBUFF_SIZE]; 

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



static inline void decrement_timers(){
	delay_timer -= (delay_timer > 0);
	sound_timer -= (sound_timer > 0);
}

Uint32 last60hz = 0;
Uint32 current60hz;
static inline void _60hz(int currentCycleTime){
	current60hz = currentCycleTime;
	if(current60hz - last60hz >= 1000/60){


		decrement_timers();			
		render_gbuff();

		last60hz = current60hz;
	}
}

static inline void exit_emu(){
	destroy_graphics();
	exit(0);
}

static inline void verbose_opcode(_16B PC, _16B opcode, char* message, ...){

	#if(VERBOSE_OPCODES)
	char buffer[256];
	va_list args;
	va_start(args, message);
	vsnprintf(buffer, 255, message, args);
	va_end(args);

	printf("%03X: %04X %s\n", PC, opcode, buffer);
	#endif
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
 
	char rom_location[200];
	scanf("%s", rom_location);
	FILE *romfptr = fopen(rom_location, "rb");
	if(romfptr == NULL){
		printf("Couldn't open rom file: %s\n", rom_location);
		return 1; 
	} 
	size_t rom_size = fread(memory+0x200, sizeof(_8B), 4096-0x1FF, romfptr);
	fclose(romfptr);
	printf("Rom size: %ld\n", rom_size);

	if(!init_graphics())
		return 1;


	PC = 0x200;
	opcode = 0;
	memset(V, 0, 16);
	SP = 1;
	I = 0;


	int cycles_per_second = 144;
	Uint32 lastCycleTime = 0;
	Uint32 currentCycleTime;

	SDL_Event event;
	for(;;){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_QUIT:
				exit_emu();
			}		
		}		
		
		currentCycleTime = SDL_GetTicks();
		if(currentCycleTime - lastCycleTime >= 1000/cycles_per_second){

			opcode = memory[PC] << 8 | memory[PC+1];
			PC+=2;
			printf("%X\n", opcode);
			if(!opcode)
				exit_emu();
		
			switch(opcode & 0xF000){
			case 0: // opcodes starting with 0
				switch(opcode & 0x00FF){
				case 0xE0: // 0x00E0 - clear the screen
					verbose_opcode(PC, opcode, "Clearing screen");
					memset(gbuff, 0, GBUFF_SIZE);
					break;
				case 0xEE: // 0x00EE - return from subroutine call
					PC = stack[SP-1];
					verbose_opcode(PC, opcode, "Returning from subroutine to: %03X",stack[SP-1]);
					SP--;
					break;
				}
				break;

			case 1: // 0x1NNN - jump to location NNN
				_16B target_location = opcode & 0x0FFF;
				PC = target_location;
				verbose_opcode(PC, opcode, "Jumping to location: %03X", target_location);
				break;
			case 2: // 0x2NNN - jump to subroutine at address NNN
				_16B target_subroutine = opcode & 0x0FFF;
				stack[(SP++)-1] = PC+2;
				PC = target_location;
				verbose_opcode(PC, opcode, "Jumping to subroutine: %03X", target_subroutine);
				break;
			}

			lastCycleTime = currentCycleTime;
		}

		
		_60hz(currentCycleTime);	
		SDL_Delay(0);		
	}

	
	destroy_graphics();	
	return 0;
}
