#include <ncurses.h>

#include "const.h"

int load_map(char *filepath, struct map_information_t *map);

void print_map(struct server_information_t* s, struct map_information_t *map, WINDOW *win);
