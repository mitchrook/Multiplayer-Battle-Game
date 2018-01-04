#include "gamestate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../common/messages.h"
#include "database.h"
#include "statsserver.h"
#include "subserver.h"

void initGameState(GameState* game, ConnectionList* conn_list, char* uid1, char* uid2) {
    int i;
    for(i = 0; i < NUM_OF_PLAYERS; i++) {
        generateStats(&game->player[i]);
    }

    game->conn_list = conn_list;

    game->uid[0] = uid1;
    game->uid[1] = uid2;
}

int isGameOver(GameState* game) {
    return (game->player[0].hp <= 0 || game->player[1].hp <= 0) ? 1 : 0;
}

static void sendStatsMessage(int sockfd, Stats* attacker, Stats* defender) {
    StatsMessage msg = { *attacker, *defender };
    sendMessage(sockfd, msg);
}

void gameLoop(GameState* game) {
    ClientThreadArgs args[NUM_OF_PLAYERS];
    pthread_t tid[NUM_OF_PLAYERS];
    int i, atk_idx;
    AttackMessage atk_msg;
    Connection* it;

    //player connections
    int connections[2];
    connections[0] = game->conn_list->list->sockfd;
    connections[1] = game->conn_list->list->next->sockfd;

    //the attacker for one client is the defender
    //for the other client and vice versa
    for(i = 0; i < NUM_OF_PLAYERS; i++) {
        initClientThread(&args[i], connections[i], &game->player[i], &game->player[i^1]);
    }

    //loop while at least one player
    //has hp remaining
    while(!isGameOver(game)) {
        //send current game state to clients
        //and get that player's attack choice
        //spectator just need the updated stats
        for(it = game->conn_list->list, i = 0; it != NULL; it = it->next) {
            if(it->state == CLIENT_PLAYER) {
                //the attacker for one client is the defender
                //for the other client and vice versa
                tid[i] = updateClient(&args[i]);
                i++;
            }
            else if(it->state == CLIENT_SPECTATOR) {
                //send stats to spectators
                StatsMessage stats_msg = { game->player[0], game->player[1] };
                sendMessage(it->sockfd, stats_msg);
            }
        }
        for(i = 0; i < NUM_OF_PLAYERS; i++) {
            pthread_join(tid[i], NULL);
        }
        
        //send each client their opponents attack choice
        for(it = game->conn_list->list, i = 0; it != NULL; it = it->next) {
            if(it->state == CLIENT_PLAYER) {
                //send each player their opponents attack choice
                atk_msg.attack_index = args[i^1].attack_index_out;
                sendMessage(args[i].sockfd, atk_msg);
                i++;
            }
            else if(it->state == CLIENT_SPECTATOR) {
                //send both attacks to spectators
                for(i = 0; i < NUM_OF_PLAYERS; i++) {
                    atk_msg.attack_index = args[i].attack_index_out;
                    sendMessage(it->sockfd, atk_msg);
                }
            }
        }

        //perform attacks
        for(i = 0; i < NUM_OF_PLAYERS; i++) {
            atk_idx = args[i].attack_index_out;
            if(isValidAttackIndex(atk_idx)) {
                attackFunctions[atk_idx](args[i].attacker, args[i].defender);
                printf("attack: %d\n", atk_idx);
            }
            else {
                printf("Invalid attack %d\n", atk_idx);

                //atk_idx is -1 when a play disconnects
                //so kill them off
                if(atk_idx == -1) {
                    game->player[i].hp = 0;
                }
            }
        }
    }

    //send final stats to clients
    for(it = game->conn_list->list, i = 0; it != NULL; it = it->next) {
        if(it->state == CLIENT_PLAYER) {
            //the attacker for one client is the defender
            //for the other client and vice versa
            sendStatsMessage(args[i].sockfd, args[i].attacker, args[i].defender);
            i++;
        }
        else if(it->state == CLIENT_SPECTATOR) {
            //send stats to spectators
            StatsMessage stats_msg = { game->player[0], game->player[1] };
            sendMessage(it->sockfd, stats_msg);
        }
    }
}

static char* loadUserName(const char* uid) {
    FILE* f = openUserFileRead(uid);
    char* user_name = readUserName(f);
    closeUserFile(f);
    return user_name;
}

static void* gameThread(void* args) {
    RecordMessage rec[2] = {
        { 0, 0, 0 },
        { 0, 0, 0 },
    };
    ConnectionList* conn_list = (ConnectionList*)args;
    Connection* it;
    int connections[2];
    GameState game;
    int i = 0;
    FILE* f;

    char* uid[2];
    char* usernames[2];

    //player connections
    connections[0] = conn_list->list->sockfd;
    connections[1] = conn_list->list->next->sockfd;

    //login
    startLogin(connections[0], connections[1], &uid[0], &uid[1]);

    //load usernames from files
    usernames[0] = loadUserName(uid[0]);
    usernames[1] = loadUserName(uid[1]);

    //send usernames
    for(it = conn_list->list, i = 0; it != NULL; it = it->next) {
        if(it->state == CLIENT_PLAYER) {
            //send opponent's username
            sendString(it->sockfd, usernames[i^1], USER_STRING_LENGTH);
            i++;
        }
        else if(it->state == CLIENT_SPECTATOR) {
            //send both usernames to spectators
            sendString(it->sockfd, usernames[0], USER_STRING_LENGTH);
            sendString(it->sockfd, usernames[1], USER_STRING_LENGTH);
        }
    }

    free(usernames[1]);
    free(usernames[0]);
    
    //start game
    initGameState(&game, conn_list, uid[0], uid[1]);
    gameLoop(&game);

    // once game is over, announce winner
    printf("Game over, ");
    if(game.player[0].hp <= 0) {
        if(game.player[1].hp <= 0) {
            printf("it's a tie\n");
            rec[0].ties++;
            rec[1].ties++;
        }
        else {
            printf("player 2 wins\n");
            rec[0].loses++;
            rec[1].wins++;
        }
        
    }
    else if(game.player[1].hp <= 0) {
        printf("player 1 wins\n");
        rec[0].wins++;
        rec[1].loses++;
    }

    //write player records
    for(i = 0; i < NUM_OF_PLAYERS; i++) {
        //read and modify records
        f = openUserFileUpdate(uid[i]);
        rec[i].wins += readUserInt(f, USER_WINS);
        rec[i].loses += readUserInt(f, USER_LOSES);
        rec[i].ties += readUserInt(f, USER_TIES);

        //write new records
        writeUserInt(f, USER_WINS, rec[i].wins);
        writeUserInt(f, USER_LOSES, rec[i].loses);
        writeUserInt(f, USER_TIES, rec[i].ties);
        closeUserFile(f);
    }

    //send your records first then opponents
    for(i = 0; i < NUM_OF_PLAYERS; i++) {
        sendMessage(connections[i], rec[i]);
        sendMessage(connections[i], rec[i^1]);
    }
    for(it = conn_list->list, i = 0; it != NULL; it = it->next) {
        if(it->state == CLIENT_PLAYER) {
            //send your record first
            sendMessage(connections[i], rec[i]);
            sendMessage(connections[i], rec[i^1]);
            i++;
        }
        else if(it->state == CLIENT_SPECTATOR) {
            //send both records to spectators
            sendMessage(it->sockfd, rec[0]);
            sendMessage(it->sockfd, rec[1]);
        }
    }

    free(uid[1]);
    free(uid[0]);

    // once game is over, close connections
    //allocated in connectionThread
    destroyConnectionList(conn_list);

    return NULL;
}

pthread_t createGameThread(ConnectionList* conn_list) {
    pthread_t tid;

    if(pthread_create(&tid, NULL, gameThread, (void*)conn_list)) {
        perror("Error starting gameThread");
    }

    return tid;
}
