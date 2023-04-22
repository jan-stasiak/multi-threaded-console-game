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


#include "map.h"

#define SHM_CONNECTION "so2_connection_information"

#define SEMAPHORE_SERVER_NAME "so2_server"

#define PLAYER_VISION 3

bool check_pos(int y, int x, struct server_information_t *s) {
    if (s->map.board[y][x] != AIR) {
        return false;
    }

    if (s->map.campsite.x == x && s->map.campsite.y == y) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (s->players[i].connected) {
            if (s->players[i].coordinate.x == x && s->players[i].coordinate.y == y) {
                return false;
            }
        }
    }
    for (int i = 0; i < 4; ++i) {
        if (s->beast[i].exist == true) {
            if (s->beast[i].coordinate.x == x && s->beast[i].coordinate.y == y) {
                return false;
            }
        }
    }
    return true;
}

void prepare_map(struct server_information_t *server, struct map_information_t *player_map) {
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 51; j++) {
            if (server->map.board[i][j] == AIR) {
                player_map->board[i][j] = AIR;
            } else if (server->map.board[i][j] == WALL) {
                player_map->board[i][j] = WALL;
            } else if (server->map.board[i][j] == BUSH) {
                player_map->board[i][j] = BUSH;
            } else if (server->map.board[i][j] == COIN) {
                player_map->board[i][j] = COIN;
            } else if (server->map.board[i][j] == TREASURE) {
                player_map->board[i][j] = TREASURE;
            } else if (server->map.board[i][j] == LARGE_TREASURE) {
                player_map->board[i][j] = LARGE_TREASURE;
            }
        }
    }

    player_map->board[server->map.campsite.y][server->map.campsite.x] = CAMP;
    for (int i = 0; i < 4; ++i) {
        if (server->beast[i].exist == true) {
            player_map->board[server->beast[i].coordinate.x][server->beast[i].coordinate.y] = BEAST;
        }
    }

    for (int i = 0; i < 4; ++i) {
        if (server->players[i].connected == true) {
            switch (i) {
                case 0:
                    player_map->board[server->players[i].coordinate.y][server->players[i].coordinate.x] = PLAYER1;
                    break;
                case 1:
                    player_map->board[server->players[i].coordinate.y][server->players[i].coordinate.x] = PLAYER2;
                    break;
                case 2:
                    player_map->board[server->players[i].coordinate.y][server->players[i].coordinate.x] = PLAYER3;
                    break;
                case 3:
                    player_map->board[server->players[i].coordinate.y][server->players[i].coordinate.x] = PLAYER4;
                    break;
            }
        }
    }
}

void create_players(struct server_information_t *s_info) {
    s_info->is_run = true;
    int pid = getpid();
    sprintf(s_info->server_pid, "%d", pid);
    s_info->round = 0;


    for (int i = 0; i < 4; ++i) {
        s_info->players[i].connected = false;
        s_info->players[i].pid = 0;
        s_info->players[i].coordinate.x = 0;
        s_info->players[i].coordinate.y = 0;
        s_info->players[i].spawn_pos.x = 0;
        s_info->players[i].spawn_pos.y = 0;
        s_info->players[i].coins_brought = 0;
        s_info->players[i].coins_carried = 0;
        s_info->players[i].deaths = 0;
        s_info->beast[i].exist = false;
        s_info->players[i].direction = NONE;
        s_info->players[i].want_to_bush = false;
    }

    for (int i = 0; i < 10; ++i) {
        s_info->treasure->is_free = true;
        s_info->treasure->coordinate.x = 0;
        s_info->treasure->coordinate.y = 0;
        s_info->treasure->coins = 0;
    }
}

bool check_pos_beast(int x, int y, struct server_information_t *s) {
    if (s->map.board[y][x] != AIR && s->map.board[y][x] != COIN && s->map.board[y][x] != TREASURE &&
        s->map.board[y][x] != LARGE_TREASURE) {
        return false;
    }

    return true;
}


void add_big_treasure(struct server_information_t *s) {
    int x = rand() % 22 + 1;
    int y = rand() % 49 + 1;
    while (1) {
        if (s->map.board[x][y] == AIR) {
            break;
        }
        x = rand() % 22 + 1;
        y = rand() % 49 + 1;
    }
    s->map.board[x][y] = LARGE_TREASURE;
}

void add_treasure(struct server_information_t *s) {
    int x = rand() % 22 + 1;
    int y = rand() % 49 + 1;
    while (1) {
        if (s->map.board[x][y] == AIR) {
            break;
        }
        x = rand() % 22 + 1;
        y = rand() % 49 + 1;
    }
    s->map.board[x][y] = TREASURE;
}

void add_coin(struct server_information_t *s) {
    int x = rand() % 49 + 1;
    int y = rand() % 22 + 1;
    while (1) {
        if (s->map.board[y][x] == AIR) {
            break;
        }
        x = rand() % 49 + 1;
        y = rand() % 22 + 1;
    }
    s->map.board[y][x] = COIN;
}


void add_beast(struct server_information_t *s) {
    for (int i = 0; i < 4; ++i) {
        if (!s->beast[i].exist) {
            int x = rand() % 49 + 1;
            int y = rand() % 22 + 1;
            while (1) {
                if (check_pos(y, x, s)) {
                    break;
                }
                x = rand() % 49 + 1;
                y = rand() % 22 + 1;
            }
            s->beast[i].coordinate.x = x;
            s->beast[i].coordinate.y = y;
            s->beast[i].exist = true;
            break;
        }
    }
}

int find_player(struct server_information_t *s, int x, int y) {
    int how_deep = 0, beast_x = x, beast_y = y;
    bool find_player = false;
    while (1) {
        for (int i = 0; i < 4; ++i) {
            if (s->players[i].coordinate.x == x && s->players[i].coordinate.y == (y + how_deep)) {
                return 3;
            }
        }
        if (s->map.board[beast_y + how_deep][beast_x] == WALL) {
            break;
        } else if (how_deep == PLAYER_VISION) {
            break;
        }
        how_deep++;
    }

    how_deep = 0;
    while (1) {
        for (int i = 0; i < 4; ++i) {
            if (s->players[i].coordinate.x == x && s->players[i].coordinate.y == (y - how_deep)) {
                return 2;
            }
        }
        if (s->map.board[beast_y - how_deep][beast_x] == WALL) {
            break;
        } else if (how_deep == PLAYER_VISION) {
            break;
        }
        how_deep++;
    }

    how_deep = 0;
    while (1) {
        for (int i = 0; i < 4; ++i) {
            if (s->players[i].coordinate.x == (x + how_deep) && s->players[i].coordinate.y == y) {
                return 1;
            }
        }

        if (s->map.board[beast_y][beast_x + how_deep] == WALL) {
            break;
        } else if (how_deep == PLAYER_VISION) {
            break;
        }
        how_deep++;
    }

    how_deep = 0;
    while (1) {
        for (int i = 0; i < 4; ++i) {
            if (s->players[i].coordinate.x == (x - how_deep) && s->players[i].coordinate.y == y) {
                return 0;
            }
        }

        if (s->map.board[beast_y][beast_x - how_deep] == WALL) {
            break;
        } else if (how_deep == PLAYER_VISION) {
            break;
        }
        how_deep++;
    }

    return 5;
}


void *beast_handler(void *args) {
    struct beast_container *c = ((struct beast_container *) args);
    struct map_information_t map;


    while (1) {
        if (c->s->is_run == false) {
            return NULL;
        }
        if (c->b->exist) {
            int beast_x = c->b->coordinate.x, beast_y = c->b->coordinate.y;

            sem_wait(c->s->sem_connection);

            direction_t direction1 = find_player(c->s, beast_x, beast_y);
            if (direction1 != NONE) {
                c->b->direction = direction1;
            } else {
                direction_t direction = NONE;
                int new_x = c->b->coordinate.x, new_y = c->b->coordinate.y;
                do {
                    new_x = c->b->coordinate.x, new_y = c->b->coordinate.y;
                    direction = rand() % 4;
                    if (direction == LEFT) {
                        new_x--;
                    } else if (direction == RIGHT) {
                        new_x++;
                    } else if (direction == UP) {
                        new_y--;
                    } else if (direction == DOWN) {
                        new_y++;
                    }
                } while (!check_pos_beast(new_x, new_y, c->s));
                c->b->direction = direction;
            }


            sem_post(c->s->sem_connection);
            usleep(100);
        }
    }
}

void kill_player(struct server_information_t *s, struct player_information_t *p) {
    p->deaths++;
    p->coins_carried = 0;
    p->coordinate.x = p->spawn_pos.x;
    p->coordinate.y = p->spawn_pos.y;
}

void create_treasure(struct server_information_t *s, size_t how_much, int y, int x) {
    for (int i = 0; i < MAX_DROP_TREASURE; ++i) {
        if (s->treasure[i].is_free == true) {
            s->treasure[i].is_free = false;
            s->treasure[i].coordinate.y = y;
            s->treasure[i].coordinate.x = x;
            s->treasure[i].coins = how_much;
            break;
        }
    }
}


void move_beast(struct server_information_t *s) {
    for (int i = 0; i < 4; ++i) {
        if (s->beast[i].exist) {
            int x = s->beast[i].coordinate.x, y = s->beast[i].coordinate.y;
            if (s->beast[i].direction == UP) {
                if (s->map.board[y - 1][x] != WALL) {
                    if (s->map.board[y - 1][x] == AIR) {
                        s->beast[i].coordinate.x = x;
                        s->beast[i].coordinate.y = y - 1;
                    }
                    for (int j = 0; j < 4; ++j) {
                        if (s->players[j].connected == true) {
                            int player_x = s->players[i].coordinate.x, player_y = s->players[i].coordinate.y;
                            if (s->beast[i].coordinate.x == x && s->beast[i].coordinate.y == y) {
                                create_treasure(s, s->players[j].coins_carried, y, x);
                                kill_player(s, &s->players[j]);
                            }
                        }
                    }
                }
            } else if (s->beast[i].direction == DOWN) {
                if (s->map.board[y + 1][x] != WALL) {
                    if (s->map.board[y + 1][x] == AIR) {
                        s->beast[i].coordinate.x = x;
                        s->beast[i].coordinate.y = y + 1;
                    }
                    for (int j = 0; j < 4; ++j) {
                        if (s->players[j].connected == true) {
                            int player_x = s->players[i].coordinate.x, player_y = s->players[i].coordinate.y;
                            if (s->beast[i].coordinate.x == x && s->beast[i].coordinate.y == y) {
                                create_treasure(s, s->players[j].coins_carried, y, x);
                                kill_player(s, &s->players[j]);
                            }
                        }
                    }
                }
            } else if (s->beast[i].direction == LEFT) {
                if (s->map.board[y][x - 1] != WALL) {
                    if (s->map.board[y][x - 1] == AIR) {
                        s->beast[i].coordinate.x = x - 1;
                        s->beast[i].coordinate.y = y;
                    }
                    for (int j = 0; j < 4; ++j) {
                        if (s->players[j].connected == true) {
                            int player_x = s->players[i].coordinate.x, player_y = s->players[i].coordinate.y;
                            if (s->beast[i].coordinate.x == x && s->beast[i].coordinate.y == y) {
                                create_treasure(s, s->players[j].coins_carried, y, x);
                                kill_player(s, &s->players[j]);
                            }
                        }
                    }
                }
            } else if (s->beast[i].direction == RIGHT) {
                if (s->map.board[y][x + 1] != WALL) {
                    if (s->map.board[y][x + 1] == AIR) {
                        s->beast[i].coordinate.x = x + 1;
                        s->beast[i].coordinate.y = y;
                    }
                    for (int j = 0; j < 4; ++j) {
                        if (s->players[j].connected == true) {
                            int player_x = s->players[i].coordinate.x, player_y = s->players[i].coordinate.y;
                            if (s->beast[i].coordinate.x == x && s->beast[i].coordinate.y == y) {
                                create_treasure(s, s->players[j].coins_carried, y, x);
                                kill_player(s, &s->players[j]);
                            }
                        }
                    }
                }
            }
        }
        s->beast[i].direction = NONE;
    }
}

void add_campsite(struct map_information_t *m) {
    int x = rand() % 49 + 1;
    int y = rand() % 22 + 1;
    while (1) {
        if (m->board[x][y] == AIR) {
            break;
        }
        x = rand() % 49 + 1;
        y = rand() % 22 + 1;
    }
    m->campsite.x = x;
    m->campsite.y = y;
}

void display_init(struct display_server_t *dis) {
    WINDOW *g = newwin(27, 53, 0, 0);
    WINDOW *s = newwin(50, 50, 0, 53 + 2);
    dis->game = g;
    dis->stats = s;
    box(dis->game, 0, 0);
}


void print_legend(WINDOW *dis, struct server_information_t *s) {
    int y = 1, x = 0;
    char string[50] = {'\0'};
    refresh();
    wclear(dis);
    refresh();

    sprintf(string, "Server's PID: %s", s->server_pid);
    mvwprintw(dis, y, x, string);
    y = 2, x = 0;
    sprintf(string, " Campsite X/Y: %d/%d", s->map.campsite.y, s->map.campsite.x);
    mvwprintw(dis, y++, x, string);
    sprintf(string, "Round: %d", (int) s->round);
    x = 0;
    mvwprintw(dis, y++, x, string);
    y = 4, x = 0;
    sprintf(string, "Parameter:   Player1  Player2  Player3  Player4");
    mvwprintw(dis, y++, x, string);
    mvwprintw(dis, y++, x, " PID");
    mvwprintw(dis, y++, x, " Type");
    mvwprintw(dis, y++, x, " Curr x/y");
    mvwprintw(dis, y++, x, " Deaths");
    y += 3;
    mvwprintw(dis, y++, x, "Coins");
    mvwprintw(dis, y++, x, "  carried");
    mvwprintw(dis, y++, x, "  brought");
    y += 3;
    mvwprintw(dis, y++, x, "Legend: ");
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(PLAYER));
    mvwprintw(dis, y, x, "1234");
    x += 4;
    wattron(dis, COLOR_PAIR(NORM));

    mvwprintw(dis, y++, x, " - players");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(WALL));
    mvwprintw(dis, y, x++, " ");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, "    - wall");
    x = 0;
    mvwprintw(dis, y++, x, "  #    - bushes (slow down)");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(BEAST));
    mvwprintw(dis, y, x++, "*");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, "    - wild beast");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(COIN));
    mvwprintw(dis, y, x++, "C");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y, x, "    - one coin  ");
    x += 23;
    wattron(dis, COLOR_PAIR(COIN));
    mvwprintw(dis, y, x++, "D");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, " - dropped treasure ");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(COIN));
    mvwprintw(dis, y, x++, "t");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, " - treasure (10 coins)");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(COIN));
    mvwprintw(dis, y, x++, "T");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, " - large treasure (50 coins)");
    x = 0;
    mvwprintw(dis, y, x, "  ");
    x += 2;
    wattron(dis, COLOR_PAIR(CAMP));
    mvwprintw(dis, y, x++, "A");
    wattron(dis, COLOR_PAIR(NORM));
    mvwprintw(dis, y++, x, " - campsite");

    int y_player = 5, x_player = 13;
    for (int i = 0; i < 4; ++i) {
        y_player = 5;
        if (s->players[i].connected == true) {
            sprintf(string, "%d", s->players[i].pid);
            mvwprintw(dis, y_player++, x_player, "%s", string);
            mvwprintw(dis, y_player++, x_player, "HUMAN");
            sprintf(string, "%d/%d", s->players[i].coordinate.x, s->players[i].coordinate.y);
            mvwprintw(dis, y_player++, x_player, "%s", string);
            sprintf(string, "%d", s->players[i].deaths);
            mvwprintw(dis, y_player++, x_player, "%s", string);
            y_player += 4;
            sprintf(string, "%d", s->players[i].coins_carried);
            mvwprintw(dis, y_player++, x_player, "%s", string);
            sprintf(string, "%d", s->players[i].coins_brought);
            mvwprintw(dis, y_player++, x_player, "%s", string);
        } else {
            mvwprintw(dis, y_player++, x_player, "-");
            mvwprintw(dis, y_player++, x_player, "-");
            mvwprintw(dis, y_player++, x_player, "--/--");
            mvwprintw(dis, y_player++, x_player, "-");
            y_player += 4;
            mvwprintw(dis, y_player++, x_player, "NaN");
            mvwprintw(dis, y_player++, x_player, "NaN");
        }
        x_player += 9;
    }
    char aaa[10];
    int how_many = 0;
    for (int i = 0; i < 4; ++i) {
        if (s->beast[i].exist == true) {
            how_many++;
        }
    }

    wrefresh(dis);
    refresh();
}


void colors_init() {
    start_color();
    init_pair(PLAYER, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(NORM, COLOR_WHITE, COLOR_BLACK);
    init_pair(BEAST, COLOR_RED, COLOR_BLACK);
    init_pair(BUSH, COLOR_BRIGHT_GREEN, COLOR_BLACK);
    init_pair(COIN, COLOR_BLACK, COLOR_GOLDEN);
    init_pair(CAMP, COLOR_GOLDEN, COLOR_DARK_GREEN);
}

bool check_player_pos(struct server_information_t *server, struct player_information_t *player, int x, int y) {
    if (server->map.board[y][x] != AIR) {
        return false;
    }

    if (server->map.campsite.x == x && server->map.campsite.y == y) {
        return false;
    }


    return true;
}

void move_player(struct server_information_t *server) {
    for (int i = 0; i < 4; ++i) {
        if (server->players[i].connected) {
            int x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
            if (server->players[i].direction == UP) {
                x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
                y--;
                if (server->map.board[y][x] == AIR) {
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == COIN) {
                    server->players[i].coins_carried += 1;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == TREASURE) {
                    server->players[i].coins_carried += 10;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == LARGE_TREASURE) {
                    server->players[i].coins_carried += 50;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == BUSH) {
                    if (server->players[i].is_in_bush == true || server->players[i].want_to_bush == true) {
                        server->players[i].coordinate.y = y;
                        server->players[i].coordinate.x = x;
                    } else if (server->players[i].is_in_bush == false && server->players[i].want_to_bush == false) {
                        server->players[i].want_to_bush = true;
                    }
                }
            } else if (server->players[i].direction == LEFT) {
                x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
                x--;
                if (server->map.board[y][x] == AIR) {
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == COIN) {
                    server->players[i].coins_carried += 1;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == TREASURE) {
                    server->players[i].coins_carried += 10;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == LARGE_TREASURE) {
                    server->players[i].coins_carried += 50;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == BUSH) {
                    if (server->players[i].is_in_bush == true || server->players[i].want_to_bush == true) {
                        server->players[i].coordinate.y = y;
                        server->players[i].coordinate.x = x;
                    } else if (server->players[i].is_in_bush == false && server->players[i].want_to_bush == false) {
                        server->players[i].want_to_bush = true;
                    }
                }

            } else if (server->players[i].direction == DOWN) {
                x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
                y++;
                if (server->map.board[y][x] == AIR) {
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == COIN) {
                    server->players[i].coins_carried += 1;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == TREASURE) {
                    server->players[i].coins_carried += 10;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == LARGE_TREASURE) {
                    server->players[i].coins_carried += 50;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == BUSH) {
                    if (server->players[i].is_in_bush == true || server->players[i].want_to_bush == true) {
                        server->players[i].coordinate.y = y;
                        server->players[i].coordinate.x = x;
                    } else if (server->players[i].is_in_bush == false && server->players[i].want_to_bush == false) {
                        server->players[i].want_to_bush = true;
                    }
                }
            } else if (server->players[i].direction == RIGHT) {
                x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
                x++;
                if (server->map.board[y][x] == AIR) {
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == COIN) {
                    server->players[i].coins_carried += 1;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == TREASURE) {
                    server->players[i].coins_carried += 10;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == LARGE_TREASURE) {
                    server->players[i].coins_carried += 50;
                    server->players[i].coordinate.y = y;
                    server->players[i].coordinate.x = x;
                    server->map.board[y][x] = AIR;
                    server->players[i].is_in_bush = false;
                    server->players[i].want_to_bush = false;
                } else if (server->map.board[y][x] == BUSH) {
                    if (server->players[i].is_in_bush == true || server->players[i].want_to_bush == true) {
                        server->players[i].coordinate.y = y;
                        server->players[i].coordinate.x = x;
                    } else if (server->players[i].is_in_bush == false && server->players[i].want_to_bush == false) {
                        server->players[i].want_to_bush = true;
                    }
                }
            } else if (server->players[i].direction == CLOSE) {

            }
            for (int j = 0; j < MAX_DROP_TREASURE; ++j) {
                if (server->treasure[j].is_free == false) {
                    x = server->players[i].coordinate.x, y = server->players[i].coordinate.y;
                    if (server->treasure[i].coordinate.x == x && server->treasure[i].coordinate.y == y) {
                        server->players[i].coins_carried += server->treasure[j].coins;
                        server->treasure[j].coins = 0;
                        server->treasure[j].is_free = true;
                    }
                }
            }
            for (int j = 0; j < 4; ++j) {
                if (server->beast[j].exist == true) {
                    int beast_x = server->beast[j].coordinate.x, beast_y = server->beast[j].coordinate.y;
                    if (server->players[i].coordinate.x == server->beast[j].coordinate.x &&
                        server->players[i].coordinate.y == server->beast[j].coordinate.y) {
                        create_treasure(server, server->players[i].coins_carried, y, x);
                        kill_player(server, &server->players[i]);
                    }
                }
            }
            if (server->map.campsite.x == server->players[i].coordinate.x &&
                server->map.campsite.y == server->players[i].coordinate.y) {
                server->players[i].coins_brought += server->players[i].coins_carried;
                server->players[i].coins_carried = 0;
            }
            server->players[i].direction = NONE;
            for (int j = 0; j < 4; ++j) {
                if (i != j) {
                    if (server->players[i].coordinate.x == server->players[j].coordinate.x &&
                        server->players[i].coordinate.y == server->players[j].coordinate.y) {
                        create_treasure(server, server->players[i].coins_carried + server->players[j].coins_carried,
                                        server->players[i].coordinate.y, server->players[i].coordinate.x);
                        kill_player(server, &server->players[i]);
                        kill_player(server, &server->players[j]);
                    }
                }
            }
        }
    }
}

void *print_map_thread(void *args) {
    struct server_information_t *server = ((struct server_information_t *) args);
    while (1) {
        sem_wait(server->sem_connection);
        if (!server->is_run) {
            sem_post(server->sem_connection);
            return NULL;
        }
        move_player(server);
        move_beast(server);
        server->round++;
        print_legend(server->display->stats, server);
        print_map(server, &server->map, server->display->game);
        refresh();
        sem_post(server->sem_connection);
        usleep(500000);
    }
}

void *keyboard(void *args) {
    struct server_information_t *server = ((struct server_information_t *) args);
    while (1) {
        if (server->is_run == false) {
            return NULL;
        }
        char key = getchar();
        sem_wait(server->sem_connection);
        switch (key) {
            case 'b':
            case 'B':
                add_beast(server);

                break;
            case 'q':
            case 'Q':
                server->is_run = false;
                sem_post(server->sem_connection);
                usleep(1000);
                return NULL;
            case 'c':
                add_coin(server);
                break;
            case 't':
                add_treasure(server);
                break;
            case 'T':
                add_big_treasure(server);
                break;
        }
        sem_post(server->sem_connection);
        usleep(1000);
    }
}


void add_player(struct server_information_t *s, int i) {
    int y = rand() % 22 + 1;
    int x = rand() % 49 + 1;

    while (1) {
        if (check_pos(y, x, s)) {
            break;
        }
        y = rand() % 22 + 1;
        x = rand() % 49 + 1;
    }
    s->players[i].coordinate.x = x;
    s->players[i].coordinate.y = y;
    s->players[i].spawn_pos.x = x;
    s->players[i].spawn_pos.y = y;
}

void *player_handler(void *args) {
    struct player_handler_t *h = ((struct player_handler_t *) args);

    char shm_name[20] = {'\0'};

    sprintf(shm_name, "so2_player_shm_%zu", h->num);


    int fd = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (fd == -1) {
        h->server->is_run = false;
        return NULL;
    }


    ftruncate(fd, sizeof(struct shm_memory_t));
    struct shm_memory_t *shm = (struct shm_memory_t *) mmap(0, sizeof(struct shm_memory_t), PROT_READ | PROT_WRITE,
                                                            MAP_SHARED, fd, 0);

    int test = 0;
    struct map_information_t p_map;

    shm->player_pid = 0;
    shm->want_to_disconnect = false;
    shm->is_server = true;
    shm->direction = NONE;
    shm->server_pid = getpid();
    shm->want_to_disconnect = false;
    sem_init(&shm->sem_shm, 1, 1);


    while (1) {
        sem_wait(h->server->sem_connection);
        if (h->server->is_run == false) {
            shm->is_server = false;
            sem_destroy(&shm->sem_shm);
            shm_unlink(shm_name);
            munmap((void *) shm, sizeof(struct shm_memory_t));
            sem_post(h->server->sem_connection);
            return NULL;
        }
        if (h->player->connected == true) {
            if (shm->want_to_disconnect == true) {
                h->player->connected = false;
                h->server->playerStatus->free_place[h->num];
            } else {
                h->player->pid = shm->player_pid;
                shm->round = h->server->round;
                shm->number = h->num;
                shm->deaths = h->player->deaths;
                shm->coin_found = h->player->coins_carried;
                shm->coin_brought = h->player->coins_brought;
                shm->player_coordinate.x = h->player->coordinate.x;
                shm->player_coordinate.y = h->player->coordinate.y;
                prepare_map(h->server, &p_map);

                for (int i = 0; i < 25; i++) {
                    for (int j = 0; j < 51; j++) {
                        shm->map.board[i][j] = AIR;
                        shm->campsite.x = 0;
                        shm->campsite.y = 0;
                    }
                }

                for (int i = 0; i < 25; i++) {
                    for (int j = 0; j < 51; j++) {
                        if ((i > h->player->coordinate.y - PLAYER_VISION &&
                             i < h->player->coordinate.y + PLAYER_VISION) &&
                            (j > h->player->coordinate.x - PLAYER_VISION &&
                             j < h->player->coordinate.x + PLAYER_VISION)) {
                            shm->map.board[i][j] = p_map.board[i][j];
                        }
                    }
                }

                for (int i = 0; i < 4; ++i) {
                    if (h->server->beast[i].exist == true) {
                        if ((h->server->beast[i].coordinate.y > h->player->coordinate.y - PLAYER_VISION &&
                             h->server->beast[i].coordinate.y < h->player->coordinate.y + PLAYER_VISION) &&
                            (h->server->beast[i].coordinate.x > h->player->coordinate.x - PLAYER_VISION &&
                             h->server->beast[i].coordinate.x < h->player->coordinate.x + PLAYER_VISION)) {
                            shm->map.board[h->server->beast[i].coordinate.y][h->server->beast[i].coordinate.x] = BEAST;
                        }

                    }
                }

                int x_camp = h->server->map.campsite.x, y_camp = h->server->map.campsite.y, x_player = h->player->coordinate.x, y_player = h->player->coordinate.y;
                if (x_camp > x_player - PLAYER_VISION && x_camp < x_player + PLAYER_VISION &&
                    y_camp > y_player - PLAYER_VISION && y_camp < y_player + PLAYER_VISION) {
                    shm->campsite.x = x_camp;
                    shm->campsite.y = y_camp;
                } else {
                    shm->campsite.x = 0;
                    shm->campsite.y = 0;
                }


                if (shm->direction == LEFT) {
                    h->player->direction = LEFT;

                } else if (shm->direction == RIGHT) {
                    h->player->direction = RIGHT;

                } else if (shm->direction == UP) {
                    h->player->direction = UP;

                } else if (shm->direction == DOWN) {
                    h->player->direction = DOWN;

                } else if (shm->direction == CLOSE) {
                    h->player->coordinate.x = 0;
                    h->player->coordinate.y = 0;
                    h->player->coins_carried = 0;
                    h->player->coins_brought = 0;
                    h->player->deaths = 0;
                    h->player->pid = 0;
                    h->player->connected = false;
                    h->server->playerStatus->free_place[h->num] = true;
                }
                shm->direction = NONE;
            }

        } else if (h->server->playerStatus->free_place[h->num] == false) {
            h->player->connected = true;
            h->player->pid = shm->player_pid;
            add_player(h->server, h->num);
        }

        sem_post(h->server->sem_connection);
        usleep(100);
    }
}

void create_server_connection(struct server_information_t *server) {
    server->sem_connection = sem_open(SEMAPHORE_SERVER_NAME, O_CREAT, 0666, 1);
    sem_wait(server->sem_connection);
    int fd = shm_open(SHM_CONNECTION, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (fd == -1) {
        if (errno == EEXIST) {
            printf("Memory object already exists!\n");
            shm_unlink("server");
        } else {
            printf("Error occured. Error message: %s\n", strerror(errno));
        }
        server->is_run = false;
        return NULL;
    }

    ftruncate(fd, sizeof(struct player_status_t));
    server->playerStatus = (struct player_status_t *) mmap(0, sizeof(struct player_status_t), PROT_READ | PROT_WRITE,
                                                           MAP_SHARED, fd, 0);
    for (int i = 0; i < 4; ++i) {
        server->playerStatus->free_place[i] = true;
    }
    sem_post(server->sem_connection);
    usleep(1000);
}

void delete_unused_shm(struct server_information_t *server) {
    char shm_name[20] = {'\0'};
    for (int i = 0; i < 4; ++i) {
        if (server->players[i].connected == false) {
            sprintf(shm_name, "so2_player_shm_%d", i);
            unlink(shm_name);
        }
    }
}

