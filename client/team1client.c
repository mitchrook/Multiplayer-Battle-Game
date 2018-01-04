#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "../common/stats.h"
#include "../common/messages.h"

#define HOST "server1.cs.scranton.edu" 	// the hostname of the game server
#define PORT "17100"   		            // Assigned port by Dr. Bi

#define MAX_LOGIN 20
#define MAX_PASSWORD 20

void printStats(Stats *p1, Stats *p2, const char* name1, const char* name2);
int promptAttack();
void printMoves();
char *login(int clientSocket);
void printRecords(char *username, char *player2Name, RecordMessage* recs);

char *attackNames[NUM_OF_ATTACKS] = {"strike", "fireball", "heal" };

int main(int argc, char* argv[]) {
    RecordMessage recs[2];
    StatsMessage stats_msg;
    AttackMessage atk_msg;
    AttackMessage player1Attack;
    AttackMessage player2Attack;
    int clientSocket;
    int isPlayer = 0;
    int is_connection_found = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char *userName;
    char player1Name[20] = "";
    char player2Name[20] = "";
    const char* host_name;
    char choice_buf[2];

    //allow user to specify host name
    //or use default
    if(argc > 1) {
        host_name = argv[1];
    }
    else {
        host_name = HOST;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host_name, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((clientSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(clientSocket, p->ai_addr, p->ai_addrlen) == -1) {
            close(clientSocket);
            perror("client: connect");
            continue;
        }
        
        is_connection_found = 1;
        break;
    }

    if(!is_connection_found) {
        exit(1);
    }
    
    do {
        printf("Do you want to PLAY (p) or SPECTATE (s)?");
        scanf("%1s", choice_buf);

        choice_buf[0] = (char)tolower((int)choice_buf[0]);

        if (choice_buf[0] == 'p') {
            isPlayer = CLIENT_PLAYER;
        } else if (choice_buf[0] == 's') {
            isPlayer = CLIENT_SPECTATOR;
        }
    } while (isPlayer != CLIENT_PLAYER && isPlayer != CLIENT_SPECTATOR);

    sendInt(clientSocket, isPlayer);

    if(isPlayer == CLIENT_PLAYER) {
        printf("Welcome Challenger!\n");
        printf("Waiting for your adversary to enter the arena...\n\n");

        // login
        userName = login(clientSocket);
        recvString(clientSocket, player2Name, MAX_LOGIN);
        printf("\nGodspeed %s!\n", userName);
        printf("You are challenged by %s!\n\n", player2Name);
    } else if(isPlayer == CLIENT_SPECTATOR) {
        printf("Waiting for the challengers to enter the arena...\n\n");
        recvString(clientSocket, player1Name, MAX_LOGIN);
        recvString(clientSocket, player2Name, MAX_LOGIN);
        printf("You are spectating %s battling %s!\n", player1Name, player2Name);
    } else {
        printf("you should not have gotten here");
    }
    
    while(1) {
        recvMessage(clientSocket, stats_msg);
        if(isPlayer == CLIENT_PLAYER) {
            printStats(&stats_msg.player1, &stats_msg.player2, "You", "Opponent");
        }
        else {
            printStats(&stats_msg.player1, &stats_msg.player2, player1Name, player2Name);
        }

        // if player
        if (isPlayer == CLIENT_PLAYER) {
            printMoves();
            if(stats_msg.player1.hp <= 0 || stats_msg.player2.hp <= 0) {
                if(stats_msg.player1.hp <=0) {
                    if(stats_msg.player2.hp <= 0) {
                        printf("YOU BOTH PERISHED MISERABLY!\n");
                    }
                    else {
                        printf("YOU WERE DEFEATED!\n");
                    }
                }
                else {
                    printf("YOU ARE VICTORIOUS!\n");
                }
                break;
            }

            //get and send attack
            atk_msg.attack_index = promptAttack();
            sendMessage(clientSocket, atk_msg);
            printf("Waiting for the right moment to act...\n\n");

            //print out both player's attacks
            recvMessage(clientSocket, player2Attack);
            printf("You used %s!\n", attackNames[atk_msg.attack_index]);
            printf("Your opponent used %s!\n\n", attackNames[player2Attack.attack_index]);
        } else if (isPlayer == CLIENT_SPECTATOR) {
            // do spectator stuff
            if(stats_msg.player1.hp <= 0 || stats_msg.player2.hp <= 0) {
                if(stats_msg.player1.hp <=0) {
                    if(stats_msg.player2.hp <= 0) {
                        printf("BOTH PLAYERS PERISHED MISERABLY!\n");
                    }
                    else {
                        printf("%s WAS VICTORIOUS!\n", player2Name);
                        printf("%s WAS DEFEATED!\n", player1Name);
                    }
                }
                else {
                    printf("%s WAS VICTORIOUS!\n", player1Name);
                    printf("%s WAS DEFEATED!\n", player2Name);
                }
                break;
            }

            recvMessage(clientSocket, player1Attack);
            recvMessage(clientSocket, player2Attack);

            printf("%s used %s!\n", player1Name, attackNames[player1Attack.attack_index]);
            printf("%s used %s!\n\n", player2Name, attackNames[player2Attack.attack_index]);
        } else {
            printf("should not have gotten to this point...again");
        }
    }

    recvMessage(clientSocket, recs[0]);
    recvMessage(clientSocket, recs[1]);

    if(isPlayer == CLIENT_PLAYER) {
        printRecords(userName, player2Name, recs);
        free(userName);
    } else if (isPlayer == CLIENT_SPECTATOR) {
        printRecords(player1Name, player2Name, recs);
    }

    close(clientSocket);
    return 0;
}

int promptAttack() {
    char s[20];
    int i;

    while(1) {
        printf("Choose your next move> "); 
        scanf("%s", s);

        for(i = 0; s[i]; i++){
            s[i] = tolower((int)s[i]);
        }

        for(i = 0; i < NUM_OF_ATTACKS; i++) {
            if(strcmp(attackNames[i], s) == 0) {
                return i;
            }
        }

        printf("What is this \"%s\" move you speak of!?!\n", s);
    }
    return -1;
}

void printStat(const char *stat_name, int stat1, int stat2) {
    printf("%-6s|%5d%5s%6d\n", stat_name, stat1, "|", stat2);
}

void printStats(Stats *p1, Stats *p2, const char* name1, const char* name2) {
    printf("%13s:%16s:\n", name1, name2);
    printf("------------------------------\n");
    printStat("HP:", p1->hp, p2->hp);
    printStat("ATK:", p1->atk, p2->atk);
    printStat("DEF:", p1->def, p2->def);
    printStat("MAG:", p1->magic, p2->magic);
    printStat("MP:", p1->mana, p2->mana);
    printf("\n");
}

void printMoves() {
    printf("%-12s%-10s%s\n", "Move", "MP Cost", "Description");
    printf("----------------------------------------\n");
    printf("%-12s%-10d%s\n", "Strike", 0,"Basic melee attack");
    printf("%-12s%-10d%s\n", "Fireball", FIREBALL_COST, "Shoots a fireball");
    printf("%-12s%-10d%s\n", "Heal", HEAL_COST, "Heal some HP");
    printf("\n");
}

char *login(int clientSocket) {
    char uniqueID[MAX_LOGIN];
    char password[MAX_PASSWORD];
    char *userName;
    LoginMessage validLogin = { 0 };

    userName = (char *) malloc(MAX_LOGIN * sizeof(char));
    userName[0] = '\0';

    recvMessage(clientSocket, validLogin);
    while(validLogin.login_status == LOGIN_INVALID) {
        printf("Enter your unique ID (max of %d characters): ", MAX_LOGIN);
        scanf("%s", uniqueID);
        printf("Enter your password (max of %d characters): ", MAX_PASSWORD);
        scanf("%s", password);

        sendString(clientSocket, uniqueID, MAX_LOGIN);
        sendString(clientSocket, password, MAX_PASSWORD);

        recvMessage(clientSocket, validLogin);
        switch(validLogin.login_status) {
            case LOGIN_INVALID:
                printf("invalid login, try again\n");
                break;
            case LOGIN_NEW:
                printf("you must register first.\n");
                printf("Enter your username (max of %d characters): ", MAX_LOGIN);
                scanf("%s", userName);
                sendString(clientSocket, (const void*)userName, MAX_LOGIN);
                break;      
            case LOGIN_EXISTING:
                printf("successfully logged in!\n");
                recvString(clientSocket, (void*)userName, MAX_LOGIN);
                break;
            default:
                printf("something went wrong, try again %d\n", validLogin.login_status);
        }
    }

    return userName;
}

void printRecords(char *username, char *player2Name, RecordMessage* recs) {
    printf("                    |  Wins |  Ties | Loses\n");
    printf("--------------------------------------------\n");
    printf("%-20s|%7d|%7d|%7d\n", username, recs[0].wins, recs[0].ties, recs[0].loses);
    printf("%-20s|%7d|%7d|%7d\n", player2Name, recs[1].wins, recs[1].ties, recs[1].loses);
}
