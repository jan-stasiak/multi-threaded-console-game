#ifndef MULTI_THREADED_CONSOLE_GAME_SERVER_FUNCTIONS_H
#define MULTI_THREADED_CONSOLE_GAME_SERVER_FUNCTIONS_H


bool check_pos(int y, int x, struct server_information_t *s);

void add_big_treasure(struct server_information_t *s);

void add_player(struct server_information_t *s, int i);

void add_treasure(struct server_information_t *s);

bool check_pos_beast(int x, int y, struct server_information_t *s);

void *player_handler(void *args);

void add_coin(struct server_information_t *s);

void add_beast(struct server_information_t *s);

void add_campsite(struct map_information_t *m);

void create_players(struct server_information_t *s_info);

void display_init(struct display_server_t *s_info);

void move_beast(struct server_information_t *s);

void kill_player(struct server_information_t *s, struct player_information_t *p);

void create_treasure(struct server_information_t *s, size_t how_much, int y, int x);

void print_legend(WINDOW *dis, struct server_information_t *s);

void colors_init();

void *print_map_thread(void *args);

void *keyboard(void *args);

void create_server_connection(struct server_information_t *server);

void prepare_map(struct server_information_t *server, struct map_information_t *player_map);

int find_player(struct server_information_t *s, int x, int y);

void delete_unused_shm(struct server_information_t *server);

bool check_player_pos(struct server_information_t *server, struct player_information_t *player, int x ,int y);

void move_player(struct server_information_t *server);

void *beast_handler(void *args);

#endif //MULTI_THREADED_CONSOLE_GAME_SERVER_FUNCTIONS_H
