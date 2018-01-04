#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/types.h>
#include <sys/socket.h>
#include "stats.h"

typedef struct {
    int attack_index;
} AttackMessage;

typedef struct {
    Stats player1;
    Stats player2;
} StatsMessage;

typedef struct {
    int wins, loses, ties;
} RecordMessage;

typedef struct {
    int login_status;
} LoginMessage;

enum {
    LOGIN_INVALID = 0,
    LOGIN_NEW,
    LOGIN_EXISTING,
};

enum {
    CLIENT_INVALID = 0,
    CLIENT_PLAYER,
    CLIENT_SPECTATOR,
};

#define sendMessage(sockfd, msg) send(sockfd, (const void*)(&(msg)), sizeof(msg), 0)
#define recvMessage(sockfd, msg) recv(sockfd, (void*)(&(msg)), sizeof(msg), 0)

void sendString(int sockfd, const char* str, int len);
void recvString(int sockfd, char* str, int len);

void sendInt(int sockfd, int val);
int recvInt(int sockfd);

#endif
