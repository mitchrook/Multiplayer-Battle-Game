#ifndef CONNECTION_LIST_H
#define CONNECTION_LIST_H
#include <pthread.h>

typedef struct Connection {
    int sockfd;
    int state;
    struct Connection* prev;
    struct Connection* next;
} Connection;

typedef struct {
    Connection* list;
    pthread_mutex_t lock;
} ConnectionList;

void initConnectionList(ConnectionList* list);
void destroyConnectionList(ConnectionList* list);
void lockConnectionList(ConnectionList* list);
void unlockConnectionList(ConnectionList* list);

Connection* createConnection(ConnectionList* list, int sockfd);
void destroyConnection(Connection* conn);

void insertConnection(ConnectionList* list, Connection* conn);
void removeConnection(ConnectionList* list, Connection* conn);

void startConnectionThread(ConnectionList* list, Connection* conn);

#endif
