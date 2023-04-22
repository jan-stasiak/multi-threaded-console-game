#include <stdio.h>
#include <threads.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include "const.h"

#include "client_functions.h"


void create_names(char *sem_name, char *shm_name, size_t num) {
    sprintf(sem_name, "so2_player_sem_%zu", num);
    sprintf(shm_name, "so2_player_shm_%zu", num);
    return;
}

void display_init_client(struct display_server_t *dis) {
    WINDOW *g = newwin(27, 53, 0, 0);
    WINDOW *s = newwin(50, 50, 0, 53 + 2);
    dis->game = g;
    dis->stats = s;
    box(dis->game, 0, 0);
    wrefresh(dis->game);
    refresh();
}

void *key_handler_client(void *args) {
    struct client_struct_t *c = ((struct client_struct_t *) args);

    bool stop_loop = false;
    while (1) {
        if (c->c->status == false) {
            return NULL;
        }
        if (c->c->shm->is_server == false) {
            c->c->shm->want_to_disconnect = true;
            *c->c->status = false;
            sem_post(&c->c->shm->sem_shm);
            printf("Server closed!");
            return NULL;
        }
        int key = getch();
        sem_wait(c->c->sem);
        switch (key) {
            case 'w':
            case 'W':
            case KEY_UP: {
                c->c->shm->direction = UP;
                break;
            }
            case 'a':
            case 'A':
            case KEY_LEFT: {
                c->c->shm->direction = LEFT;
                break;
            }
            case 's':
            case 'S':
            case KEY_DOWN: {
                c->c->shm->direction = DOWN;
                break;
            }
            case 'd':
            case 'D':
            case KEY_RIGHT: {
                c->c->shm->direction = RIGHT;
                break;
            }
            case 'q':
            case 'Q': {
                *c->c->status = false;
                c->c->shm->direction = CLOSE;
                sem_post(c->t->sem);
                return NULL;
            }
        }
        sem_post(c->c->sem);
        usleep(1000);
    }
    return NULL;
}

void print_client_game(struct map_information_t *map, WINDOW *win) {
    wclear(win);
    clear();
    wrefresh(win);
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
            } else if (map->board[i][j] == PLAYER1) {
                wattron(win, COLOR_PAIR(PLAYER));
                mvwprintw(win, i + 1, j + 1, "1");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == PLAYER2) {
                wattron(win, COLOR_PAIR(PLAYER));
                mvwprintw(win, i + 1, j + 1, "2");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == PLAYER3) {
                wattron(win, COLOR_PAIR(PLAYER));
                mvwprintw(win, i + 1, j + 1, "3");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == PLAYER4) {
                wattron(win, COLOR_PAIR(PLAYER));
                mvwprintw(win, i + 1, j + 1, "4");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == CAMP) {
                wattron(win, COLOR_PAIR(CAMP));
                mvwprintw(win, i + 1, j + 1, "C");
                wattron(win, COLOR_PAIR(NORM));
            } else if (map->board[i][j] == BEAST) {
                wattron(win, COLOR_PAIR(BEAST));
                mvwprintw(win, i + 1, j + 1, "*");
                wattron(win, COLOR_PAIR(NORM));
            }
        }

    }

    box(win, 0, 0);
    wrefresh(win);
    refresh();
}

void print_legend(struct client_struct_t *c, WINDOW *win) {
    int y = 1, x = 0;
    char string[50] = {'\0'};
    refresh();
    wclear(win);
    refresh();

    sprintf(string, "Server's PID: %d", c->c->shm->server_pid);
    mvwprintw(c->t->dis->stats, y, x, string);

    if (c->c->shm->campsite.x == 0) {
        y = 2, x = 0;
        sprintf(string, " Campsite X/Y: %s", "unknown");
        mvwprintw(c->t->dis->stats, y++, x, string);
    } else {
        y = 2, x = 0;
        sprintf(string, " Campsite X/Y: %d/%d", c->c->shm->campsite.x, c->c->shm->campsite.y);
        mvwprintw(c->t->dis->stats, y++, x, string);
    }

    sprintf(string, "Round: %d", (int) c->c->shm->round);
    x = 0;
    mvwprintw(c->t->dis->stats, y++, x, string);

    y = 4, x = 0;
    sprintf(string, "Player:");
    mvwprintw(c->t->dis->stats, y++, x, string);
    sprintf(string, " Type: %s", "HUMAN");
    mvwprintw(c->t->dis->stats, y++, x, string);
    sprintf(string, " Curr X/Y: %d/%d", c->c->shm->player_coordinate.x, c->c->shm->player_coordinate.y);
    mvwprintw(c->t->dis->stats, y++, x, string);
    sprintf(string, " Deaths: %d", (int) c->c->shm->deaths);
    mvwprintw(c->t->dis->stats, y++, x, string);
    sprintf(string, " Coins found: %d", (int) c->c->shm->coin_found);
    mvwprintw(c->t->dis->stats, y++, x, string);
    sprintf(string, " Coins brought: %d", (int) c->c->shm->coin_brought);
    mvwprintw(c->t->dis->stats, y++, x, string);
    y+=3;

    mvwprintw(c->t->dis->stats, y++, x, "Legend: ");
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(PLAYER));
    mvwprintw(c->t->dis->stats, y, x, "1234");
    x += 4;
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));

    mvwprintw(c->t->dis->stats, y++, x, " - players");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(WALL));
    mvwprintw(c->t->dis->stats, y, x++, " ");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, "    - wall");
    x = 0;
    mvwprintw(c->t->dis->stats, y++, x, "  #    - bushes (slow down)");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(BEAST));
    mvwprintw(c->t->dis->stats, y, x++, "*");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, "    - wild beast");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(COIN));
    mvwprintw(c->t->dis->stats, y, x++, "C");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y, x, "    - one coin  ");
    x += 23;
    wattron(c->t->dis->stats, COLOR_PAIR(COIN));
    mvwprintw(c->t->dis->stats, y, x++, "D");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, " - dropped treasure ");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(COIN));
    mvwprintw(c->t->dis->stats, y, x++, "t");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, " - treasure (10 coins)");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(COIN));
    mvwprintw(c->t->dis->stats, y, x++, "T");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, " - large treasure (50 coins)");
    x = 0;
    mvwprintw(c->t->dis->stats, y, x, "  ");
    x += 2;
    wattron(c->t->dis->stats, COLOR_PAIR(CAMP));
    mvwprintw(c->t->dis->stats, y, x++, "A");
    wattron(c->t->dis->stats, COLOR_PAIR(NORM));
    mvwprintw(c->t->dis->stats, y++, x, " - campsite");

    wrefresh(win);
    refresh();
}

void *print_game(void *args) {
    struct client_struct_t *c = ((struct client_struct_t *) args);

    while (1) {
        sem_wait(c->c->sem);
        if (*c->t->status == false) {
            wclear(c->t->dis->game);
            wclear(c->t->dis->stats);
            refresh();
            mvprintw(0, 0, "Server closed! Push any button to end program.");
            refresh();
            sem_post(c->c->sem);
            return NULL;
        }

        print_client_game(c->t->map, c->t->dis->game);
        print_legend(c, c->t->dis->stats);
        sem_post(c->c->sem);
        usleep(250000);
    }
}

void colors_init() {
    start_color();
    init_pair(PLAYER, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(NORM, COLOR_WHITE, COLOR_BLACK);
    init_pair(BEAST, COLOR_ORANGE, COLOR_BLACK);
    init_pair(BUSH, COLOR_BRIGHT_GREEN, COLOR_BLACK);
    init_pair(COIN, COLOR_BLACK, COLOR_GOLDEN);
    init_pair(CAMP, COLOR_GOLDEN, COLOR_DARK_GREEN);
}