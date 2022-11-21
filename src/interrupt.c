/* mipslabwork.c

	 This file written 2015 by F Lundevall
	 Updated 2017-04-21 by F Lundevall

	 This file should be changed by YOU! So you must
	 add comment(s) here with your name(s) and date(s):

	 This file modified 2017-04-31 by Ture Teknolog 

	 For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "boilerplate.h"  /* Declatations for these labs */

int mytime = 0x5957;

char textstring[] = "text, more text, and even more text!";
int timeoutcount = 0;
int prime = 1234567;

/* Interrupt Service Routine */
void user_isr() {
	uint8_t img[512];

	if (IFS(0) & 0x100) {
		IFS(0) &= ~0x100;

		if (++timeoutcount == 10) {
			
			time2string( textstring, mytime );
			display_string( 3, textstring );
			tick( &mytime );
			PORTE = ( PORTE + 1 ) & 0xff; // LED increment
			timeoutcount = 0;
		}
	} 
	if (IFS(0) & 0x800) {
		IFS(0) &= ~0x800;
		mytime += 3;
	}
}

/* Lab-specific initialization goes here */
void labinit() {
	// io 
	TRISE &= ~0xff; // set as outputs
	PORTE &= ~0xff; // led 7-0 zero
	TRISD |= 0b111111100000; // 11 through 5 input sw
	// PORTD |= 0b111111100000; // 11 through 5 input instruction says so
	
	// Enable interrupts for timer2

	T2CON = ( T2CON & ~0b1110000 ) | 0b1110000; // initialize counter with 256 prescaling
	T2CON = T2CON & ~0b10; // tcs use internal clock source
	T2CON = T2CON & ~0x2000; // SIDL 0
	PR2 = 31250; // set timer period
	TMR2 = 0; // reset counter

	T2CONSET = 0x8000; // start timer

	IPC(2) = 4;
	IEC(0) = (1 << 8);
	
	// enable interrupts for sw2
	IEC(0) |= 0b100000000000; // enable
	IPC(2) = (IPC(2) & 0xE3FFFFFF) | (6 << 26);
	IPC(2) = (IPC(2) & 0xFAFFFFFF) | (1 << 24); 

	enable_interrupt();
}

int timertest() {
	if (IFS(0) & 0x100) {
		IFS(0) = 0;
		return 1;
	}

	return 0;
}

/* This function is called repetitively from the main program */
void labwork() { // TODO remove
	// display_update();
	return;
}
