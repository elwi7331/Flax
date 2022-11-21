/* game.h
 * Definitions for game.c
*/
#include <stdint.h>

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

void jump(Flax *player, float amount);
void perform_gravity(Flax *player, float dt);
void move_player(Flax *player, float dt);
void draw_game(Game *game);