#include <stdint.h>
#include <string.h>
#include "game.h"

#define MAX_Y 31

const float player_vel_limit_down = -20; // TODO set
const float player_jump_vel = 20;
const float player_gravity = -20.0; // TODO set, should be negative

/* function jump
update the vertical velocity of player
vel = vel + amount

Argument player: the instance of Flax representing the player,
will be mutated

Argument amount: how much the speed should be incremented
*/
// TODO should this functino set or increment the vel?
void jump(Flax *player) {
	player->vel = player_jump_vel;
}

/* function perform_gravity
update the vertical velocity of player to simulate gravity

Argument player: the instance of Flax representing the player,
will be mutated

argument dt: how much time has passed since this function was last
called. Will probably be "game time" between frames
*/
void perform_gravity(Flax *player, float dt) {
	player->vel += dt * player_gravity;
	if ( player->vel < player_vel_limit_down ) {
		player->vel = player_vel_limit_down;
	}
};

/* function move_player
move the player by its own velocity.

Argument player: the instance of Flax representing the player,
will be mutated

Argument dt: time that has passed
*/
void move_player(Flax *player, float dt) {
	float y = player->y + player->vel * dt;

	if ( y < 0 || y > MAX_Y ) {
		;
		// TODO handle the player losing
		// maybe different things happen when hitting ceiling vs floor?
	}
	player->y = y;
};

void draw_player(Flax *player, uint8_t screen[32][128]) {
	uint8_t x = (uint8_t) player->x;
	uint8_t y = 31 - (uint8_t) player->y; // low indexes are top of screen

	screen[y][x] = 1;
	screen[y-1][x+1] = 1;
	screen[y-1][x-1] = 1;
};

void draw_game(Game *game) {
	memset(game->screen, 0, 4096);
	draw_player(&game->player, game->screen);
};

