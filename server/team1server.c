#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "gamestate.h"
#include "connectionlist.h"

#define HOST "127.0.0.1" // the hostname of the game server
#define PORT "17100"     // the port client will be connecting to

#define BACKLOG 10     // how many pending connestions queue will hold

int getServerSocket(char *host, char *port); // get a server socket
void *getInAddr(struct sockaddr *sa);        // get internet address
void reapTerminatedChild(int status);        // reap all dead processes
void startSubserver(int new_fd);             // start subserver as thread
void *subserver(void *new_fd);               // subserver thread

int main(int argc, char* argv[]) {
    int sockfd, new_fd; // listen on sockfd, new connection on new_fd
    struct sockaddr_storage their_addr; // connector's address info
    socklen_t sin_size;           // length/size of sockets
    char s[INET6_ADDRSTRLEN];     // address of connecting client

    ConnectionList list; // struct for list of connections

    srand(time(NULL));  // get new random seed
    
    // make socket and bind it to start server
    sockfd = getServerSocket(HOST, PORT);
    
    // check to see if you can listen on server
    if(listen(sockfd, BACKLOG) == -1) {
        printf("error: server did not start correctly");
        exit(1);
    }

    // initialize the client thread
    initConnectionList(&list); 

    // now accept incomming connections
    while(1) {
        printf("server: waiting for connections...\n");

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1) {
            // did not accept a client
            perror("accept");
            continue;
        }
        else {
            // accepted a client
            inet_ntop(their_addr.ss_family, 
            getInAddr((struct sockaddr *)&their_addr), s, sizeof s);
            printf("server: got connection from %s\n", s); 
            // create the connection, add to list, and start connThread
            lockConnectionList(&list);
            Connection* conn = createConnection(&list, new_fd);
            unlockConnectionList(&list);
            startConnectionThread(&list, conn);
        }
    }
    return 0;
}

int getServerSocket(char *host, char *port) {
    struct addrinfo hints, *servinfo, *p;
    int rv;             // return value for getaddrinfo();
    int sockfd;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //AF_UNSPEC;   <==Bi's temp fix**
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // create the socket
        if((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        // re-use if port is not in use
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("set socket option");
            exit(1);
        }
        // bind socket to HOST and PORT
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if(listen(sockfd, BACKLOG) == -1) {
       perror("listen");
       exit(1);
    }

    return sockfd;
}

void *getInAddr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
