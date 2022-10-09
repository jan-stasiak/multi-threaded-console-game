#include "const.h"
#include <ncurses.h>

void print_map(struct map_information_t *map, WINDOW *win);

int load_map(char *filepath, struct map_information_t *map);
