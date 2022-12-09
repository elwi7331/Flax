/* mipslab.h
   Header file for all labs.
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

/* Display related functions */
void display_image(uint8_t *data);
void display_init(void);
void display_string(int line, char *s);
void display_update(int light);
uint8_t spi_send_recv(uint8_t data);

/* Declare lab-related functions from mipslabfunc.c */
char * itoaconv( int num );

/* Declare display_debug - a function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr );

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare text buffer for display output */
extern char textbuffer[4][16];

void image_to_data(uint8_t image[32][128], uint8_t *data, int light);
void io_init();
int sw_is_toggled(int);
int btn_is_pressed(int);
int getsw(void);
void enable_interrupt(void);
void write_led(uint8_t num);
void set_timer_period(uint16_t period);

int halted;
