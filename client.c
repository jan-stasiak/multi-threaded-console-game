#include <stdio.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "const.h"
#include "client_functions.h"

#define SEMAPHORE_SERVER_NAME "so2_server"
#define SHM_CONNECTION "so2_connection_information"

struct display_server_t display;
bool server_status;

int main() {
    initscr();
    noecho();
    raw();
    keypad(stdscr, TRUE);
    curs_set(0);
    display_init_client(&display);
    colors_init();

    sem_t *sem_server = sem_open(SEMAPHORE_SERVER_NAME, 0);
    if (sem_server == NULL) {
        refresh();
        mvwprintw(display.game, 0, 0, "Server is closed! Press any button to close.");
        wrefresh(display.game);
        refresh();
        getchar();
        endwin();
        return 1;
    } else {
        printf("OK");
    }

    int fd_server = shm_open(SHM_CONNECTION, O_RDWR, 0666);

    if (fd_server == -1) {
        printf("Error occured. Error message: %s\n", strerror(errno));
        endwin();
        return 1;
    }

    ftruncate(fd_server, sizeof(struct player_status_t));
    struct player_status_t *data = (struct player_status_t *) mmap(0, sizeof(struct player_status_t),
                                                                   PROT_READ | PROT_WRITE, MAP_SHARED, fd_server, 0);

    sem_wait(sem_server);
    bool was_place = false;
    int i = 0;
    for (i = 0; i < 4; ++i) {
        if (data->free_place[i] == true) {
            data->free_place[i] = false;
            was_place = true;
            break;
        }
    }
    sem_post(sem_server);
    if (was_place == false) {
        munmap(data, sizeof(struct player_status_t));
        printf("Server is full\n");
        endwin();
        return EXIT_SUCCESS;
    }

    char sem_name[20] = {'\0'};
    char shm_name[20] = {'\0'};

    create_names(sem_name, shm_name, i);


    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd == -1) {
        printf("Error occured. Error message: %s\n", strerror(errno));
        munmap(data, sizeof(struct player_status_t));
        endwin();
        return 1;
    }

    ftruncate(fd, sizeof(struct shm_memory_t));
    struct shm_memory_t *shmMemory = (struct shm_memory_t *) mmap(0, sizeof(struct shm_memory_t),
                                                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    sem_wait(sem_server);
    shmMemory->player_pid = getpid();
    server_status = true;
    sem_post(sem_server);

    struct client_key_t key;
    key.sem = sem_server;
    key.shm = shmMemory;
    key.status = &server_status;
    strcpy(key.shm_name, shm_name);
    key.server_was_closed = false;

    struct print_t print_struct;
    print_struct.map = &shmMemory->map;
    print_struct.status = &server_status;
    print_struct.dis = &display;
    print_struct.sem = sem_server;
    print_struct.shm = shmMemory;

    struct client_struct_t clientStruct;
    clientStruct.c = &key;
    clientStruct.t = &print_struct;


    pthread_t display_thread, key_handler;
    pthread_create(&display_thread, NULL, print_game, &clientStruct);
    pthread_create(&key_handler, NULL, key_handler_client, &clientStruct);
    pthread_join(key_handler, NULL);
    pthread_join(display_thread, NULL);


    munmap(data, sizeof(struct player_status_t));
    munmap(shmMemory, sizeof(struct shm_memory_t));

    endwin();
    return 0;
}
