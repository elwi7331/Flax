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

int main(void) {
	setup();
	
	// display image on screen
	uint8_t img_data[512];

	float dt = 1.0 / 25.0;

	Flax player;
	player.x = 10;
	player.y = 31;
	player.vel = 0;
	
	PipePair pipe;
	pipe.upper_edge = 20;
	pipe.lower_edge = 2;
	pipe.left_border = 100;
	pipe.right_border = 104;
	

	Game game;
	memset(game.screen, 0, 4096);
	game.player = player;
	game.pipes[0] = pipe;
	game.pipes_len = 1;
	game.state = Playing;

	spawn_pipe(game.pipes, &game.pipes_len);
	
	halted = 1;
	io_init();

	display_init();
	draw_game(&game);
	image_to_data(game.screen, img_data);
	display_image(img_data);
	
	while( 1 ) {
		draw_game(&game);
		image_to_data(game.screen, img_data);
		display_image(img_data);
		update_game(&game, dt);
		
		if (btn_is_pressed(4)) {
			jump(&game.player);
		}

		if ( game.player.y < 1 ) {
			jump(&game.player);
		}
		
		// wait for timer
		while ( halted != 0 ) {
			if ( game.player.y < 5 ) {
				jump(&game.player);
			}
		}
		halted = 1;

	}
	return 0;
}
