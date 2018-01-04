#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "connectionlist.h"
#include "../common/stats.h"

#define NUM_OF_PLAYERS 2

typedef struct {
    Stats player[NUM_OF_PLAYERS];
    ConnectionList* conn_list;
    char* uid[NUM_OF_PLAYERS];
} GameState;

void initGameState(GameState* game, ConnectionList* conn_list, char* uid1, char* uid2);
int isGameOver(GameState* game);

void gameLoop(GameState* game);

pthread_t createGameThread(ConnectionList* conn_list);

#endif
