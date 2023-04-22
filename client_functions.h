#ifndef MULTI_THREADED_CONSOLE_GAME_CLIENT_FUNCTIONS_H
#define MULTI_THREADED_CONSOLE_GAME_CLIENT_FUNCTIONS_H

#include <ncurses.h>

void create_names(char *sem_name, char *shm_name, size_t num);

void display_init_client(struct display_server_t *dis);

void print_client_game(struct map_information_t *map, WINDOW *win);

void *print_game(void *args);

void colors_init();

void *key_handler_client(void *args);

#endif //MULTI_THREADED_CONSOLE_GAME_CLIENT_FUNCTIONS_H
