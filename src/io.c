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

/* Interrupt Service Routine */
void user_isr() {
	if (IFS(0) & 0x100) {
		IFS(0) &= ~0x100;
		halted = 0;
	} 
}

/* IO initialization */
void io_init() {
	// io 
	TRISE &= ~0xff; // set as outputs
	PORTE &= ~0xff; // led 7-0 zero
	TRISD |= 0b111111100000; // 11 through 5 input sw
	// PORTD |= 0b111111100000; // 11 through 5 input instruction says so

	// Enable interrupts for timer2

	T2CON = ( T2CON & ~0b1110000 ) | 0b1110000; // initialize counter with 256 prescaling
	T2CON = T2CON & ~0b10; // tcs use internal clock source
	T2CON = T2CON & ~0x2000; // SIDL 0, stop in idle mode bit, 0: continue operation in idle mode
	//PR2 = 1250; // set timer period 25 hz
	PR2 = 12500;
	TMR2 = 0; // reset counter
	T2CONSET = 0x8000; // start timer

	IPC(2) = 4; // priority control
	IEC(0) = (1 << 8); // interrupt enable
	
	// TODO wat?
	// enable interrupts for sw2
	IEC(0) |= 0b100000000000; // enable
	IPC(2) = (IPC(2) & 0xE3FFFFFF) | (6 << 26);
	IPC(2) = (IPC(2) & 0xFAFFFFFF) | (1 << 24); 

	enable_interrupt();
}

int getsw() {
	return (PORTD >> 8) & 0xf;
}

int getbtns() {
	return (PORTD >> 5) & 0b111;
}

int timertest() {
	if (IFS(0) & 0x100) {
		IFS(0) = 0;
		return 1;
	}

	return 0;
}

int btn_is_pressed(int btn) {
	int btns = getbtns();
	switch (btn) {
		case 4:
		return ((btns & 0b100) == 0b100 );
	};
}
