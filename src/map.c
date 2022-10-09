#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>


#include "const.h"


int load_map(char *filepath, struct map_information_t *map) {
    if (filepath == NULL || map == NULL) {
        return -1;
    }

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        return -2;
    }

    int row = 0;
    char line[52] = {'\0'};
    while (fscanf(fp, "%[^\n] ", line) == 1) {
        for (int i = 0; i < 52; i++) {
            if (*(line + i) == ' ') {
                *(*(map->board + row) + i) = AIR;
            } else if (*(line + i) == 'O') {
                *(*(map->board + row) + i) = WALL;
            } else if (*(line + i) == '#') {
                *(*(map->board + row) + i) = BUSH;
            }
        }
        row++;
    }
}

void print_map(struct map_information_t *map, WINDOW *win) {
    refresh();
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 51; j++) {
            if (map->board[i][j] == AIR) {
                mvwprintw(win, i + 1, j + 1, " ");
            } else if (map->board[i][j] == WALL) {
                wattron(win, COLOR_PAIR(1));
                mvwprintw(win, i + 1, j + 1, "O");
                wattron(win, COLOR_PAIR(2));
            } else if (map->board[i][j] == BUSH) {
                mvwprintw(win, i + 1, j + 1, "#");
            }
        }

    }

    wrefresh(win);
    refresh();
}