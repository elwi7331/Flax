/* game.h
 * Definitions for game.c
*/
#include <stdint.h>

#define MAX_Y 31
#define MAX_X 127

enum GameState {
	MainMenu,
	HighScoreMenu,
	Playing,
	GameOver 
};
typedef enum GameState GameState;

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

struct Game {
	GameState state;
	Flax player;
	uint8_t screen[32][128];
	PipePair pipes[20];
	int pipes_len;
};
typedef struct Game Game;

void jump(Flax *player);
void update_game(Game *game, float dt);
void draw_game(Game *game);
void spawn_pipe(PipePair *pipes, int *pipes_len);