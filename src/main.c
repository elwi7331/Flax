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

#define ARBITRARY_LARGE_NUMBER 4711
#define DEFAULT_NAMES_LEN 5

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
* @return int difference, used by qsort
*/
int highscore_cmp(const void *a, const void *b) {
	return ((Highscore *) b)->score - ((Highscore *)a)->score;
}

/* Main function */
int main( void ) {
	// variables for entering highscore information
	char player_name[PLAYER_NAME_LEN] = "_      ";
	Highscore highscores[HIGHSCORES_LEN];
	int ch_idx = 0, highscore_len = 0, highscores_idx = 0;
	// initalize the memory to zero. Not doing this caused some issues in the past
	memset(highscores, 0, HIGHSCORES_LEN*sizeof(Highscore));

	// If the player does not input a name on the highscore list, a random one is assigned
	const char default_names[DEFAULT_NAMES_LEN][8] = {
		"podobas",
		"glassey",
		"branden",
		"ekola  ",
		"kann   "
	};

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

	// The GAME
	Game game;
	game.state = StartMenu;

	// set global variable for telling if game is waiting for delay
	halted = 1;

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
	
	while( 1 ) {
		switch(game.state) {
			/**
			 * The startmenu is only run once, when the game is started. It is otherwise identical to MainMenu.
			 * The reason for having StartMenu, is that in this state, the seed for the random function is incremented
			 * every frame. Thus, the random seed depends on how long the player spends in this state on startup.
			*/
			case StartMenu:
				display_string(0, "    ^ Flax ^");
				display_string(1, "");
				display_string(2, "BTN4 > play");
				display_string(3, "BTN3 > scores");
				display_update(light);

				// increment the seed to be used in rand
				if (UINT32_MAX - seed_counter < ARBITRARY_LARGE_NUMBER ) {
					seed_counter = 0;
				} else {
					seed_counter += ARBITRARY_LARGE_NUMBER;
				}
				
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
			
			/**
			 * The main menu where the user can enter a new game or go the highscore list.
			*/
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

			/**
			 * The game is running.
			*/
			case Playing:
				dead = update_game(&game, dt);
				draw_game(&game);
				image_to_data(game.screen, img_data, light);
				display_image(img_data);
				write_led(game.score);
				
				if ( btn4 ) {
					jump(&game.player);
				}

				if ( dead == 1 ) {
					if ( game.score == 0 ) {
						game.state = MainMenu;
						set_default_game_state(&game);
					} else {
						game.state = EnterHighscore;
					}

					set_timer_period(MENU_TIME_PERIOD);
				}

			break;

			/**
			 * Here, the player enters their name, which will later appear in the highscore list.
			 * This state is only entered if the player has at least 1 score.
			*/
			case EnterHighscore:
				display_string(0, "   Enter name");
				display_string(1, player_name);
				display_string(2, "btn 4  3   2");
				display_string(3, "  menu >   ch");
				display_update(light);

				// btn 2 increments the character value by 1, on the index where the cursor is pointing.
				// We assert that the characters can only be latin letters.
				// Wraps around when reaching last letter.
				if ( btn2 ) {
					if ( player_name[ch_idx] == '_') {
						player_name[ch_idx] = 'A';
					} else if ( player_name[ch_idx] >= 'Z' ) {
						player_name[ch_idx] = '_';
					} else {
						++player_name[ch_idx];
					}
				}
				
				// move the cursor to the next character, wraps around at the end
				if ( btn3 ) {
					// change current character to underscore if no letter is selected
					if (player_name[ch_idx] == '_') {
						player_name[ch_idx] = ' ';
					} else { // make the current character uppercase
						player_name[ch_idx] += 32;
					}

					++ch_idx;

					// wrap around to the first character
					if ( ch_idx > PLAYER_NAME_LEN - 2 ) {
						ch_idx = 0;
					}
					
					// if no character has been selected at the current position it should be an underscore
					if (player_name[ch_idx] == ' ') {
						player_name[ch_idx] = '_';
					} else { // the previous character is set to lowercase
						player_name[ch_idx] -= 32;
					}
				}

				// go back to main menu
				if ( btn4 ) {
					// if no letter has been selected at the current position, it is restored to whitespace
					if ( player_name[ch_idx] == '_' ) {
						player_name[ch_idx] = ' ';
					// set upper case to lower case
					} else if ( player_name[ch_idx] >= 65 && player_name[ch_idx] <= 90 ) {
						player_name[ch_idx] += 32;
					}
					
					// if the player has not entered a name, a random one is assigned
					if ( strcmp(player_name, "       ") == 0 ) {
						int index = randrng(0, DEFAULT_NAMES_LEN - 1);
						memcpy(player_name, default_names[index], PLAYER_NAME_LEN);
					}

					ch_idx = 0;

					int updated = 0;
					for ( int i = 0; i < highscore_len; ++i ) {
						// if the entered name is already in the highscore list
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

					// Create a new highscore if a new name was entered
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

			/**
			* A menu where the user can scroll through a list of highscores.
			*/
			case HighScoreMenu:
				display_string(0, "   Highscores");

				/**
				 * Three highscores are shown at the same time.
				 * if the amount of highscores is not divisible by three, 
				 * the last page will populate the empty slots with whitespace.
				*/
				for ( int i = 0; i < 3 && highscores_idx + i < HIGHSCORES_LEN+1; ++i ) {
					char buffer[15];
					// if there is no highscore at the current index, populate with whitespace
					if (highscores_idx+i >= HIGHSCORES_LEN ) {
						memcpy(buffer, "              ", 14);
						buffer[15] = '\0';
					// empty highscore entries
					} else if ( strlen(highscores[highscores_idx+i].name) == 0 ) {
						snprintf(buffer, 15, "%02d", highscores_idx+i+1);
					} else { // complete highscore information
						snprintf(
							buffer, 15, "%02d %s %03d",
							highscores_idx+i+1, highscores[highscores_idx+i].name, highscores[highscores_idx+i].score
						);
					}
					display_string(i+1, buffer);
				}

				display_update(light);

				// scroll down
				if ( btn2 ) {
					highscores_idx += 3;
					// wrap around at the end
					if ( highscores_idx >= HIGHSCORES_LEN ) {
						highscores_idx = 0;
					}
				// scroll up if not at the top
				} else if ( btn3 && highscores_idx > 0 ) {
					highscores_idx -= 3;
				// go back to main menu and reset highscore scrolling
				} else if ( btn4 ) {
					highscores_idx = 0;
					game.state = MainMenu;
				}
				
			break;
		}
		// wait for timer and update user input
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
