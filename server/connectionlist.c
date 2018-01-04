#include "connectionlist.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gamestate.h"
#include "../common/messages.h"

void initConnectionList(ConnectionList* list) {
    list->list = NULL;
    pthread_mutex_init(&list->lock, NULL);
}

void destroyConnectionList(ConnectionList* list) {
    Connection* conn_destroy;
    Connection* temp;

    // free each connection in list
    conn_destroy = list->list;
    while(conn_destroy != NULL) {
        temp = conn_destroy->next;
        destroyConnection(conn_destroy);
        conn_destroy = temp;
    }

    // once list is empty, free the list
    free(list);
}

void lockConnectionList(ConnectionList* list) {
    pthread_mutex_lock(&list->lock);
}

void unlockConnectionList(ConnectionList* list) {
    pthread_mutex_unlock(&list->lock);
}

Connection* createConnection(ConnectionList* list, int sockfd) {
    Connection* conn = (Connection*)malloc(sizeof(Connection));
    conn->sockfd = sockfd;
    conn->state = CLIENT_INVALID;
    conn->next = NULL;
    conn->prev = NULL;

    insertConnection(list, conn);
    return conn;
}

void destroyConnection(Connection* conn) {
    close(conn->sockfd);
    free(conn);
}

void insertConnection(ConnectionList* list, Connection* conn) {
    conn->next = list->list;
    conn->prev = NULL;
    if(list->list != NULL) {
        list->list->prev = conn;
    }
    list->list = conn;
}

void removeConnection(ConnectionList* list, Connection* conn) {
    if(conn->next) {
        conn->next->prev = conn->prev;
    }

    if(conn->prev) {
        conn->prev->next = conn->next;
    }
    else {
        list->list = conn->next;
    }

    conn->prev = NULL;
    conn->next = NULL;
}

typedef struct {
    ConnectionList* list;
    Connection* conn;
} ConnThreadArgs;

static void* connectionThread(void* args) {
    ConnThreadArgs* conn_args = (ConnThreadArgs*)args;
    int num_players = 0;
    Connection* it;
    Connection* temp;
    int i;
    Connection* players[2];
    ConnectionList* game_list;
    
    int client_state = recvInt(conn_args->conn->sockfd);

    lockConnectionList(conn_args->list);

    conn_args->conn->state = client_state;
    if(client_state == CLIENT_PLAYER) {
        for(it = conn_args->list->list; it != NULL; it = it->next) {
            if(it->state == CLIENT_PLAYER) {
                players[num_players++] = it;
            }
        }

        //if 2 players, start game
        //with all players and spectators only (no invalid)
        if(num_players == NUM_OF_PLAYERS) {
            game_list = (ConnectionList*)malloc(sizeof(ConnectionList));
            initConnectionList(game_list);

            //move spectators to game_list
            it = conn_args->list->list;
            while(it != NULL) {
                temp = it->next;
                if(it->state == CLIENT_SPECTATOR) {
                    removeConnection(conn_args->list, it);
                    insertConnection(game_list, it);
                }
                it = temp;
            }

            //insert player connections at the start of the list
            for(i = 0; i < NUM_OF_PLAYERS; i++) {
                removeConnection(conn_args->list, players[i]);
                insertConnection(game_list, players[i]);
            }

            createGameThread(game_list);
        }
    }

    unlockConnectionList(conn_args->list);

    free(conn_args);

    return NULL;
}

void startConnectionThread(ConnectionList* list, Connection* conn) {
    pthread_t tid;
    ConnThreadArgs args = { list, conn };

    ConnThreadArgs* pargs = (ConnThreadArgs*)malloc(sizeof(ConnThreadArgs));
    *pargs = args;

    if(pthread_create(&tid, NULL, connectionThread, (void*)pargs)) {
        perror("Error starting startConnectionThread");
    }
}
