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
#include <stdlib.h> // srand
#include <stdio.h> // snprintf
#include <string.h>

/* Various setup */
void setup( void ) {
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

/**
* Compares two highscores based on the score value
* to sort in descending order
* @param a should point to struct of Highscore type
* @param b should point to struct of Highscore type
*/
int highscore_cmp(const void *a, const void *b) {
	return ((Highscore *) b)->score - ((Highscore *)a)->score;
}


/* Main function */
int main( void ) {
	// variables for entering highscore information
	char player_name[PLAYER_NAME_LEN] = "_      ";
	int ch_idx = 0;
	Highscore highscores[HIGHSCORES_LEN];
	int highscore_len = 0;
	int highscores_idx = 0;
	memset(highscores, 0, HIGHSCORES_LEN*sizeof(Highscore));

	// boolean values for whether button is pressed or not
	int btn2 = 0, btn3 = 0, btn4 = 0;

	// boolean value for dark/light mode
	int light = 0;
	// boolean value telling whether player is alive or not
	int dead;
	// Buffer for rendering to screen
	uint8_t img_data[512];
	// game time passed between frames
	float dt = 1.0 / 25.0;
	// counting time passed before user input, used for srand()
	uint32_t seed_counter = 0;

	Game game;
	game.state = StartMenu;

	// dummy highscores ------------------------------------------------
	// ----------------------------------------------------------------
	highscores[0].score = 10;
	memcpy(highscores[0].name, &"elliot ", PLAYER_NAME_LEN);
	++highscore_len;
	
	highscores[1].score = 20;
	memcpy(highscores[1].name, &"hugo   ", PLAYER_NAME_LEN);
	++highscore_len;
	
	highscores[2].score = 20;
	memcpy(highscores[2].name, &"birger ", PLAYER_NAME_LEN);
	++highscore_len;

	qsort(highscores, highscore_len, sizeof(Highscore), highscore_cmp);
	// ----------------------------------------------------------------
	// ----------------------------------------------------------------

	// general initialization
	setup();
	io_init();
	display_init();
	set_timer_period(MENU_TIME_PERIOD);
	// set global variable for telling if game is waiting for delay
	halted = 1;
	
	while( 1 ) {
		switch(game.state) {
			case StartMenu:
				display_string(0, "    ^ Flax ^");
				display_string(1, "");
				display_string(2, "BTN4 > play");
				display_string(3, "BTN3 > scores");
				display_update(light);

				seed_counter += 4711;
				
				if ( btn4 ) {
					set_timer_period(GAME_TIME_PERIOD);
					game.state = Playing;
					srand(seed_counter);
				} else if ( btn3 ) {
					game.state = HighScoreMenu;
					srand(seed_counter);
				}

				set_default_game_state(&game);
				break;

			case MainMenu:
				display_string(0, "    ^ Flax ^");
				display_string(1, "");
				display_string(2, "BTN4 > play");
				display_string(3, "BTN3 > scores");
				display_update(light);
				
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

				if ( game.pipes[game.pipes_len-1].right< MAX_X - PIPE_SPACING ) {
					spawn_pipe(game.pipes, &game.pipes_len, Static, DEFAULT_DYNAMIC_PIPE_SPEED, MAX_X);
				}
				
				if ( btn4 ) {
					jump(&game.player);
				}

				if ( dead == 1 ) {
					if ( game.score == 0 ) {
						game.state = MainMenu;
						set_default_game_state(&game);
					} else {
						game.state = GameOver;
					}

					set_timer_period(MENU_TIME_PERIOD);
				}

				break;

			case GameOver:
				display_string(0, "   Enter name");
				display_string(1, player_name);
				display_string(2, "btn 4  3   2");
				display_string(3, "  menu >   ch");
				display_update(light);

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
					if ( ch_idx > PLAYER_NAME_LEN - 2 ) {
						ch_idx = 0;
					}
					
					if (player_name[ch_idx] == ' ') {
						player_name[ch_idx] = '_';
					} else {
						player_name[ch_idx] -= 32;
					}
				}

				if ( btn4 ) {
					if ( player_name[ch_idx] == '_' ) {
						player_name[ch_idx] = ' ';
					} else if ( player_name[ch_idx] >= 65 && player_name[ch_idx] <= 90 ) { // fix upper case letter
						player_name[ch_idx] += 32;
					}

					ch_idx = 0;

					int updated = 0;
					for ( int i = 0; i < highscore_len; ++i ) {
						if ( strcmp(highscores[i].name, player_name) == 0 ) {
							// update existing highscore
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
						memcpy(highscores[highscore_len-1].name, player_name, PLAYER_NAME_LEN);
						highscores[highscore_len-1].score = game.score;
						qsort(highscores, highscore_len, sizeof(Highscore), highscore_cmp);
					}

					// reset values for the next game
					memcpy(player_name, (const char*) &"_      ", PLAYER_NAME_LEN);
					set_default_game_state(&game);
					write_led(0);

					game.state = MainMenu;
				}

				break;

			case HighScoreMenu:
				display_string(0, "   Highscores");
				for ( int i = 0; i < 3 && highscores_idx + i < HIGHSCORES_LEN+1; ++i ) {
					char buffer[15];
					if (highscores_idx+i >= HIGHSCORES_LEN ) {
						memcpy(buffer, "              ", 14);
						buffer[15] = '\0';
					} else if ( strlen(highscores[highscores_idx+i].name) == 0 ) {
						snprintf(buffer, 15, "%02d", highscores_idx+i+1);
					} else {
						snprintf(
							buffer, 15, "%02d %s %03d",
							highscores_idx+i+1, highscores[highscores_idx+i].name, highscores[highscores_idx+i].score
						);
					}
					display_string(i+1, buffer);
				}

				display_update(light);

				if ( btn2 ) {
					highscores_idx += 3;
					if ( highscores_idx >= HIGHSCORES_LEN ) {
						highscores_idx = 0;
					}
				} else if ( btn3 && highscores_idx > 0 ) {
					highscores_idx -= 3;
				} else if ( btn4 ) {
					game.state = MainMenu;
				}
				
				break;
		}
		// wait for timer
		while ( halted != 0 ) {
			btn2 = btn_is_pressed(2);
			btn3 = btn_is_pressed(3);
			btn4 = btn_is_pressed(4);
			
			light = sw_is_toggled(1);
		}
		halted = 1;
	}
	return 0;
}
