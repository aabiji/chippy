#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PIXEL_SIZE 12
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

uint8_t SP = 0; //STACK pointer
uint16_t PC = 0x200; //program counter
uint16_t I = 0;
uint8_t SOUND_TIMER = 0;
uint8_t DELAY_TIMER = 0;
uint8_t V[16] = {0}; //registers
uint16_t STACK[16] = {0};
int KEYBOARD[16] = {0};
int DISPLAY[DISPLAY_HEIGHT][DISPLAY_WIDTH] = {0};
uint8_t MEMORY[4096] = {
	//font set
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

void capture_keyboard_input()
{
	if (IsKeyDown(KEY_ONE)) KEYBOARD[1] = 1;
	if (IsKeyDown(KEY_TWO)) KEYBOARD[2] = 1;
	if (IsKeyDown(KEY_THREE)) KEYBOARD[3] = 1;
	if (IsKeyDown(KEY_FOUR)) KEYBOARD[12] = 1;
	if (IsKeyDown(KEY_Q)) KEYBOARD[4] = 1;
	if (IsKeyDown(KEY_W)) KEYBOARD[5] = 1;
	if (IsKeyDown(KEY_E)) KEYBOARD[6] = 1;
	if (IsKeyDown(KEY_R)) KEYBOARD[13] = 1;
	if (IsKeyDown(KEY_A)) KEYBOARD[7] = 1;
	if (IsKeyDown(KEY_S)) KEYBOARD[8] = 1;
	if (IsKeyDown(KEY_D)) KEYBOARD[9] = 1;
	if (IsKeyDown(KEY_F)) KEYBOARD[13] = 1;
	if (IsKeyDown(KEY_Z)) KEYBOARD[10] = 1;
	if (IsKeyDown(KEY_X)) KEYBOARD[0] = 1;
	if (IsKeyDown(KEY_C)) KEYBOARD[11] = 1;
	if (IsKeyDown(KEY_V)) KEYBOARD[15] = 1;
}

void load_rom(char rom_file_str[])
{
	FILE *f;
	f = fopen(rom_file_str, "rb");
	unsigned char c;
	int index = 0x200;
	do {
		c = fgetc(f);
		if (!feof(f)) MEMORY[index] = c;
		index += 1;
	} while (!feof(f));
	fclose(f);
}

int execute_instruction(uint16_t instruction)
{
	capture_keyboard_input();
	int return_flag = 0;
	uint16_t nnn = instruction & 0xFFF;
	uint16_t kk = instruction & 0xFF;
	uint8_t n = instruction & 0x000F;
	uint8_t x = (instruction & 0x0F00) >> 8;
	uint8_t y = (instruction & 0x00F0) >> 4;

	switch (instruction & 0xF000)
	{
		case 0x0000:
			switch (n)
			{
				case 0x0000: //00E0
					memset(DISPLAY, 0, sizeof(DISPLAY));
					PC += 2;
					break;

				case 0x000E: //00EE
					SP -= 1;
					PC = STACK[SP];
					PC += 2;
					break;
			}
			break;

		case 0x1000: //1nnn
			PC = nnn;
			break;

		case 0x2000: //2nnn
			STACK[SP] = PC;
			SP += 1;
			PC = nnn;
			break;

		case 0x3000: //3xkk
			if (V[x] == kk) PC += 2;
			PC += 2;
			break;

		case 0x4000: //4xkk
			if (V[x] != kk) PC += 2;
			PC += 2;
			break;

		case 0x5000: //5xy0
			if (V[x] == V[y]) PC += 2;
			PC += 2;
			break;

		case 0x6000: //6xkk
			V[x] = kk;
			PC += 2;
			break;

		case 0x7000: //7xkk
			V[x] += kk;
			PC += 2;
			break;

		case 0x8000:
			switch (n)
			{
				case 0x0000: //8xy0
					V[x] = V[y];
					PC += 2;
					break;

				case 0x0001: //8xy1
					V[x] = V[x] | V[y];
					PC += 2;
					break;

				case 0x0002: //8xy2
					V[x] = V[x] & V[y];
					PC += 2;
					break;

				case 0x0003: //8xy3
					V[x] = V[x] ^ V[y];
					PC += 2;
					break;

				case 0x0004: //8xy4
					if ((V[x] + V[y]) > 255) V[0xF] = 1;
					V[x] = V[x] + V[y];
					PC += 2;
					break;

				case 0x0005: //8xy5
					if (V[x] > V[y]) V[0xF] = 1;
					V[x] = V[x] - V[y];
					PC += 2;
					break;

				case 0x0006: //8xy6
					V[x] = V[x] >> 1;
					V[0xF] = (V[x] & 0x01);
					PC += 2;
					break;

				case 0x0007: //8xy7
					if (V[y] > V[x]) V[0xF] = 1;
					V[x] = V[y] - V[x];
					PC += 2;
					break;

				case 0x000E:
					V[x] = V[x] << 1;
					V[0xF] = (V[x] & 0x80) >> 7;
					PC += 2;
					break;
			}
			break;

		case 0x9000: //9xy0
			if (V[x] != V[y]) PC += 2;
			PC += 2;
			break;

		case 0xA000: //Annn
			I = nnn;
			PC += 2;
			break;

		case 0xC000: //Cxkk
			V[x] = (rand() % 255) & kk;
			PC += 2;
			break;

		case 0xD000: //Dxyn
			V[0xF] = 0;
			uint8_t xpos = V[x] % 64;
			uint8_t ypos = V[y] % 32;
			uint8_t height = n;
			for (int row = 0; row < height; row++)
			{
				uint8_t sprite = MEMORY[I + row];
				for (int col = 0; col < 8; col++)
				{
					if ((sprite & (0x80 >> col)) != 0)
					{
						int pixel = DISPLAY[row + ypos][col + xpos];
						if (pixel == 1) V[0xF] = 1;
						DISPLAY[ypos + row][xpos + col] ^= 1;
					}
				}
			}
			PC += 2;
			break;

		case 0xE000:
			switch (n) 
			{
				case 0x000E: //Ex9E
					if (KEYBOARD[V[x]] == 1) PC += 2;
					PC += 2;
					break;

				case 0x0001: //ExA1
					capture_keyboard_input();
					if (KEYBOARD[V[x]] == 0) PC += 2;
					PC += 2;
					break;
			}
			break;

		case 0xF000:
			switch (kk)
			{
				case 0x0007: //Fx07
					V[x] = DELAY_TIMER;
					PC += 2;
					break;

				case 0x000A: //Fx0A
				{
					int captured = 0;
					capture_keyboard_input();
					for (int i = 0; i < 16; i++)
					{
						if (KEYBOARD[i] == 1)
						{
							V[x] = i;
							captured = 1;
							break;
						}
					}
					if (captured != 1)
					{
						PC += 2;
					} else {
						PC -= 2;
					}
					break;
				}

				case 0x0015: //Fx15
					DELAY_TIMER = V[x];
					PC += 2;
					break;

				case 0x0018: //Fx18
					SOUND_TIMER = V[x];
					PC += 2;
					break;

				case 0x001E: //Fx1E
					I = I + V[x];
					PC += 2;
					break;

				case 0x0029: //Fx29
					I = V[x] * 5;
					PC += 2;
					break;

				case 0x0033: //Fx33
				{
					uint8_t number = V[x];
					MEMORY[I + 2] = number % 10;
					number = number / 10;
					MEMORY[I + 1] = number % 10;
					number = number / 10;
					MEMORY[I] = number;
					PC += 2;
					break;
				}

				case 0x0055: //Fx55
					for (int i = 0; i <= x; i++) MEMORY[I + i] = V[i];
					PC += 2;
					break;

				case 0x0065: //Fx65
					for (int i = 0; i <= x; i++) V[i] = MEMORY[I + i];
					PC += 2;
					break;
			}
			break;

		default:
			printf("Error --> Unknown instruction\n\n: %04X", instruction);
			return_flag = 1;
	}

	return return_flag;
}

int main(const int argc, char *argv[])
{
	/* STRING MANIPULATIONS */
	char window_title[] = "Chippy | Currently emulating: ";
	char game_name[100] = "";
	char game_name1[100] = "";
	strncpy(game_name, argv[1] + 12, (int)strlen(argv[1]) - 12);
	int game_name_size = (int)strlen(game_name);
	strncpy(game_name1, game_name + 0, game_name_size - 4);
	strcat(window_title, game_name1);
	
	/* INIT */
	//int KEYBOARD_reload = 0;
	srand(time(NULL));
	load_rom(argv[1]);
	InitWindow(PIXEL_SIZE * DISPLAY_WIDTH, PIXEL_SIZE * DISPLAY_HEIGHT, window_title);
	InitAudioDevice();
	SetTargetFPS(75);
	Sound beep_sfx = LoadSound("assets/Beep.wav");

    int cyclesPerSecond = 150;

	while (!WindowShouldClose())
	{
		/* TIMERS */
		if (DELAY_TIMER > 0) DELAY_TIMER--;
		if (SOUND_TIMER > 0)
		{
			PlaySoundMulti(beep_sfx);
			SOUND_TIMER--;
		}

		/* FETCH, DECODE, EXECUTE INSTRUCTION*/
        for (int i = 0; i < cyclesPerSecond; i++) {
            capture_keyboard_input();
		    uint16_t instruction = (MEMORY[PC] << 8) + MEMORY[PC+1];
		    int err = execute_instruction(instruction);
		    if (err != 0) break;
        }

		/* RENDER DISPLAY */
		BeginDrawing();
		ClearBackground(BLACK);
		for (int y = 0; y < DISPLAY_HEIGHT; y++)
		{
			for (int x = 0; x < DISPLAY_WIDTH; x++)
			{
				if (DISPLAY[y][x] == 1) 
					DrawRectangle(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, WHITE);
			}
		}	
		EndDrawing();

		/* MISC 
		KEYBOARD_reload += 1;
		if (KEYBOARD_reload % 45 == 1)
			memset(KEYBOARD, 0, sizeof(KEYBOARD)); //reset KEYBOARD
        */
	}

	/* Cleanup */
	StopSoundMulti();
	UnloadSound(beep_sfx);
	CloseAudioDevice();
	CloseWindow();
}
