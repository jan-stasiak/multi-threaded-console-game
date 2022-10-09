#define MAP_PATH "./src/map.txt"

typedef enum {
    AIR = 0,
    WALL,
    BUSH,
    CAMP,
    BEAST,
    PLAYER,
    COIN,
    TREASURE,
    LARGE_TREASURE,
    DROP_TREASURE
} map_structure_t;

struct coordinate_t {
    unsigned int x;
    unsigned int y;
};

struct map_information_t {
    map_structure_t board[25][52];
};