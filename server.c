#include <pthread.h>
#include <ncurses.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/mman.h>
#include <string.h>


#include "server_functions.h"
#include "map.h"

#define SEMAPHORE_SERVER_NAME "so2_server"
#define SHM_CONNECTION "so2_connection_information"

struct server_information_t server;
struct display_server_t displayServer;


int main() {
    srand(time(NULL));
    initscr();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    curs_set(0);

    create_players(&server);
    server.display = &displayServer;

    colors_init();
    display_init(&displayServer);
    load_map(MAP_PATH, &server.map);
    add_campsite(&server.map);
    pthread_mutex_init(&server.mutex_key, NULL);
    create_server_connection(&server);


    pthread_t players_communication[4], connections_to_server;


    pthread_t print, key, player1, player2, player3, player4, beast1, beast2, beast3, beast4;
    struct player_handler_t p1, p2, p3, p4;
    p1.num = 0;
    p1.server = &server;
    p1.player = &server.players[0];
    p2.num = 1;
    p2.server = &server;
    p2.player = &server.players[1];
    p3.num = 2;
    p3.server = &server;
    p3.player = &server.players[2];
    p4.num = 3;
    p4.server = &server;
    p4.player = &server.players[3];


    struct beast_container b1, b2, b3, b4;
    b1.s = &server;
    b2.s = &server;
    b3.s = &server;
    b4.s = &server;
    b1.b = &server.beast[0];
    b2.b = &server.beast[1];
    b3.b = &server.beast[2];
    b4.b = &server.beast[3];


    pthread_create(&print, NULL, print_map_thread, &server);
    pthread_create(&key, NULL, keyboard, &server);
    pthread_create(&player1, NULL, player_handler, &p1);
    pthread_create(&player2, NULL, player_handler, &p2);
    pthread_create(&player3, NULL, player_handler, &p3);
    pthread_create(&player4, NULL, player_handler, &p4);

    pthread_create(&beast1, NULL, beast_handler, &b1);
    pthread_create(&beast2, NULL, beast_handler, &b2);
    pthread_create(&beast3, NULL, beast_handler, &b3);
    pthread_create(&beast4, NULL, beast_handler, &b4);


    pthread_join(key, NULL);
    pthread_join(print, NULL);
    pthread_join(player1, NULL);
    pthread_join(player2, NULL);
    pthread_join(player3, NULL);
    pthread_join(player4, NULL);

    pthread_join(beast1, NULL);
    pthread_join(beast2, NULL);
    pthread_join(beast3, NULL);
    pthread_join(beast4, NULL);
    sem_close(server.sem_connection);
    sem_unlink(SEMAPHORE_SERVER_NAME);

    munmap((void *) server.playerStatus, sizeof(struct player_status_t));
    shm_unlink(SHM_CONNECTION);

    delete_unused_shm(&server);

    wdelch(displayServer.game);
    wdelch(displayServer.stats);

    endwin();
    return 0;
}