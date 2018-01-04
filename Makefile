
CLIENT_TARGET := team1client
SERVER_TARGET := team1server

USER_DIR := userdata

CLIENT_SRC := client common
SERVER_SRC := server common

CLIENT_C_FILES := $(foreach dir, $(CLIENT_SRC), $(wildcard $(dir)/*.c))
SERVER_C_FILES := $(foreach dir, $(SERVER_SRC), $(wildcard $(dir)/*.c))

CLIENT_H_FILES := $(foreach dir, $(CLIENT_SRC), $(wildcard $(dir)/*.h))
SERVER_H_FILES := $(foreach dir, $(SERVER_SRC), $(wildcard $(dir)/*.h))

CC := cc
FLAGS := -g -Wall
FLAGS_SERVER := -lpthread

all: $(CLIENT_TARGET) $(SERVER_TARGET) $(USER_DIR)

$(CLIENT_TARGET) : $(CLIENT_C_FILES) $(CLIENT_H_FILES)
	$(CC) $(FLAGS) -o $@ $(CLIENT_C_FILES)

$(SERVER_TARGET) : $(SERVER_C_FILES) $(SERVER_H_FILES)
	$(CC) $(FLAGS) $(FLAGS_SERVER) -o $@ $(SERVER_C_FILES)

$(USER_DIR) :
	mkdir -p $@

clean:
	rm $(CLIENT_TARGET)
	rm $(SERVER_TARGET)
