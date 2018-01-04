#include "messages.h"

void sendString(int sockfd, const char* str, int len){
    send(sockfd, str, len, 0);
}

void recvString(int sockfd, char* str, int len) {
    int recv_len = 0;
    while(recv_len < len) {
        recv_len += recv(sockfd, str, len - recv_len, 0);
    }
}

void sendInt(int sockfd, int val) {
    send(sockfd, &val, sizeof(val), 0);
}

int recvInt(int sockfd) {
    int buf;
    recv(sockfd, &buf, sizeof(buf), 0);
    return buf;
}
