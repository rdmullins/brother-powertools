
#ifndef GAME_2048_H
#define GAME_2048_H

#include <stdbool.h>

#define GAME_2048_SIZE 4

typedef struct {
    int board[GAME_2048_SIZE][GAME_2048_SIZE];
    int score;
    bool game_over;
} Game2048;

typedef enum {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT
} MoveDirection;

/* Game engine */
void game_2048_init(Game2048 *game);
bool game_2048_move(Game2048 *game, MoveDirection direction);
bool game_2048_move_only(Game2048 *game, MoveDirection direction);
bool game_2048_is_over(const Game2048 *game);

/* Game interface */
void game_2048(void);

#endif
