SOURCE_DIR = src
CC = gcc
CFLAGS = -w -pthread -I.
LIBS = -pthread -lncurses -lrt
DEPS = map.h server_functions.h const.h client_functions.h

%.o: %.c $(DEPS)
	$(CC) -w -g -c -o $@ $< $(CFLAGS)
	
all: server client

server: server.o server_functions.o map.o
	$(CC) -w -g -o server server.c server_functions.o map.o $(LIBS)
	
client: client.o client_functions.o map.o
	$(CC) -w -g -o client client.c client_functions.o map.o $(LIBS)

.PHONY: clean

clean:
	rm server client server_functions.o map.o client.o server.o client_functions.o
