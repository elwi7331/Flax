/* game.h
 * Definitions for game.c
*/
#include <stdint.h>

#define GAME_TIME_PERIOD 12500
#define MENU_TIME_PERIOD 65535

#define MAX_Y 31
#define MAX_X 127

#define PLAYER_START_X 10
#define PLAYER_START_Y 16

#define PIPE_START_X 40
#define PIPE_SPACING 20
#define PIPES_CAPACITY 20

#define HIGHSCORES_LEN 20

#define DEFAULT_DYNAMIC_PIPE_SPEED 3

enum PipeMovementType {
	Static,
	Squeezing,
	Uniform
};
typedef enum PipeMovementType PipeMovementType;

/* enum Direction
Used for telling which direction pipes are currently moving (vertically)
*/
enum Direction {
	In,
	Out,
	Up,
	Down,
	Still
};
typedef enum Direction Direction;

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
// struct PipePair {
// 	float left_border;
// 	float right_border;
// 	float upper_edge;
// 	float lower_edge;
// };
// typedef struct PipePair PipePair;

struct PipePair {
	float left;
	float right;
	float upper;
	float lower;

	float upper_upper;
	float upper_lower;
	float lower_upper;
	float lower_lower;

	PipeMovementType movement_type;
	Direction direction;
	float speed;
};
typedef struct PipePair PipePair;

struct Game {
	GameState state;
	Flax player;
	uint8_t screen[32][128];
	PipePair pipes[PIPES_CAPACITY];
	int pipes_len;
	uint8_t score;
};
typedef struct Game Game;

struct Highscore {
	char name[9];
	uint8_t score;
};
typedef struct Highscore Highscore;

void jump(Flax *player);
int update_game(Game *game, float dt);
void draw_game(Game *game);
void spawn_pipe(PipePair *pipes, int *pipes_len, int x);
void set_default_game_state(Game *game);
