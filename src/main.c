/* mipslabmain.c

   This file written 2015 by Axel Isaksson,
   modified 2015, 2017 by F Lundevall

   Latest update 2017-04-21 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>  /* Declarations of uint_32 and the like */
#include <pic32mx.h> /* Declarations of system-specific addresses etc */
#include "boilerplate.h" /* Declatations for these labs */
#include "game.h"
#include <string.h>
#include <stdlib.h>

void setup(void) {
	/*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
	SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;
}

int highscore_cmp(const void *a, const void *b) {
	return ((Highscore *) b)->score - ((Highscore *)a)->score;
}

// global variables for entering highscore information
char player_name[9] = "_       \0";
int ch_idx = 0;

Highscore highscores[20];
int highscore_len = 0;

int light = 0;


int highscores_idx = 0;

int main(void) {

// --------------
	highscores[0].score = 10;
	memcpy(highscores[0].name, &"elliot  ", 9);
	++highscore_len;
	
	highscores[1].score = 20;
	memcpy(highscores[1].name, &"Hugo    ", 9);
	++highscore_len;
	
	highscores[2].score = 20;
	memcpy(highscores[2].name, &"Birger  ", 9);
	++highscore_len;

	qsort(highscores, highscore_len, sizeof(Highscore), highscore_cmp);
// -------------


	setup();

	set_timer_period(MENU_TIME_PERIOD);
	
	int dead;

	// display image on screen
	uint8_t img_data[512];

	// time between frames
	float dt = 1.0 / 25.0;

	// Flax player;
	// player.x = 10;
	// player.y = 16;
	// player.vel = 0;
	
	PipePair pipe;
	pipe.upper_edge = 20;
	pipe.lower_edge = 2;
	pipe.left_border = 100;
	pipe.right_border = 104;
	

	Game game;
	set_default_game_state(&game);
	// memset(game.screen, 0, 4096);
	// game.player = player;
	// game.score = 0;
	game.pipes[0] = pipe;
	game.pipes_len = 1;
	game.state = MainMenu;

	spawn_pipe(game.pipes, &game.pipes_len);
	
	halted = 1;
	io_init();

	display_init();
	// draw_game(&game);
	// image_to_data(game.screen, img_data);
	// display_image(img_data);
	
	int btn2 = 0;
	int btn3 = 0;
	int btn4 = 0;
	
	while( 1 ) {

		switch(game.state) {
			case MainMenu:
				display_string(0, "    ^ Flax ^");
				display_string(1, "");
				display_string(2, "BTN4 > play");
				display_string(3, "BTN3 > scores");
				display_update();
				
				if ( btn4 ) {
					set_timer_period(GAME_TIME_PERIOD);
					game.state = Playing;
				} else if ( btn3 ) {
					game.state = HighScoreMenu;
				}

				break;

			case Playing:
				dead = update_game(&game, dt);
				draw_game(&game);
				image_to_data(game.screen, img_data, light);
				display_image(img_data);
				write_led(game.score);

				if ( game.pipes[game.pipes_len-1].right_border < MAX_X - 20 ) {
					spawn_pipe(game.pipes, &game.pipes_len);
				}
				
				if ( btn4 ) {
					jump(&game.player);
				}

				if ( dead == 1 ) {
					game.state = GameOver;
					set_timer_period(MENU_TIME_PERIOD);
				}

				break;


			case GameOver:
				display_string(0, "   Enter name");
				display_string(1, player_name);
				display_string(2, "btn 2  3   4");
				display_string(3, "   ch  >  menu");
				display_update();

				if ( btn2 ) {
					player_name[ch_idx]++;
					
					if ( player_name[ch_idx] == '_'+1) {
						player_name[ch_idx] = 'A';
					} else if ( player_name[ch_idx] > 90 ) {
						player_name[ch_idx] = '_';
					}
				}
				
				if ( btn3 ) {
					if (player_name[ch_idx] == '_') {
						player_name[ch_idx] = ' ';
					} else {
						player_name[ch_idx] += 32;
					}

					++ch_idx;
					if ( ch_idx > 7 ) {
						ch_idx = 0;
					}
					
					if (player_name[ch_idx] == ' ') {
						player_name[ch_idx] = '_';
					} else {
						player_name[ch_idx] -= 32;
					}
				}

				if ( btn4 ) {
					int updated = 0;
					for ( int i = 0; i < highscore_len; ++i ) {
						if ( strcmp(highscores[i].name, player_name) == 0 ) {
							if ( game.score > highscores[i].score ) {
								highscores[i].score = game.score;
								qsort(highscores, highscore_len, sizeof(Highscore), highscore_cmp);
							}
							updated = 1;
							break;
						}
					}

					if ( updated == 0 ) {
						++highscore_len;
						strcpy(highscores[highscore_len-1].name, player_name);
						highscores[highscore_len-1].score = game.score;
						qsort(highscores, highscore_len, sizeof(Highscore), highscore_cmp);
					}

					// Reset game values
					set_default_game_state(&game);

					game.state = MainMenu;
				}

				break;

			case HighScoreMenu:
				
				
				display_string(0, "   Highscores");
				for ( int i = 0; i < 3; ++i ) {
					display_string(i+1, highscores[highscores_idx+i].name);
				}

				// display_string(1, "");
				// display_string(2, "");
				// display_string(3, "");

				display_update();

				if ( btn2 ) {
					++highscores_idx;
				} else if ( btn3 ) {
					--highscores_idx;
				} else if ( btn4 ) {
					;
				}
				break;
		}
		// wait for timer
		while ( halted != 0 ) {
			btn2 = btn_is_pressed(2);
			btn3 = btn_is_pressed(3);
			btn4 = btn_is_pressed(4);
		}
		halted = 1;
	}
	return 0;
}
