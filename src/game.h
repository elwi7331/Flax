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

/*
---|-----|------
---x=====x------ y = upper_edge
----------------
---x=====x------ y = lower_edge
---|-----|------
   left_border
         right_border
*/
struct PipePair {
	float left_border;
	float right_border;
	float upper_edge;
	float lower_edge;
};
typedef struct PipePair PipePair;

struct Game{
	Flax player;
	uint8_t screen[32][128];
	PipePair pipes;
	int pipes_len;
};
typedef struct Game Game;

void jump(Flax *player);
void update_game(Game *game, float dt);
