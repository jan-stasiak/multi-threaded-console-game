#define MAP_PATH "map.txt"

#include <sys/types.h>
#include <ncurses.h>
#include <semaphore.h>

#define MAX_DROP_TREASURE 10

#define COLOR_DARK_GREEN 22
#define COLOR_BRIGHT_GREEN 82
#define COLOR_GOLDEN 226
#define COLOR_RED 196

typedef enum {
    LEFT = 0,
    RIGHT,
    UP,
    DOWN,
    CLOSE,
    NONE
} direction_t;

typedef enum {
    AIR = 0,
    WALL,
    BUSH,
    CAMP,
    BEAST,
    PLAYER,
    PLAYER1,
    PLAYER2,
    PLAYER3,
    PLAYER4,
    COIN,
    TREASURE,
    LARGE_TREASURE,
    DROP_TREASURE,
    NORM
} map_structure_t;

typedef enum {
    HUMAN = 0,
    CPU
} player_type_t;

struct coordinate_t {
    int x;
    int y;
};

struct display_server_t {
    WINDOW *game;
    WINDOW *stats;
};

struct player_information_t {
    bool connected;
    int pid;
    struct coordinate_t coordinate;
    struct coordinate_t spawn_pos;
    unsigned int deaths;
    unsigned int coins_carried;
    unsigned int coins_brought;
    direction_t direction;
    bool want_to_bush;
    bool is_in_bush;
};

struct map_information_t {
    map_structure_t board[25][52];
    struct coordinate_t campsite;
};


struct beast_t {
    bool exist;
    struct coordinate_t coordinate;
    direction_t direction;
};

struct player_status_t {
    bool free_place[4];
};

struct dropped_treasure_t {
    bool is_free;
    size_t coins;
    struct coordinate_t coordinate;
};

struct server_information_t {
    bool is_run;
    char server_pid[10];
    struct player_information_t players[4];
    struct beast_t beast[4];
    struct map_information_t map;
    size_t round;
    pthread_mutex_t mutex_key;
    struct display_server_t *display;
    struct player_status_t *playerStatus;
    sem_t *sem_connection;
    struct dropped_treasure_t treasure[MAX_DROP_TREASURE];
};

struct player_handler_t {
    struct server_information_t *server;
    struct player_information_t *player;
    size_t num;
};

struct shm_memory_t {
    direction_t direction;
    struct map_information_t map;
    int player_pid;
    int server_pid;
    struct coordinate_t campsite;
    struct coordinate_t player_coordinate;
    size_t round;
    size_t number;
    size_t deaths;
    size_t coin_found;
    size_t coin_brought;
    bool want_to_disconnect;
    bool is_server;
    sem_t sem_shm;
};

struct print_t {
    struct display_server_t *dis;
    struct map_information_t *map;
    struct shm_memory_t *shm;
    bool *status;
    sem_t *sem;
};

struct client_key_t {
    struct shm_memory_t *shm;
    bool *status;
    bool server_was_closed;
    sem_t *sem;
    char shm_name[20];
};

struct client_struct_t {
    struct client_key_t *c;
    struct print_t *t;
};


struct beast_struct_t {
    struct server_information_t *s;
    size_t num;
};

struct beast_container {
    struct server_information_t *s;
    struct beast_t *b;
};
