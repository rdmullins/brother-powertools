
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game_2048.h"

/* --------------------------------------------------
 * Internal helpers
 * -------------------------------------------------- */

static void spawn_tile(Game2048 *game)
{
    int empty_count = 0;

    for (int r = 0; r < GAME_2048_SIZE; r++) {
        for (int c = 0; c < GAME_2048_SIZE; c++) {
            if (game->board[r][c] == 0) {
                empty_count++;
            }
        }
    }

    if (empty_count == 0)
        return;

    int target = rand() % empty_count;

    for (int r = 0; r < GAME_2048_SIZE; r++) {
        for (int c = 0; c < GAME_2048_SIZE; c++) {
            if (game->board[r][c] == 0) {
                if (target == 0) {
                    game->board[r][c] =
                        (rand() % 10 == 0) ? 4 : 2;
                    return;
                }
                target--;
            }
        }
    }
}

/*
 * Map a position in a movement line to
 * its corresponding board coordinates.
 *
 * Position zero is always the leading edge
 * in the direction of movement.
 */
static void get_position(
    MoveDirection direction,
    int line,
    int position,
    int *row,
    int *col)
{
    switch (direction) {
        case MOVE_LEFT:
            *row = line;
            *col = position;
            break;

        case MOVE_RIGHT:
            *row = line;
            *col = GAME_2048_SIZE - 1 - position;
            break;

        case MOVE_UP:
            *row = position;
            *col = line;
            break;

        case MOVE_DOWN:
            *row = GAME_2048_SIZE - 1 - position;
            *col = line;
            break;
    }
}

/*
 * Slide and merge one row or column.
 * Returns true if anything changed.
 */
static bool process_line(
    Game2048 *game,
    MoveDirection direction,
    int line)
{
    int values[GAME_2048_SIZE] = {0};
    int compact[GAME_2048_SIZE] = {0};
    int count = 0;
    int score_added = 0;
    bool changed = false;

    /* Read the line, starting at the leading edge. */
    for (int i = 0; i < GAME_2048_SIZE; i++) {
        int row, col;

        get_position(direction, line, i, &row, &col);
        values[i] = game->board[row][col];

        if (values[i] != 0)
            compact[count++] = values[i];
    }

    /* Merge matching adjacent tiles. */
    int output[GAME_2048_SIZE] = {0};
    int out = 0;

    for (int i = 0; i < count; i++) {
        if (i + 1 < count && compact[i] == compact[i + 1]) {
            output[out] = compact[i] * 2;
            score_added += output[out];
            i++;
        } else {
            output[out] = compact[i];
        }

        out++;
    }

    /* Write the resulting line back to the board. */
    for (int i = 0; i < GAME_2048_SIZE; i++) {
        int row, col;

        get_position(direction, line, i, &row, &col);

        if (game->board[row][col] != output[i])
            changed = true;

        game->board[row][col] = output[i];
    }

    game->score += score_added;

    return changed;
}

/* --------------------------------------------------
 * Public game engine
 * -------------------------------------------------- */

void game_2048_init(Game2048 *game)
{
    if (game == NULL)
        return;

    for (int r = 0; r < GAME_2048_SIZE; r++) {
        for (int c = 0; c < GAME_2048_SIZE; c++) {
            game->board[r][c] = 0;
        }
    }

    game->score = 0;
    game->game_over = false;

    spawn_tile(game);
    spawn_tile(game);
}

bool game_2048_move_only(Game2048 *game, MoveDirection direction)
{
    if (game == NULL || game->game_over)
        return false;

    bool changed = false;

    for (int line = 0; line < GAME_2048_SIZE; line++) {
        if (process_line(game, direction, line))
            changed = true;
    }

    game->game_over = game_2048_is_over(game);

    return changed;
}

bool game_2048_move(Game2048 *game, MoveDirection direction)
{
    if (!game_2048_move_only(game, direction))
        return false;

    spawn_tile(game);

    game->game_over = game_2048_is_over(game);

    return true;
}

bool game_2048_is_over(const Game2048 *game)
{
    if (game == NULL)
        return true;

    for (int r = 0; r < GAME_2048_SIZE; r++) {
        for (int c = 0; c < GAME_2048_SIZE; c++) {
            if (game->board[r][c] == 0)
                return false;

            if (r + 1 < GAME_2048_SIZE &&
                game->board[r][c] == game->board[r + 1][c])
                return false;

            if (c + 1 < GAME_2048_SIZE &&
                game->board[r][c] == game->board[r][c + 1])
                return false;
        }
    }

    return true;
}

/* --------------------------------------------------
 * Terminal display
 * -------------------------------------------------- */

static void draw_board(const Game2048 *game)
{
    printf("\n");
    printf("================================\n");
    printf("              2048\n");
    printf("================================\n\n");

    printf(" SCORE: %d\n\n", game->score);

    for (int r = 0; r < GAME_2048_SIZE; r++) {
        printf("+------+------+------+------+\n");

        for (int c = 0; c < GAME_2048_SIZE; c++) {
            if (game->board[r][c] == 0)
                printf("|      ");
            else
                printf("| %4d ", game->board[r][c]);
        }

        printf("|\n");
    }

    printf("+------+------+------+------+\n\n");

    printf(" W = UP       A = LEFT\n");
    printf(" S = DOWN     D = RIGHT\n");
    printf(" Q = QUIT\n\n");
}

/* --------------------------------------------------
 * Game entry point
 * -------------------------------------------------- */

void game_2048(void)
{
    Game2048 game;
    char input;

    srand((unsigned int)time(NULL));

    game_2048_init(&game);

    while (1) {
        draw_board(&game);

        if (game.game_over) {
            printf("GAME OVER! Final score: %d\n", game.score);
            printf("Press R to restart or Q to quit: ");
        } else {
            printf("Your move: ");
        }

        if (scanf(" %c", &input) != 1)
            break;

        if (input == 'q' || input == 'Q')
            break;

        if (game.game_over) {
            if (input == 'r' || input == 'R') {
                game_2048_init(&game);
                continue;
            }

            continue;
        }

        switch (input) {
            case 'w':
            case 'W':
                game_2048_move(&game, MOVE_UP);
                break;

            case 'a':
            case 'A':
                game_2048_move(&game, MOVE_LEFT);
                break;

            case 's':
            case 'S':
                game_2048_move(&game, MOVE_DOWN);
                break;

            case 'd':
            case 'D':
                game_2048_move(&game, MOVE_RIGHT);
                break;

            default:
                continue;
        }
    }

    printf("\nThanks for playing 2048!\n");
}
