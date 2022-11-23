#include <stdint.h>
#include <string.h>
#include "game.h"
#include <stdlib.h>

#define MAX_Y 31
#define MAX_X 127

#define PIPE_MIN_LOWER 3
#define PIPE_MAX_LOWER 20
#define PIPE_MIN_GAP 10
#define PIPE_WIDTH 5

const float player_vel_limit_down = -20;
const float player_jump_vel = 12;
const float player_gravity = -20.0;
const float pipe_speed = - 3;

// make stdlib work...
void *stdout = (void *) 0;

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

	if ( y < 0 || y > MAX_Y ) {
		;
		// TODO handle the player losing
		// maybe different things happen when hitting ceiling vs floor?
	}
	player->y = y;
};

void draw_player(Flax *player, uint8_t screen[32][128]) {
	uint8_t x = (uint8_t) player->x;
	uint8_t y = MAX_Y - (uint8_t) player->y; // low indexes are top of screen

	screen[y][x] = 1;
	if ( player->vel < 0 ) { // TODO is it trying to draw too big/small indexes?
		screen[y-1][x+1] = 1;
		screen[y-1][x-1] = 1;
	} else {
		screen[y+1][x+1] = 1;
		screen[y+1][x-1] = 1;
	}
};

/*
Generate two random numbers, and create a pipe out of that information.
One number for the gap between the pipes,
and one number for the height of the lower pipe
*/
void spawn_pipe(PipePair *pipes, int *pipes_len) { // TODO seed random...
	int lower = PIPE_MIN_LOWER + rand() % (PIPE_MAX_LOWER - PIPE_MIN_LOWER);
	int upper = MAX_Y - rand() % ( MAX_Y - lower - PIPE_MIN_GAP);

	PipePair pair;
	pair.upper_edge = upper;
	pair.lower_edge = lower;
	pair.left_border = MAX_X;
	pair.right_border = MAX_X + PIPE_WIDTH - 1;
	
	*pipes_len += 1;
	pipes[*pipes_len-1] = pair;
}

void move_pipes(PipePair *pipes, int *pipes_len, float dt) {
	for (int i=0; i < *pipes_len; ++i) {
		pipes[i].left_border += pipe_speed * dt;
		pipes[i].right_border += pipe_speed * dt;
	}
	
	if ( pipes[0].right_border < 0 ) { // remove first pipe (out of frame) 
		for ( int i = 0; i < *pipes_len-1; ++i ) {
			pipes[i] = pipes[i+1];
		}
		*pipes_len -= 1;
	}
}

void draw_pipes(PipePair *pipes, int pipes_len, uint8_t screen[32][128]) {
	for (int i=0; i<pipes_len; ++i) {
		uint8_t left = (uint8_t) pipes[i].left_border;
		uint8_t right = (uint8_t) pipes[i].right_border;
		uint8_t upper = MAX_Y - (uint8_t) pipes[i].upper_edge;
		uint8_t lower = MAX_Y - (uint8_t) pipes[i].lower_edge;

		// draw pipe borders
		for (int y=0; y<MAX_Y; ++y) {
			if ( y <= upper || y >= lower ) {
				if ( right <= MAX_X ) {
					screen[y][right] = 1;
				}
				if ( left >= 0 ) {
					screen[y][left] = 1;
				}
			}
		}
	
		// draw pipe edges
		for ( int x = left + 1; x < right; ++x ) {
			if ( x <= 127 && x >= 0 ) {
				screen[upper][x] = 1;
				screen[lower][x] = 1;
			}
		}
	}
}

void update_game(Game *game, float dt) {
	perform_gravity(&game->player, dt);
	move_player(&game->player, dt);
	move_pipes(game->pipes, &game->pipes_len, dt);
}

void draw_game(Game *game) {
	memset(game->screen, 0, 4096);
	draw_player(&game->player, game->screen);
	draw_pipes(game->pipes, game->pipes_len, game->screen);
};

