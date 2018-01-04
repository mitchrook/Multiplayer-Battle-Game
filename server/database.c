#include "database.h"
#include <stdlib.h>
#include <string.h>

static char* userFileName(const char* uid) {
    char* buf = (char*)malloc(FILENAME_MAX_LENGTH);
    sprintf(buf, USER_DIR "%s", uid);
    return buf;
}

FILE* openUserFileRead(const char* uid) {
    char* fn = userFileName(uid);
    FILE* f = fopen(fn, "rb");
    free(fn);
    return f;
}

FILE* openUserFileWrite(const char* uid) {
    char* fn = userFileName(uid);
    FILE* f = fopen(fn, "wb");
    free(fn);
    return f;
}

FILE* openUserFileUpdate(const char* uid) {
    char* fn = userFileName(uid);
    FILE* f = fopen(fn, "r+b");
    free(fn);
    return f;
}

void writeUserFile(FILE* f, const char* name, const char* password) {
    //we'll assume that the name/password
    //being passed to this function are
    //shorter than USER_STRING_LENGTH bytes
    if(name == NULL) {
        fprintf(stderr, "Invalid name being writen to user file\n");
        fflush(stderr);
    }
    if(password == NULL) {
        fprintf(stderr, "Invalid password being writen to user file\n");
        fflush(stderr);
    }
    if(name == NULL || password == NULL) {
        return;
    }
    
    size_t name_len = strlen(name);
    size_t password_len = strlen(password);
    const char zero_buf[USER_STRING_LENGTH] = { 0 };

    printf("name: %s\npass: %s\n", name, password);

    if(name_len >= USER_STRING_LENGTH) name_len = USER_STRING_LENGTH - 1;
    if(password_len >= USER_STRING_LENGTH) password_len = USER_STRING_LENGTH - 1;

    fseek(f, 0, SEEK_SET);

    fwrite(name, 1, name_len, f);
    fwrite(zero_buf, 1, USER_STRING_LENGTH - name_len, f);

    fwrite(password, 1, password_len, f);
    fwrite(zero_buf, 1, USER_STRING_LENGTH - password_len, f);

    fwrite(zero_buf, 4, 3, f);
}

void closeUserFile(FILE* f) {
    fclose(f);
}

static char* readUserString(FILE* f, long ofs) {
    char* buf = (char*)malloc(USER_STRING_LENGTH + 1);
    fseek(f, ofs, SEEK_SET);
    fread(buf, 1, USER_STRING_LENGTH, f);
    return buf;
}

char* readUserPassword(FILE* f) {
    return readUserString(f, USER_PASSWORD);
}

char* readUserName(FILE* f) {
    return readUserString(f, USER_NAME);
}

void writeUserInt(FILE* f, size_t offset, int value) {
    fseek(f, offset, SEEK_SET);
    fwrite(&value, sizeof(int), 1, f);
}

int readUserInt(FILE* f, size_t offset) {
    int ret;
    fseek(f, offset, SEEK_SET);
    fread((void*)&ret, sizeof(int), 1, f);
    return ret;
}
