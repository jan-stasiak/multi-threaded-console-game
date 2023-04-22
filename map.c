#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

#include "map.h"


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

    return 0;
}

void print_map(struct server_information_t *s, struct map_information_t *map, WINDOW *win) {
    refresh();


    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 51; j++) {
            if (map->board[i][j] == AIR) {
                mvwprintw(win, i + 1, j + 1, " ");
            } else if (map->board[i][j] == WALL) {
                wattron(win, COLOR_PAIR(WALL));
                mvwprintw(win, i + 1, j + 1, "O");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == BUSH) {
                mvwprintw(win, i + 1, j + 1, "#");
            } else if (map->board[i][j] == COIN) {
                wattron(win, COLOR_PAIR(COIN));
                mvwprintw(win, i + 1, j + 1, "C");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == TREASURE) {
                wattron(win, COLOR_PAIR(COIN));
                mvwprintw(win, i + 1, j + 1, "t");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == LARGE_TREASURE) {
                wattron(win, COLOR_PAIR(COIN));
                mvwprintw(win, i + 1, j + 1, "T");
                wattron(win, COLOR_PAIR(NORM));
            }
        }
    }

    wattron(win, COLOR_PAIR(CAMP));
    mvwprintw(win, map->campsite.y + 1, map->campsite.x + 1, "A");
    wattron(win, COLOR_PAIR(NORM));


    for (int i = 0; i < 4; ++i) {
        if (s->players[i].connected == true) {
            wattron(win, COLOR_PAIR(PLAYER));
            char player_num[2] = {'\0'};
            sprintf(player_num, "%d", i + 1);
            mvwprintw(win, s->players[i].coordinate.y + 1, s->players[i].coordinate.x + 1, "%s", player_num);
            wattron(win, COLOR_PAIR(NORM));
        }
    }

    for (int i = 0; i < 4; ++i) {
        if (s->beast[i].exist) {
            wattron(win, COLOR_PAIR(BEAST));
            mvwprintw(win, s->beast[i].coordinate.y + 1, s->beast[i].coordinate.x + 1, "*");
            wattron(win, COLOR_PAIR(NORM));
        }
    }

    for (int i = 0; i < MAX_DROP_TREASURE; ++i) {
        if (s->treasure[i].is_free == false) {
            if (s->treasure[i].coordinate.y != 0 && s->treasure[i].coordinate.x != 0) {
                wattron(win, COLOR_PAIR(COIN));
                mvwprintw(win, s->treasure[i].coordinate.y + 1, s->treasure[i].coordinate.x + 1, "D");
                wattron(win, COLOR_PAIR(NORM));
            }

        }
    }
    wattron(win, COLOR_PAIR(WALL));
    mvwprintw(win, 0, 0, "O");
    wattron(win, COLOR_PAIR(NORM));
    box(win, 0, 0);
    wrefresh(win);
    refresh();
    wattron(win, COLOR_PAIR(2));

}