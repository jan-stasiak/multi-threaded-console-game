#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#include "const.h"


int main(void) {
    initscr();
    start_color();
    struct map_information_t map;
    WINDOW *win = newwin(27, 53, 1, 1);
    refresh();
    box(win, 0, 0);
    load_map(MAP_PATH, &map);
    print_map(&map, win);


    getch();
    endwin();

    return EXIT_SUCCESS;
} 