#include "subserver.h"
#include <stdio.h>
#include "../common/messages.h"
#include "database.h"
#include <string.h>
#include <stdlib.h>

static void* updateClientThread(void* client_thread_args) {
    ClientThreadArgs* args = (ClientThreadArgs*)client_thread_args;
    StatsMessage stats_msg = { *args->attacker, *args->defender };
    AttackMessage atk_msg;

    sendMessage(args->sockfd, stats_msg);
    if(recvMessage(args->sockfd, atk_msg) <= 0) {
        printf("A client has disconnected\n");
        atk_msg.attack_index = -1;
    }

    args->attack_index_out = atk_msg.attack_index;

    return NULL;
}

void initClientThread(ClientThreadArgs* args, int sockfd, Stats* attacker, Stats* defender) {
    args->sockfd = sockfd;
    args->attacker = attacker;
    args->defender = defender;
    args->attack_index_out = -1;
}

pthread_t updateClient(ClientThreadArgs* args) {
    pthread_t tid;

    if(pthread_create(&tid, NULL, updateClientThread, (void*)args)) {
        perror("Error starting updateClientThread");
    }

    return tid;
}

void startLogin(int sockfd1, int sockfd2, char** uid1_out, char** uid2_out) {
    // threads for logging in each user
    pthread_t p1;
    pthread_t p2;

    // create struct for each user with sockfd
    LoginThreadArgs arg1 = {sockfd1};
    LoginThreadArgs arg2 = {sockfd2};
    
    // launch each user's login thread
    if(pthread_create(&p1, NULL, loginClientThread, (void*)&arg1)) {
        perror("Error starting loginClientThread for p1 on sockfd1");
    }
    if(pthread_create(&p2, NULL, loginClientThread, (void*)&arg2)) {
        perror("Error starting loginClientThread for p2 on sockfd2");
    }

    // wait for both users to login
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    // access the uid's for both users from game thread
    *uid1_out = arg1.uid_out;
    *uid2_out = arg2.uid_out;

}

void* loginClientThread(void* login_thread_args) {
    LoginThreadArgs* args = (LoginThreadArgs*)login_thread_args;
    LoginMessage login_msg = {LOGIN_INVALID};
       
    // buffers for unique id and password
    char* uid_buff = (char*)malloc(24);
    char pwd_buff[24];

    // buffers for name and password check
    char *name_buff = NULL;
    char *pwd_check;

    // singal to client we need a login
    sendMessage(args->sockfd, login_msg);

    while(login_msg.login_status == LOGIN_INVALID) {

        // recieve usernamer and pass, store in buffer
        recvString(args->sockfd, uid_buff, USER_STRING_LENGTH);
        recvString(args->sockfd, pwd_buff, USER_STRING_LENGTH);
        
        // put unique id into LoginThreadArgs struct
        args->uid_out = uid_buff;
        
        // try and open uid file and see if it exists
        FILE* uid_file_ptr = openUserFileRead(args->uid_out);

        // no uid file found, create file (NEW USER)
        if(uid_file_ptr == NULL) {
            // send login status to client
            login_msg.login_status = LOGIN_NEW;
            sendMessage(args->sockfd, login_msg);

            name_buff = (char*)malloc(24);

            // ask for name
            recvString(args->sockfd, name_buff, USER_STRING_LENGTH); // recieve name
            
            // create the uid file
            uid_file_ptr = openUserFileWrite(args->uid_out);

            // write new file with user info
            writeUserFile(uid_file_ptr, name_buff, pwd_buff);

            fflush(uid_file_ptr);

            free(name_buff);
        }
        // EXISTING USER
        else {
            // get password
            pwd_check = readUserPassword(uid_file_ptr);
            
            // while password is not valid
            if(strcmp(pwd_check, pwd_buff) != 0) {
                // send status 0 == invalid login
                login_msg.login_status = LOGIN_INVALID;
                sendMessage(args->sockfd, login_msg);
                continue;
            }
            else {
                // password is valid, send success and store 
                // username for sending
                login_msg.login_status = LOGIN_EXISTING;
                sendMessage(args->sockfd, login_msg);
                name_buff = readUserName(uid_file_ptr);
                // send username
                sendString(args->sockfd, name_buff, USER_STRING_LENGTH);
            }
        }

        closeUserFile(uid_file_ptr);
    }

    return 0;
}

