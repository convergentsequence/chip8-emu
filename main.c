#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

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

	printf("%03X: %04X %s\n", PC-2, opcode, buffer);
	#endif
}



int main()
{

	srand(time(NULL));

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


	int cycles_per_second = 500;
	Uint32 lastCycleTime = 0;
	Uint32 currentCycleTime;

	int sx;
	int sy;
	int sn;
	int pixel;

	int sub;


	bool keycodes[16];
	int keycode;
	bool pressed;
	memset(keycodes, 0, 16*sizeof(bool));

	SDL_Event event;
	for(;;){
		while(SDL_PollEvent(&event)){
			switch(event.type){
			case SDL_KEYDOWN:
				keycode = event.key.keysym.scancode;
				if(keycode >= 4 && keycode <= 9){
					keycodes[0xA + (keycode - 4)] = true;
				}
				if(keycode >= 30 && keycode <= 38){
					keycodes[1+(keycode - 30)] = true;
				}
				if(keycode == 39){
					keycodes[0] = true;
				}
				break;
			case SDL_KEYUP:
				keycode = event.key.keysym.scancode;
				if(keycode >= 4 && keycode <= 9){
					keycodes[0xA + (keycode - 4)] = false;
				}
				if(keycode >= 30 && keycode <= 38){
					keycodes[1+(keycode - 30)] = false;
				}
				if(keycode == 39){
					keycodes[0] = false;
				}
				break;
			case SDL_QUIT:
				exit_emu();
			}		
		}		
		
		currentCycleTime = SDL_GetTicks();
		if(currentCycleTime - lastCycleTime >= 1000/cycles_per_second){
			opcode = memory[PC] << 8 | memory[PC+1];
			if(!opcode){
				printf("Exiting, PC: %d, OPCODE: 0x%4X\n", PC, opcode);
				exit_emu();
			}	
			PC+=2;

			switch(opcode & 0xF000){
			case 0: // opcodes starting with 0
				switch(opcode & 0x00FF){
				case 0xE0: // 0x00E0 - clear the screen
					verbose_opcode(PC, opcode, "Clearing screen");
					memset(gbuff, 0, GBUFF_SIZE);
					break;
				case 0xEE: // 0x00EE - return from subroutine call
					verbose_opcode(PC, opcode, "Returning from subroutine to: %03X",stack[SP-1]);
					PC = stack[--SP];
					break;
				}

				break;

			case 0x1000: // 0x1NNN - jump to location NNN
				verbose_opcode(PC, opcode, "Jumping to location: %03X", opcode & 0x0FFF);
				PC = opcode & 0xFFF;
				break;
			
			case 0x2000: // 0x2NNN - jump to subroutine at address NNN
				verbose_opcode(PC, opcode, "Jumping to subroutine: %03X", opcode & 0xFFF);
				stack[(SP++)] = PC;
				PC = opcode & 0xFFF;
				break;
			
			case 0x3000: // 0x3XRR - skip next instruction if V[X] == 0xRR 
				verbose_opcode(PC, opcode, "Skipping next instruction if V%d == %X", (opcode & 0xF00) / 0x100,  (opcode & 0xFF)); 
				PC += 2 * ( V[ (opcode & 0xF00) / 0x100 ] == (opcode & 0xFF) );
				break;
			
			case 0x4000: // 0x4XRR - skip next intruction if V[X] != 0xRR
				verbose_opcode(PC, opcode, "Skipping next instruction if V%d != %X",  (opcode & 0xF00) / 0x100 ,  (opcode & 0xFF)); 
				PC += 2 * ( V[ (opcode & 0xF00) / 0x100 ] != (opcode & 0xFF)  );
				break;
			
			case 0x5000: // 0x5XY0 - skip next instruction if V[X] == V[Y]
				verbose_opcode(PC, opcode, "Skipping next instruction if V%d != V%d", (opcode & 0xF00) / 0x100, (opcode & 0xF0) / 0x10 );
				PC += 2 * ( V[ (opcode & 0xF00) / 0x100  ] == V[ (opcode & 0xF0) / 0x10 ] );
				break;

			case 0x6000: // 0x6XRR - move constant RR into V[X]
				verbose_opcode(PC, opcode, "Moving 0x%X into V%d", opcode & 0xFF, (opcode & 0xF00) / 0x100);
				V[ (opcode & 0xF00) / 0x100 ] = opcode & 0xFF;
				break;
				
			case 0x7000: // 0x7XRR - add RR to value of V[X]
				verbose_opcode(PC, opcode, "Adding %X to V%d", (opcode & 0xFF), (opcode & 0xF00) / 0x100);
				V[ (opcode & 0xF00) / 0x100 ] += opcode & 0xFF;
				break;

			case 0x8000: // opcodes that start with 8
				switch(opcode & 0x00F){ // check opcode endings
				case 0: // 0x8XY0 - move register VY to register VX
					verbose_opcode(PC, opcode, "Moving V%d into V%D", (opcode & 0xF0) / 0x10, (opcode & 0xF00) / 0x100);
					V[ (opcode & 0xF00) / 0x100 ] = V [ (opcode & 0xF0) / 0x10 ];
					break;
				case 1: // 0x8XY1 - stores the value of VX | VY into VX
					verbose_opcode(PC, opcode, "Setting V%1$d to V%1$d | V%2$d", (opcode & 0xF00) / 0x100, (opcode & 0xF0) / 0x10);
					V[ (opcode & 0xF00) / 0x100 ] |= V[ (opcode & 0xF0) / 0x10 ];
					break;
				case 2: // 0x8XY2 - add value of VY to VX
					verbose_opcode(PC, opcode, "Bitwise AND the value of V%d and V%d", (opcode & 0xF0) / 0x10, (opcode & 0xF00) / 0x100);
					V[ (opcode & 0xF00) / 0x100 ] &= V[ (opcode & 0xF0) / 0x10 ];
					break;
				case 3: // 0x8XY3 - XOR VY and X store in VX
					verbose_opcode(PC, opcode, "Setting V%1$d to V%1$d XOR V%2$d", (opcode & 0xF00) / 0x100, (opcode & 0xF0) / 0x10);
					V[ (opcode & 0xF00) / 0x100 ] ^= V[ (opcode & 0xF0) / 0x10 ];
					break;
				case 4: // 0x8XY4 - Add VY to VX store carry in V15
					verbose_opcode(PC, opcode, "Adding V%d to V%d and storing the carry in V15", (opcode & 0xF0) / 0x10, (opcode & 0xF00) / 0x100);
					int sum = V[ (opcode & 0xF00) / 0x100 ] + V[ (opcode & 0xF0) / 0x10 ];
					V[15] = sum > 255;
					V[ (opcode & 0xF00) / 0x100 ]  = sum;
					break;
				case 5: // 0x8XY5 - Subtract VY from VX and store the borrow in V15
					verbose_opcode(PC, opcode, "Subtracting V%d from V%d and storing the borrow in V15", (opcode & 0xF0) / 0x10, (opcode & 0xF00) / 0x100);
					sub =  V[ (opcode & 0xF00) / 0x100 ] - V[ (opcode & 0xF0) / 0x10];
					V[15] = sub >= 0;
					V[ (opcode & 0xF00) / 0x100 ] = sub;
					break;
				case 6: // 0x8X06 - Shift VX to right, first bit goes to V[15]
					verbose_opcode(PC, opcode, "Shiting V%d to the right, storing 1st bit in V15", (opcode & 0xF00) / 0x100);
					V[ (opcode & 0xF00) / 0x100 ] >>= 1;
					V[15] = V[ (opcode & 0xF00) / 0x100 ] & 1;
					break;
				case 7: // 0x8XY7 - Subtract VX from VY result stored in VX
					verbose_opcode(PC, opcode, "Subtracting V%d from V%d storing the borrow in V15", (opcode & 0xF00) / 0x100, (opcode & 0xF0) / 0x10);
					sub = V[ (opcode & 0xF0) / 0x10 ] - V[ (opcode & 0xF00) / 0x100 ];
					V[15] = sub >= 0;
					V[ (opcode & 0xF00) / 0x100 ] = sub;
					printf("Result: %d, borrow: %d\n", V[ (opcode & 0xF00) / 0x100 ], V[15]); 
					break;
				case 0xE: // 0x8X0E - Shift VX to left,most significant bit goes to V15
					verbose_opcode(PC, opcode, "Shifting VX to left storing the most significant bit in V15", (opcode & 0xF00) / 0x100);
					V[15] = V[ (opcode & 0xF00) / 0x100 ] >> 7;
					V[ (opcode & 0xF00) / 0x100 ] <<= 1;
					break;
			
				}

				break;
			
			case 0xA000: // 0xANNN - Put NNN into I
				verbose_opcode(PC, opcode, "Putting %03X into I", opcode & 0xFFF);
				I = opcode & 0xFFF;
				break;
			
			case 0xB000: // 0xBNNN - Jump to address NNN plus register V0
				verbose_opcode(PC, opcode, "Jumping to address %03X + V0", opcode & 0xFFF);
				PC = (opcode & 0xFFF) + V[0];
				break;
			
			case 0xC000: // 0xCXKK - Set VX to (random number between 0 - 255) & KK
				verbose_opcode(PC, opcode, "Setting V%d to random number & %02X", (opcode & 0xF00) / 0x100, opcode & 0xFF);
				V[ (opcode & 0xF00) / 0x100 ] = (rand() % 255) & (opcode & 0xFF);
				break;

			/*
			*
			*	Dxyn - DRW Vx, Vy, nibble
			*	Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			*	The interpreter reads n bytes from memory, starting at the address stored in I. These bytes are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites are XORed onto the existing screen.
			*	If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is outside the coordinates of the display, 
			*	it wraps around to the opposite side of the screen.
			*	
			*	A sprite is 8 bits of length and n bits of height
			*
			*/

			case 0xD000: 
				verbose_opcode(PC, opcode, "Drawing sprite with length %d memory location %X at coordinates %d %d", opcode & 0xF, I ,V[(opcode & 0xF00) / 0x100], V[(opcode & 0xF0) / 0x10]);
				
				sx = V[(opcode & 0x0F00) >> 8];
               			sy = V[(opcode & 0x00F0) >> 4];
                		sn = opcode & 0x000F;
                		V[15] &= 0;

                		for(int i = 0; i < sn; i++){
                    			pixel = memory[I + i];
                    			for(int j = 0; j < 8; j++){
                        			if(pixel & (0b10000000 >> j)){
                                			V[15] = gbuff[(j+sx)%64+((i+sy)%32)*64];
                            				gbuff[(j+sx)%64+((i+sy)%32)*64] ^= 1;
                        			}
                    			}
                		}

				break;

			case 0xE000: // opcodes starting with 0xE
				switch(opcode & 0xFF){ // check opcode endings
				case 0x9E: // 0xEx9E - skip next instruction if key in Vx is pressed
					verbose_opcode(PC, opcode, "Skip next instruction if key in V%d (%X) is pressed", (opcode & 0xF00) / 0x100, V[(opcode & 0xF00) / 0x100] );
					if(keycodes[V[ (opcode & 0xF00) / 0x100]])
						PC += 2;
					break;	
				case 0xA1: // 0xExA1 - skip next instruction if key in Vx is not pressed
					verbose_opcode(PC, opcode, "Skip the next instruction if the key in V%d (%X) is not pressed",  (opcode & 0xF00) / 0x100, V[(opcode & 0xF00) / 0x100] );
					if(!keycodes[V[ (opcode & 0xF00) / 0x100]])
						PC += 2;
					break;
				}
				break;

			case 0xF000: // opcodes starting with 0xF
				switch(opcode & 0xFF){
				case 0x07: // 0xFx07 - put delay timer into Vx
					verbose_opcode(PC, opcode, "Putting value of delay timer into V%d", (opcode & 0xF00) / 0x100);
					V[ (opcode & 0xF00) / 0x100 ] = delay_timer;
					break;

				case 0x0A: // 0xFx0A - Wait for key press store the value of the key in Vx
					verbose_opcode(PC, opcode, "Waiting for key press and storing the value of key in V%d", (opcode & 0xF00) / 0x100);
					pressed = false;
					while(!pressed){
						while(SDL_PollEvent(&event)){
							case SDL_KEYDOWN:
								keycode = event.key.keysym.scancode;
								if(keycode >= 4 && keycode <= 9){
									V[(opcode & 0xF00) / 0x100] = 0xA + keycode - 4;
									pressed = true;
								}
								if(keycode >= 30 && keycode <= 38){
									V[(opcode & 0xF00) / 0x100] = 1 + (opcode - 30);
									pressed = true;
								}
								if(keycode == 39){
									V[(opcode & 0xF00) / 0x100]  = 0;
									pressed = true;
								}
								break;
							case SDL_QUIT:
								exit_emu();
						}
					}

					break;
				
				case 0x15: // 0xFx15 - Set delay timer to value of Vx
					verbose_opcode(PC, opcode, "Setting delay timer value to V%d", (opcode & 0xF00) / 0x100);
					delay_timer = V[ (opcode & 0xF00) / 0x100 ];
					break;
				
				case 0x18: // 0xFx18 - set timer value to Vx
					verbose_opcode(PC, opcode, "Setting sound timer value to V%d", (opcode & 0xF00) / 0x100);
					sound_timer = V[ (opcode & 0xF00) / 0x100 ] ;
					break;

				case 0x1E: // 0xFx1E - value of Vx is added to I
					verbose_opcode(PC, opcode, "Adding V%d to I", (opcode & 0xF00) / 0x100);
					I += V[ (opcode & 0xF00) / 0x100 ];
					break;

				case 0x29: // 0xFx29 - the value of I is set to sprite location of digit Vx
					verbose_opcode(PC, opcode, "I is set to sprite location of %X", (opcode & 0xF00) / 0x100);
					I = V[((opcode & 0xF00) / 0x100)] * 5;
					break;
				
				case 0x33: // 0xFx33 - store BCD represebtation of Vx in I
					verbose_opcode(PC, opcode, "Storing BCD of V%d in I", (opcode & 0xF00) / 0x100);
					memory[I] = V[ (opcode & 0xF00) / 0x100 ] / 100;
					memory[I+1] = (V[ (opcode & 0xF00) / 0x100 ] / 10) % 10;
					memory[I+2] =  V[ (opcode & 0xF00) / 0x100 ] % 10;
					break;
				
				case 0x55: // 0xFx55 - store the value of registers 0 to X into memory at I
					verbose_opcode(PC, opcode, "Store the value of registers V0 to V%d into memory at I", (opcode & 0xF00) / 0x100);
					for(int i = 0; i <=((opcode & 0xF00) / 0x100); i++){
						memory[I+i] = V[i];
					}
					break;
				
				case 0x65: // 0xFx65 load registers from V0 to VX from location I
					verbose_opcode(PC, opcode, "Read registers V0 to V%d from I", (opcode & 0xF00) / 0x100);
					for(int i = 0; i <=((opcode & 0xF00) / 0x100); i++){
						 V[i] = memory[I+i];
					}
					break;
				}
				
				break;
			default:
				verbose_opcode(PC, opcode, "Unkown opcode");
			
			}
			lastCycleTime = currentCycleTime;
		}

			
		_60hz(currentCycleTime);	
		SDL_Delay(0);		
	}

	
	destroy_graphics();	
	return 0;
}
