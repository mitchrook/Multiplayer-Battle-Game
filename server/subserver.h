#ifndef SUBSERVER_H
#define SUBSERVER_H

#include <pthread.h>
#include "../common/stats.h"

typedef struct {
    int sockfd;
    Stats* attacker;
    Stats* defender;
    int attack_index_out;
} ClientThreadArgs;

typedef struct {
    int sockfd;
    char* uid_out;
} LoginThreadArgs;

void initClientThread(ClientThreadArgs* args, int sockfd, Stats* attacker, Stats* defender);
pthread_t updateClient(ClientThreadArgs* args);

void startLogin(int sockfd1, int sockfd2, char** uid1_out, char** uid2_out);
void* loginClientThread(void* login_thread_args);

#endif
