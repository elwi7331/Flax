#include <stdint.h>
#include <string.h>
#include "game.h"

#define MAX_Y 31

const float player_vel_limit_down = -20;
const float player_jump_vel = 12;
const float player_gravity = -20.0;
const float pipe_speed = - 3;

/* function jump
update the vertical velocity of player
vel = amount

Argument player: the instance of Flax representing the player,
will be mutated

Argument amount: how much the speed should be 
*/
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

	// TODO Flax shouldn't be able to jump higher than than the screen edge

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

// TODO if pipe is completely out of frame, remove it from array and shift the rest
void move_pipes(PipePair *pipes, int pipes_len, float dt) {
	for (int i=0; i<pipes_len; ++i) {
		pipes[i].left_border += pipe_speed * dt;
		pipes[i].right_border += pipe_speed * dt;
	}
}

// TODO negative indexes that are bigger and smaller than map (draw parital pipe)
void draw_pipes(PipePair *pipes, int pipes_len, uint8_t screen[32][128]) {
	for (int i=0; i<pipes_len; ++i) {
		uint8_t left = (uint8_t) pipes[i].left_border;
		uint8_t right = (uint8_t) pipes[i].right_border;
		uint8_t upper = 31 - (uint8_t) pipes[i].upper_edge;
		uint8_t lower = 31 - (uint8_t) pipes[i].lower_edge;

		// draw pipe borders
		for (int y=0; y<31; ++y) {
			if ( y <= upper || y >= lower ) {
				screen[y][left] = 1;
				screen[y][right] = 1;
			}
		}
		// draw upper pipe edge
		screen[upper][left+1] = 1;
		screen[upper][left+2] = 1;
		screen[upper][right-1] = 1;

		// draw lower pipe edge
		screen[lower][left+1] = 1;
		screen[lower][left+2] = 1;
		screen[lower][right-1] = 1;
	}
}

void update_game(Game *game, float dt) {
	perform_gravity(&game->player, dt);
	move_player(&game->player, dt);
	move_pipes(game->pipes, game->pipes_len, dt);
}

void draw_game(Game *game) {
	memset(game->screen, 0, 4096);
	draw_player(&game->player, game->screen);
	draw_pipes(game->pipes, game->pipes_len, game->screen);
};

