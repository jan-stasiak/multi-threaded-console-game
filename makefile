CFLAGS = -lpthread -lrt -lncurses -g
SOURCE_DIR = src

SERVER_INCLUDES = map.c


server: $(SOURCE_DIR)/server.c
	gcc $(SOURCE_DIR)/server.c $(SOURCE_DIR)/$(SERVER_INCLUDES) -o ./server $(CFLAGS)
	
clean:
	rm server
