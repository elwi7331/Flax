#include <stdint.h>

const float player_vel_limit_down = 42; // TODO set
const float player_vel_limit_up = 42; // TODO set
const float player_gravity = 42; // TODO set, should be negative

struct Flax {
	float x;
	float y;
	float vel;
};
typedef struct Flax Flax;

struct Game{
	Flax player;
	uint8_t screen[32][128];
};
typedef struct Game Game;

/* function jump
update the vertical velocity of player
vel = vel + amount

Argument player: the instance of Flax representing the player,
will be mutated

Argument amount: how much the speed should be incremented
*/
// TODO should this functino set or increment the vel?
void jump(Flax *player, float amount) {
	player->vel += amount;

	if ( player->vel > player_vel_limit_up ) {
		player->vel = player_vel_limit_up;
	}
};

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
	player->y = player->y + player->vel * dt;
};

void draw_player(Flax *player, uint8_t screen[32][128]) {
	uint8_t x = (uint8_t) player->x;
	uint8_t y = 31 - (uint8_t) player->y; // low indexes are top of screen

	screen[y][x] = 1;
	screen[y-1][x+1] = 1;
	screen[y-1][x-1] = 1;
};

void draw_game(Game *game) {
	draw_player(&(game->player), game->screen);
}

