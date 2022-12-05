#include <stdint.h>
#include <string.h>
#include "game.h"
#include <stdlib.h>

#define PIPE_MIN_GAP 9
#define PIPE_MAX_GAP 26
#define PIPE_MIN_LOWER 2
#define PIPE_MAX_LOWER 18
#define PIPE_MAX_UPPER 29
#define PIPE_WIDTH 5

const float player_vel_limit_down = -20;
const float player_jump_vel = 12;
const float player_gravity = -20.0;

void *stdout = (void*) 0; // is needed when stdio is not included
int passed_pipe = 0; // used in update_game for determining when to update highscore


/**
 * @brief Returns a random number within given range (inclusive)
 * 
 * @param lower the lower bound
 * @param upper the upper bound (inclusive)
 * @return uint32_t random number
 */
uint32_t randrng(uint32_t lower, uint32_t upper) {
	return lower + rand() % (upper - lower + 1);
}

/**
 * @brief Makes Flax jump by setting his
 * velocity to a specific value
 * 
 * @param player Flax will be mutated
 */
void jump(Flax *player) {
	player->vel = player_jump_vel;
}

/**
 * @brief Updates the vertical velocity of player to simulate gravity
 * 
 * @param player Flax will be mutated
 * @param dt The time passed since this function was last
 * called. Will probably be "game time" between frames
 */
void perform_gravity(Flax *player, float dt) {
	player->vel += dt * player_gravity;
	if ( player->vel < player_vel_limit_down ) {
		player->vel = player_vel_limit_down;
	}
};


/**
 * @brief Moves the player by its own velocity
 * 
 * @param player Flax will be mutated
 * @param dt The time passed since this function was last
 * called. Will probably be "game time" between frames
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

/**
 * @brief Draws Flax
 * 
 * @param player Flax
 * @param screen the screen array
 */
void draw_player(Flax *player, uint8_t screen[32][128]) {
	uint8_t x = (uint8_t) player->x;
	uint8_t y = MAX_Y - (uint8_t) player->y; // low indexes are top of screen

	screen[y][x] = 1;
	if ( player->vel < 0 ) {
		screen[y-1][x+1] = 1;
		screen[y-1][x-1] = 1;
	} else {
		screen[y+1][x+1] = 1;
		screen[y+1][x-1] = 1;
	}
};

/**
 * @brief Randomly generate pipes
 * 
 * @param pipes Array containing the pipes
 * @param pipes_len Length of the pipe array, can be modified 
 * @param movement_type Type of pipe
 * @param speed Only applicable for non static movement type
 * @param x The x coordinate of the left side of the pipe
 */
void spawn_pipe(PipePair *pipes, int *pipes_len, PipeMovementType movement_type, float speed, int x) {
	PipePair pair;
	pair.left = x;
	pair.right = x + PIPE_WIDTH - 1;
	pair.speed = speed;
	pair.movement_type = movement_type;

	int max_lower;
	
	switch ( movement_type ) {
		case Uniform:
			// range [4, 18]
			// pair.lower_upper = rand() % (PIPE_MAX_LOWER - PIPE_MIN_LOWER + 1 - 2) + PIPE_MIN_LOWER + 2;
			pair.lower_upper = randrng(PIPE_MIN_LOWER + 2, PIPE_MAX_LOWER);

			// range [2, lower_upper]
			// pair.lower_lower = rand() % ((int) pair.lower_upper - PIPE_MIN_LOWER + 1 - 2) + PIPE_MIN_LOWER;
			pair.lower_lower = randrng(PIPE_MIN_LOWER, (int) pair.lower_upper);

			// range [lower_upper + PIPE_MIN_GAP, 29]
			// pair.upper_upper = rand() % (MAX_Y - (int) pair.lower_upper - PIPE_MIN_GAP - PIPE_MIN_LOWER + 1) + (int) pair.lower_upper + PIPE_MIN_GAP;
			pair.upper_upper = randrng( (int) pair.lower_upper + PIPE_MIN_GAP, MAX_Y - PIPE_MIN_LOWER );
			// same movement range as lower pipe
			pair.upper_lower = pair.upper_upper - (pair.lower_upper - pair.lower_lower);

			// spawn in upper or lower position
			if ( rand() % 2 == 0 ) {
				pair.upper = pair.upper_upper;
				pair.lower = pair.lower_upper;
				pair.direction = Down;
			} else {
				pair.upper = pair.upper_lower;
				pair.lower = pair.lower_lower;
				pair.direction = Up;
			}

		break;

		case Squeezing:
			max_lower = (MAX_Y - PIPE_MIN_GAP) / 2;

			pair.lower_upper = rand() % (max_lower - PIPE_MIN_LOWER + 1 - 2) + PIPE_MIN_LOWER + 2;
			pair.upper_lower = MAX_Y - pair.lower_upper;

			pair.lower_lower = rand() % ((int) pair.lower_upper - PIPE_MIN_LOWER + 1) + PIPE_MIN_LOWER;
			pair.upper_upper = MAX_Y - pair.lower_lower;

			// spawn in inner or outer position
			if ( rand() % 2 == 0 ) {
				pair.upper = pair.upper_lower;
				pair.lower = pair.lower_upper;
				pair.direction = Out;
			} else {
				pair.upper = pair.upper_upper;
				pair.lower = pair.lower_lower;
				pair.direction = In;
			}

		break;

		case Static:
			pair.lower = PIPE_MIN_LOWER + rand() % (PIPE_MAX_LOWER - PIPE_MIN_LOWER);
			pair.upper = pair.lower + PIPE_MIN_GAP + rand() % (PIPE_MAX_UPPER - (int) pair.lower - PIPE_MIN_GAP);
			pair.direction = Still; // this does nothing
		break;
	}

	*pipes_len += 1;
	pipes[*pipes_len-1] = pair;
}

/**
 * @brief Move all pipes given speed and time passed
 * also delete the first pipe if it is out of frame
 * 
 * @param pipes array pointing to the pipes
 * @param pipes_len amount of pipes, can be modified
 * @param dt The time that has passed
 */
void move_pipes(PipePair *pipes, int *pipes_len, float horizontal_speed, float dt) {
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
		pipes[i].left+= horizontal_speed * dt;
		pipes[i].right+= horizontal_speed * dt;
	}
	
	if ( pipes[0].right < 0 ) { // remove first pipe (out of frame) 
		for ( int i = 0; i < *pipes_len-1; ++i ) {
			pipes[i] = pipes[i+1];
		}
		*pipes_len -= 1;
	}
}

/**
 * @brief Draw all pipes on screen
 * 
 * @param pipes The array of pipes
 * @param pipes_len Amount of pipes
 * @param screen The array representing the screen
 */
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

/**
 * @brief determine if player has any of the pipes
 * 
 * @param player Flax
 * @param pipes array containing the pipes
 * @param pipes_len len of pipes
 * @return int 1 if the player has hit a pipe, else 0
 */
int flax_hits_pipe(Flax player, PipePair *pipes, int pipes_len) {
	for ( int i = 0; i < pipes_len; ++i ) {
		if (
			(player.y < pipes[i].lower+1 || player.y > pipes[i].upper-1) 
			&& (player.x - 1 < pipes[i].right+1 && player.x + 1 > pipes[i].left-1)
		) {
			return 1;
		}
	}
	return 0;
}

/**
 * @brief updates the game state, given the time that has passed 
 * 
 * @param game The game struct
 * @param dt time that has passed since last update
 * @return 1 if game over, else 0
 */
int update_game(Game *game, float dt) {
	int pipe_speed;
	int moving_pipe_probability;
	
	uint8_t score = game->score;
	pipe_speed = -3 -0.5*score;

	perform_gravity(&game->player, dt);
	move_player(&game->player, dt);
	move_pipes(game->pipes, &game->pipes_len, pipe_speed, dt);
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

	// spawn a new pipe
	if ( game->pipes[game->pipes_len-1].right < MAX_X - PIPE_SPACING ) {
		moving_pipe_probability = 20 + 5 * (score / 2);

		int rand = randrng(1, 100);
		PipeMovementType pipe_type;

		if ( rand <= moving_pipe_probability ) {
			if ( randrng(0,1) == 0 ) {
				pipe_type = Squeezing;
			} else {
				pipe_type = Uniform;
			}
		} else {
			pipe_type = Static;
		}

		spawn_pipe(game->pipes, &game->pipes_len, pipe_type, DEFAULT_DYNAMIC_PIPE_SPEED, MAX_X);
	}

	return 0;
}

/**
 * @brief Set the default game state struct
 * 
 * @param game The game struct
 */
void set_default_game_state(Game *game) {
	game->player.x = PLAYER_START_X;
	game->player.y = PLAYER_START_Y;
	game->player.vel = 0;
	game->pipes_len = 0;
	game->score = 0;

	for ( int i = 0; i < 4; ++i ) {
		spawn_pipe(game->pipes, &game->pipes_len, Static, DEFAULT_DYNAMIC_PIPE_SPEED, PIPE_START_X + i*(PIPE_SPACING + PIPE_WIDTH));
	}

	memset(game->screen, 0, 4096);
}

/**
 * @brief draw all elements from game
 * 
 * @param game The game struct
 */
void draw_game(Game *game) {
	memset(game->screen, 0, 4096);
	draw_player(&game->player, game->screen);
	draw_pipes(game->pipes, game->pipes_len, game->screen);
};

