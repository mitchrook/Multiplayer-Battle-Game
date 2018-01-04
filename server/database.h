#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <stdio.h>

/* User Data File Format 
 Filename is the user's unique id
 Offset | Size | Description
    00h |  14h | User display name
    14h |  14h | Password
    28h |  04h | Wins
    2Ch |  04h | Loses
    30h |  04h | Ties
*/

#define FILENAME_MAX_LENGTH 32
#define USER_DIR "userdata/"

#define USER_STRING_LENGTH 20
#define USER_NAME 0x0
#define USER_PASSWORD 0x14
#define USER_WINS 0x28
#define USER_LOSES 0x2C
#define USER_TIES 0x30

FILE* openUserFileRead(const char* uid);
FILE* openUserFileWrite(const char* uid);
FILE* openUserFileUpdate(const char* uid);
void closeUserFile(FILE* f);

void writeUserFile(FILE* f, const char* name, const char* password);

//Remember to free strings when done
char* readUserPassword(FILE* f);
char* readUserName(FILE* f);

void writeUserInt(FILE* f, size_t offset, int value);
int readUserInt(FILE* f, size_t offset);

#endif
