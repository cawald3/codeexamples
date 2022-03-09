#include "map.h"
#include "globals.h"
#include "graphics.h"

/**
 * The Map structure. This holds a HashTable for all the MapItems, along with
 * values for the width and height of the Map.
 */
struct Map {
    HashTable* items;
    int w, h;
};

/**
 * Storage area for the maps.
 * This is a global variable, but can only be access from this file because it
 * is static.
 */
static Map map;         //0
static Map underworld;  //1
static int active_map;

int get_map_num(){
    return active_map;
}

/**
 * The first step in HashTable access for the map is turning the two-dimensional
 * key information (x, y) into a one-dimensional unsigned integer.
 * This function should uniquely map (x,y) onto the space of unsigned integers.
 */
static unsigned XY_KEY(int X, int Y) {
    return X*1000+Y;
}

/**
 * This is the hash function actually passed into createHashTable. It takes an
 * unsigned key (the output of XY_KEY) and turns it into a hash value (some
 * small non-negative integer).
 */
unsigned map_hash(unsigned key)
{
    return key%get_active_map()->w; //Maps the keys by x coord into the hashtable
}

void maps_init()
{
    // TODO: Implement!    
    // Initialize hash table
    // Set width & height
    map.w = 50;
    map.h = 50;
    map.items = createHashTable (map_hash, 50);
    underworld.w = 30;
    underworld.h = 30;
    underworld.items = createHashTable(map_hash, 30);
}

Map* get_active_map()
{
    // There's only one map
    if(active_map == 0) {
        return &map;
    } else {
        return &underworld;
        }
}

Map* set_active_map(int m)
{
    active_map = m;
    return get_active_map();
}

void print_map()
{
    // As you add more types, you'll need to add more items to this array.
    char lookup[] = {'W', 'P', 'T', 'M', 'N', 'S', 'C', 'X'};
    for(int y = 0; y < map_height(); y++)
    {
        for (int x = 0; x < map_width(); x++)
        {
            MapItem* item = get_here(x,y);
            if (item) pc.printf("%c", lookup[item->type]);
            else pc.printf(" ");
        }
        pc.printf("\r\n");
    }
}

int map_width()
{
    return get_active_map()->w;
}

int map_height()
{
    return get_active_map()->h;
}

int map_area()
{
    return map_height()*map_width();
}

MapItem* get_north(int x, int y)
{
    Map* mapT = get_active_map();
    int yIndex = y-1;
    return (MapItem*)(getItem(mapT->items, XY_KEY(x, yIndex)));
}

MapItem* get_south(int x, int y)
{
    int yIndex = y+1;
    Map* mapT = get_active_map();
    return (MapItem*)getItem(mapT->items, XY_KEY(x, yIndex));
}

MapItem* get_east(int x, int y)
{
    Map* mapT = get_active_map();
    int xIndex = x+1;
    return (MapItem*)getItem(mapT->items, XY_KEY(xIndex, y));
}

MapItem* get_west(int x, int y)
{
    Map* mapT = get_active_map();
    int xIndex = x-1;
    return (MapItem*)getItem(mapT->items, XY_KEY(xIndex, y));
}

MapItem* get_here(int x, int y)
{
    Map* mapT = get_active_map();
    return (MapItem*)getItem(mapT->items, XY_KEY(x, y));
}


void map_erase(int x, int y)
{
    Map* mapT = get_active_map();
    removeItem(mapT->items, XY_KEY(x,y));
}

void add_wall(int x, int y, int dir, int len)
{
    void* val;
    void* temp;
    for(int i = 0; i < len; i++)
    {
        MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));

        w1->type = WALL;
        w1->draw = draw_wall;
        w1->walkable = false;
        w1->data = NULL;

        unsigned key = (dir == HORIZONTAL) ? XY_KEY(x+i, y) : XY_KEY(x, y+i);



        HashTable* itemsHT = get_active_map()->items;
        //printf("1i\n\r");
        //printf("HT: %d, Key: %d, Object: %d\r\n", itemsHT, key, &w1);
        temp = malloc(1);
        //printf("Check for space: %d\n\r", temp);
        free(temp);
        val = insertItem(itemsHT, key, w1);
        //printf("1b\n\r");
        if (val) free(val); // If something is already there, free it
        //printf("1c\n\r");
        //printf("SIZE: %d\n\r", sizeof(MapItem));
    }
}

void add_plant(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = PLANT;
    w1->draw = draw_plant;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it
}

void add_thorn(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = THORN;
    w1->draw = draw_thorn;
    w1->walkable = true;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it
}
void add_mine(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = MINE;
    w1->draw = draw_mine;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it
}
 
void add_npc(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = NPC;
    w1->draw = draw_npc;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it  
}

void add_sonar(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = SONAR;
    w1->draw = draw_sonar;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it  
}

void add_chest(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = CHEST;
    w1->draw = draw_chest;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it  
}

void add_spider(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = SPIDER;
    w1->draw = draw_spider;
    w1->walkable = false;
    w1->data = NULL;
    printf("Pointer: %d\r\n ",w1);
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it  
}
void add_fchest(int x, int y)
{
    MapItem* w1 = (MapItem*) malloc(sizeof(MapItem));
    w1->type = FCHEST;
    w1->draw = draw_chest;
    w1->walkable = false;
    w1->data = NULL;
    void* val = insertItem(get_active_map()->items, XY_KEY(x, y), w1);
    if (val) free(val); // If something is already there, free it  
}