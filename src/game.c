#include <stdint.h>
#include <string.h>
#include "game.h"
#include <stdlib.h>


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

	if ( y > (float) (MAX_Y)) {
		player->y = MAX_Y;
		player->vel = 0;
	} else {
		player->y = y;
	}
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
void spawn_pipe(PipePair *pipes, int *pipes_len, int x) { // TODO seed random...
	// int lower = PIPE_MIN_LOWER + rand() % (PIPE_MAX_LOWER - PIPE_MIN_LOWER);
	// int upper = MAX_Y - rand() % ( MAX_Y - lower - PIPE_MIN_GAP);

	PipePair pair;
	pair.upper_upper = 24;
	pair.upper_lower = 20;
	pair.lower_upper = 10;
	pair.lower_lower = 6;

	pair.direction = Up;
	pair.movement_type = Uniform;

	pair.upper = pair.upper_lower;
	pair.lower = pair.lower_lower;
	pair.left = x;
	pair.right = x + PIPE_WIDTH - 1;
	pair.speed = DEFAULT_DYNAMIC_PIPE_SPEED;

	*pipes_len += 1;
	pipes[*pipes_len-1] = pair;

}

/* function move_pipes
move all pipes given speed and time passed
also delete the first pipe if it is out of frame

args:
	PipePair *pipes: array pointing to the pipes
	int *pipes_len: len of the array
	float dt: the time that has passed
*/
//void move_pipes(PipePair *pipes, int *pipes_len, float dt) {
//	for (int i=0; i < *pipes_len; ++i) {
//		pipes[i].left_border += pipe_speed * dt;
//		pipes[i].right_border += pipe_speed * dt;
//	}
//	
//	if ( pipes[0].right_border < 0 ) { // remove first pipe (out of frame) 
//		for ( int i = 0; i < *pipes_len-1; ++i ) {
//			pipes[i] = pipes[i+1];
//		}
//		*pipes_len -= 1;
//	}
//}

void move_pipes(PipePair *pipes, int *pipes_len, float dt) {
	for (int i=0; i < *pipes_len; ++i) {
		// vertical movement
		
		switch ( pipes[i].movement_type ) {
			case Squeezing:
				if ( pipes[i].lower < pipes[i].lower_lower ) pipes[i].direction = In;
				if ( pipes[i].lower > pipes[i].lower_upper ) pipes[i].direction = Out;

				if ( pipes[i].direction == In ) {
					pipes[i].upper -= pipes[i].speed * dt;
					pipes[i].lower += pipes[i].speed * dt;
				} else {
					pipes[i].upper += pipes[i].speed * dt;
					pipes[i].lower -= pipes[i].speed * dt;
				}
			break;
			
			case Uniform:
				if ( pipes[i].lower < pipes[i].lower_lower ) pipes[i].direction = Up;
				if ( pipes[i].lower > pipes[i].lower_upper ) pipes[i].direction = Down;
				
				if ( pipes[i].direction == Up ) {
					pipes[i].upper += pipes[i].speed * dt;
					pipes[i].lower += pipes[i].speed * dt;
				} else {
					pipes[i].upper -= pipes[i].speed * dt;
					pipes[i].lower -= pipes[i].speed * dt;
				}
			break;
			
			case Static:
				;
			break;
		}

		// horizontal movement
		pipes[i].left+= pipe_speed * dt;
		pipes[i].right+= pipe_speed * dt;
	}
	
	if ( pipes[0].right < 0 ) { // remove first pipe (out of frame) 
		for ( int i = 0; i < *pipes_len-1; ++i ) {
			pipes[i] = pipes[i+1];
		}
		*pipes_len -= 1;
	}
}

void draw_pipes(PipePair *pipes, int pipes_len, uint8_t screen[32][128]) {
	for (int i=0; i<pipes_len; ++i) {
		int left = (int) pipes[i].left;
		int right = (int) pipes[i].right;
		int upper = MAX_Y - (uint8_t) pipes[i].upper;
		int lower = MAX_Y - (uint8_t) pipes[i].lower;

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

/*function flax_hits_pipe
returns 1 if the player has hit one of the pipes,
otherwise false.

Flax hitbox is 3x1:
--------
--xxx---
--------

None of the arguments is mutated
*/
int flax_hits_pipe(Flax player, PipePair *pipe, int pipes_len) {
	for ( int i = 0; i < pipes_len; ++i ) {
		if (
			(player.y < pipe[i].lower+1 || player.y > pipe[i].upper-1) 
			&& (player.x - 1 < pipe[i].right+1 && player.x + 1 > pipe[i].left-1)
		) {
			return 1;
		}
	}
	return 0;
}

// used in update_game for determining when to update highscore
int passed_pipe = 0;
/* function update_game
updates the game state.

args:
	Game *game,
	flaot dt (time passsed),
return: 
	1 if game over, else 0.
*/
int update_game(Game *game, float dt) {
	perform_gravity(&game->player, dt);
	move_player(&game->player, dt);
	move_pipes(game->pipes, &game->pipes_len, dt);
	if (flax_hits_pipe(game->player, game->pipes, game->pipes_len)) {
		return 1;
	} else if ( game->player.y < 0 ) {
		game->player.y = 0;
		return 1;
	}

	if (game->player.x > game->pipes[0].left && game->player.x < game->pipes[0].right) {
		passed_pipe = 0;
	} else if ( game->pipes[0].right < game->player.x && passed_pipe == 0) {
		game->score++;
		passed_pipe = 1;
	}
	return 0;
}

void set_default_game_state(Game *game) {
	game->player.x = PLAYER_START_X;
	game->player.y = PLAYER_START_Y;
	game->player.vel = 0;
	game->pipes_len = 0;
	game->score = 0;

	for ( int i = 0; i < 4; ++i ) {
		spawn_pipe(game->pipes, &game->pipes_len, PIPE_START_X + i*(PIPE_SPACING + PIPE_WIDTH));
	}

	memset(game->screen, 0, 4096);
}

void draw_game(Game *game) {
	memset(game->screen, 0, 4096);
	draw_player(&game->player, game->screen);
	draw_pipes(game->pipes, game->pipes_len, game->screen);
};

