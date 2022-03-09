// Project includes
#include "globals.h"
#include "hardware.h"
#include "map.h"
#include "graphics.h"
#include "speech.h"
#include <time.h>
#include <stdio.h> 
#include <stdlib.h>

// Functions in this file
int get_action (GameInputs inputs);
int update_game (int action);
void draw_game (int init);
void init_main_map ();
int main ();



/**
 * The main game state. Must include Player locations and previous locations for
 * drawing to work properly. Other items can be added as needed.
 */
struct {
    int x,y;    // Current locations
    int px, py; // Previous locations
    int tx, ty;
    int has_sonar;
    int moved;
    int movedAct;
    int health;
    int maxHealth;
    int found;
    int omni;
    int difficulty;
    int world;
    int stuck;
    int timer;
    int won;
    int sx, sy; //for spider movement
    // You can add other properties for the player here
} Player;

typedef struct Enemy {
    int x, y;
    int px, py;
    int type;
}Enemy;
int num;
void updateEnemy(Enemy* enemy);
/**
 * Given the game inputs, determine what kind of update needs to happen.
 * Possbile return values are defined below.
 */
#define NO_ACTION 0
#define ACTION_BUTTON 1
#define MENU_BUTTON 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_UP 5
#define GO_DOWN 6
#define RUN_LEFT 7
#define RUN_RIGHT 8
#define RUN_UP 9
#define RUN_DOWN 10
#define OMNI     11
int get_action(GameInputs inputs)
{
    if(inputs.ax > .5 && inputs.b1 == 0)    return GO_RIGHT;
    if(inputs.ax < -.5 && inputs.b1 == 0)   return GO_LEFT;
    if(inputs.ay > .4 && inputs.b1 == 0)    return GO_UP;
    if(inputs.ay  < -.5 && inputs.b1 == 0)   return GO_DOWN;
     if(inputs.ax > .5 && inputs.b1 == 1)   return RUN_RIGHT;
    if(inputs.ax < -.5 && inputs.b1 == 1)   return RUN_LEFT;
    if(inputs.ay > .4 && inputs.b1 == 1)    return RUN_UP;
    if(inputs.ay  < -.5 && inputs.b1 == 1)  return RUN_DOWN;
    if(inputs.b2 == 0)                      return ACTION_BUTTON;
    if(inputs.b3 == 0)                      return OMNI;
    return NO_ACTION;
}

/**
 * Update the game state based on the user action. For example, if the user
 * requests GO_UP, then this function should determine if that is possible by
 * consulting the map, and update the Player position accordingly.
 * 
 * Return values are defined below. FULL_DRAW indicates that for this frame,
 * draw_game should not optimize drawing and should draw every tile, even if
 * the player has not moved.
 */
#define NO_RESULT 0
#define GAME_OVER 1
#define FULL_DRAW 2
int update_game(int action)
{
    // Save player previous location before updating
    Player.px = Player.x;
    Player.py = Player.y;
    MapItem *item = get_north(0,0);

    
    // Do different things based on the each action.
    // You can define functions like "go_up()" that get called for each case.
    switch(action)
    {
        case GO_UP: 
            if(get_north(Player.x, Player.y)->walkable && Player.moved == 0 || Player.omni) {
            Player.y -= 1;
            Player.moved = 1;
            Player.movedAct = abs(Player.movedAct - 1);
            }
            else {
                Player.moved = 0;    
            }   
             break;
        case GO_LEFT:  
            if(get_west(Player.x, Player.y)->walkable && Player.moved == 0 || Player.omni){
            Player.x -= 1;
            Player.moved = 1;
            Player.movedAct = abs(Player.movedAct - 1);
            }
            else {
                Player.moved = 0;    
            }   
             break;          
        case GO_DOWN:  
            if(get_south(Player.x, Player.y)->walkable && Player.moved == 0 || Player.omni) {
            Player.y += 1;
            Player.moved = 1;
            Player.movedAct = abs(Player.movedAct - 1);
            }
            else {
                Player.moved = 0;    
            }   
             break;
        case GO_RIGHT:  
            if(get_east(Player.x, Player.y)->walkable && Player.moved == 0 || Player.omni){
            Player.x += 1;
            Player.moved = 1;
            Player.movedAct = abs(Player.movedAct - 1);
            }
            else {
                Player.moved = 0;    
            }   
             break;
        case RUN_UP: 
            if((!Player.stuck && get_north(Player.x, Player.y)->walkable) || Player.omni) {
            Player.y -= 1;
            Player.moved = abs(Player.moved - 1);
            Player.movedAct = abs(Player.movedAct - 1);
            }
             break;
        case RUN_LEFT:  
            if((!Player.stuck && get_west(Player.x, Player.y)->walkable) || Player.omni)
            Player.x -= 1;
            Player.moved = abs(Player.moved - 1);
            Player.movedAct = abs(Player.movedAct - 1);
             break;          
        case RUN_DOWN:  
            if((!Player.stuck && get_south(Player.x, Player.y)->walkable) || Player.omni)
            Player.y += 1;
            Player.moved = abs(Player.moved - 1);
            Player.movedAct = abs(Player.movedAct - 1);
             break;
        case RUN_RIGHT:  
            if((!Player.stuck && get_east(Player.x, Player.y)->walkable) || Player.omni)
            Player.x += 1;
            Player.moved = abs(Player.moved - 1);
            Player.movedAct = abs(Player.movedAct - 1);
             break;
        case ACTION_BUTTON: 
            if(get_east(Player.x, Player.y)->type == 3 || get_west(Player.x, Player.y)->type == 3) { //Mine
                printf("%d\r\n", get_east(Player.x, Player.y)->type);
                Player.sx = Player.sy = -10;
                set_active_map(abs(get_map_num()-1));
                Player.world = abs(Player.world - 1);
                draw_game(true);
                break;
                }
            else if(get_east(Player.x, Player.y)->type == 4 || get_west(Player.x, Player.y)->type == 4) {
                char* test[] = {"Oh, hi there.", "I need some help", "I've been looking", "for some treasure", "but I can't seem", "to find it", "I hear there's", "a tracking device", "down in the mines", "but Im too scared", "to go down there.", "You should go take", "a look"};
                long_speech(test,13);   
                draw_game(true); 
                add_mine(8, 10);
                set_active_map(1);
                add_mine(8, 10);
                set_active_map(0);
                break;
            }  
            else if(get_west(Player.x, Player.y)->type == 5) { 
                map_erase(1,16);
                Player.has_sonar = 1;
                set_active_map(0);
                Player.tx = rand()%48+1;
                Player.ty = rand()%48+1;
                for(int i = 0; i < Player.difficulty * 5; i++) {
                    add_fchest(rand()%48 +1, rand()%48 +1);
                }
                add_chest(Player.tx, Player.ty);
                set_active_map(1);
                speech("You found a sonar", "Go back up!");
                draw_game(true);
                Player.found = 0;
                break;
            }
            else if(get_east(Player.x, Player.y)->type == 6 || get_west(Player.x, Player.y)->type == 6) {
                map_erase(Player.tx, Player.ty);
                Player.tx = rand()%48+1;
                Player.ty = rand()%48+1;
                add_chest(Player.tx, Player.ty);
                if(Player.found) {
                   Player.won = 1;
                   }
                Player.found = 1;
                Player.timer = (4 - Player.difficulty)*100;
                draw_game(true);
                break;
                
                
            }else if(Player.has_sonar && get_map_num() == 0) {
                int diffx = Player.x - Player.tx;
                int diffy = Player.y - Player.ty;
                if(diffx > 0 ){ //Treasure is to the left
                      if(diffy > 0) { //Treasure is above
                            speech("You need to go", "North West");
                            draw_game(true);
                        }else {
                            speech("You need to go", "South West");
                            draw_game(true);
                        }
                    } else {
                        if(diffy > 0) {speech("You need to go", "North East"); draw_game(true);} else {speech("You need to go", "South East");draw_game(true);} //Treasure is to right and above
                        //   speech("You need to go", "North East");     
                       // } else {
                         //   speech("You need to go", "South East");   
                       }
                }
            break;
        case MENU_BUTTON: break;
        case OMNI:
            Player.omni = abs(Player.omni-1);
            speech("Omni toggled","");
            draw_game(true);
            break;
        default:        break;
        
    }
    if(get_here(Player.x, Player.y)->type == THORN &&( Player.x != Player.px || Player.y != Player.py)&& !Player.omni) {
        Player.health -= 1;  
    }
    if(Player.stuck) {
        Player.stuck--;
    }
    if(Player.timer == 0 && Player.has_sonar && Player.world == 0) {
        Player.timer = (4-Player.difficulty) * 100;
        map_erase(Player.tx, Player.ty);
        Player.tx = rand()%48+1;
        Player.ty = rand()%48+1;
        add_chest(Player.tx, Player.ty);
        draw_game(true);
    }
    if(Player.has_sonar && Player.world == 0) {
        Player.timer--;
        }
    return NO_RESULT;
}

/**
 * Entry point for frame drawing. This should be called once per iteration of
 * the game loop. This draws all tiles on the screen, followed by the status 
 * bars. Unless init is nonzero, this function will optimize drawing by only 
 * drawing tiles that have changed from the previous frame.
 */
void draw_game(int init)
{
    int spider = 0;
    
    // Draw game border first
    if(init) draw_border();
    // Iterate over all visible map tiles
    for (int i = -5; i <= 5; i++) // Iterate over columns of tiles
    {
        for (int j = -4; j <= 4; j++) // Iterate over one column of tiles
        {
            // Here, we have a given (i,j)
            
            // Compute the current map (x,y) of this tile
            int x = i + Player.x;
            int y = j + Player.y;
            
            // Compute the previous map (px, py) of this tile
            int px = i + Player.px;
            int py = j + Player.py;
                    
            // Compute u,v coordinates for drawing
            int u = (i+5)*11 + 3;
            int v = (j+4)*11 + 15;
            // Figure out what to draw
            DrawFunc draw = NULL;
            if (init && i == 0 && j == 0) // Only draw the player on init
            {
                //draw_player(u, v, Player.has_key, 0);
                continue;
            }
            else if (x >= 0 && y >= 0 && x < map_width() && y < map_height()) // Current (i,j) in the map
            {
                MapItem* curr_item = get_here(x, y);
                MapItem* prev_item = get_here(px, py);
                if(curr_item->type == SPIDER) {
                    draw_nothing(Player.sx, Player.sy);
                    spider = 1;
                    draw = curr_item->draw;
                    Player.sx = u;
                    Player.sy = v;
                }
                if (init || curr_item != prev_item) // Only draw if they're different
                {
                    if (curr_item) // There's something here! Draw it
                    {
                        draw = curr_item->draw;
                    }
                    else // There used to be something, but now there isn't
                    {
                        draw = draw_nothing;
                    }
                }
            }
            else if (init) // If doing a full draw, but we're out of bounds, draw the walls.
            {
                draw = draw_wall;
            }

            // Actually draw the tile
            if (draw) draw(u, v);

        }
    }
    
    // Draw status bars    
    //draw_upper_status();
    if(get_here(Player.x, Player.y)->type == THORN){
        draw_thornP(58, 59, 0);
        } else {
    draw_player(58, 59, Player.has_sonar, Player.movedAct, Player.stuck);
    }
    uLCD.locate(10,0);
    uLCD.printf("%d ", Player.x);
    uLCD.locate(13,0);
    uLCD.printf("%d ", Player.y);
    draw_lower_status();
    draw_hearts(Player.health, Player.maxHealth);
    if(!spider) {
        draw_nothing(Player.sx, Player.sy);
        }
    uLCD.locate(13,15);
    uLCD.printf("%d ", Player.timer);
}


/**
 * Initialize the main world map. Add walls around the edges, interior chambers,
 * and plants in the background so you can see motion. Note: using the similar
 * procedure you can init the secondary map(s).
 */
void init_main_map()
{
    // "Random" plants
    Map* map = set_active_map(0);
    int index;
    for(int i = 0; i <60; i ++) //39
    {
        index = rand();
        add_plant((index%50050)/1000, (index%50));
    }
    pc.printf("plants\r\n");
    pc.printf("Adding thorns!\r\n");
    for(int i = 0; i <30; i ++) //39
    {
        index = rand();
        add_thorn((index%50050)/1000, (index%50));
    }

    pc.printf("Adding walls!\r\n");
    add_wall(0, 0, HORIZONTAL, map_width());
    pc.printf("1");
    add_wall(0, map_height()-1, HORIZONTAL, map_width());
    pc.printf("2");
    add_wall(0, 0, VERTICAL, map_height());
    pc.printf("3");
    add_wall(map_width()-1,  0, VERTICAL,   map_height());
    pc.printf("Walls done!\r\n");
    
    pc.printf("Adding the mine!\r\n");
    add_npc(10, 15);


    print_map();
    
    map = set_active_map(1);

    pc.printf("Adding walls!\r\n");
    add_wall(0, 0, HORIZONTAL, map_width());
    pc.printf("1");
    add_wall(0, map_height()-1, HORIZONTAL, map_width());
    pc.printf("2");
    add_wall(0, 0, VERTICAL, map_height());
    pc.printf("3");
    add_wall(map_width()-1,  0, VERTICAL,   map_height());
    pc.printf("Walls done!\r\n");
    
    pc.printf("Adding the mine!\r\n");
    add_wall(0, 15, HORIZONTAL, 20);
    add_wall(20, 2, VERTICAL, 14);
    printf("10\n");
    add_wall(0, 5, HORIZONTAL, 19);
    add_wall(2, 3, HORIZONTAL,19);
    add_wall(23, 0, VERTICAL, 10);
    printf("20\n");
    add_wall(23, 12, VERTICAL, 12);
    add_wall(23, 10, HORIZONTAL, 10);
    add_wall(23, 12, HORIZONTAL, 10);
    add_wall(0, 17, HORIZONTAL, 25);
    add_wall(32,10, VERTICAL, 3);
          printf("40\n");
    add_sonar(1, 16);
        printf("30\n");


    print_map();
}
//pc.printf("%d", j);
    //for(int i = 0; i < 130; i++) {
      //  add_thorn(rand()%map_width(), rand()%map_height());
        //}

/**
 * Program entry point! This is where it all begins.
 * This function orchestrates all the parts of the game. Most of your
 * implementation should be elsewhere - this holds the game loop, and should
 * read like a road map for the rest of the code.
 */


int main()
{

    printf("test");
    Timer t;
    Player.sx = Player.sy = -10;
    Player.stuck = 0;
    Player.moved = 0;
    Player.movedAct = 0;
    Player.health = 4;
    Player.maxHealth = 4;
    Player.omni = 0;    
    Player.world = 0;
    Player.difficulty = 1;
    Player.timer = 0;
    int action;
    GameInputs input;
    // First things first: initialize hardware
    ASSERT_P(hardware_init() == ERROR_NONE, pc.printf("Failure"));

    input = read_inputs();
    t.start();
    pc.printf("Waiting\r\n");
    while(input.b1 && input.b2 && input.b3) {
        input = read_inputs();
        }
    t.stop();
    FILE *fp = fopen("/sd/EndMusic.wav", "r");
    FILE *fw = fopen("/sd/BasicGameSong.wav", "r");
    srand(t.read_ms());
    if(input.b1 == 0) {
        Player.difficulty = 1;
    }else if(input.b2 == 0) {
        Player.difficulty = 2;
    } else {
        Player.difficulty = 3;
        }
    // Initialize the maps
    maps_init();
    init_main_map();
    // Initialize game state
    set_active_map(0);
    Map* mapT = get_active_map();
    map_erase(5, 5);
    Player.x = Player.y = Player.px = Player.py = 5;
    // Initial drawing
    draw_game(true);
    Enemy spider;
    spider.x = spider.px = rand()%48+1;
    spider.y = spider.py = rand()%48+1;
    add_spider(spider.x, spider.y);
    print_map();
    while(1)
    {
        // Timer to measure game update speed
        Timer s;
        s.start();
        input = read_inputs();
        action = get_action(input);

        if(Player.health == 0) {
            break;
            }

        if(!get_map_num()){
            updateEnemy(&spider);
            }
        update_game(action);
        // Actually do the game update:
        // 1. Read inputs        
        // 2. Determine action (get_action)        
        // 3. Update game (update_game)
        // 3b. Check for game over
        // 4. Draw frame (draw_game)
        // 5. Frame delay
        draw_game(false);
        if(Player.won){
             uLCD.filled_rectangle(0,0,127,127,0x000000);
                    uLCD.locate(5,5);
                    uLCD.printf("You won!");
                    uLCD.locate(0,8);
                    uLCD.printf("Reset to replay!");
                    waver.play(fw);    
                }
        s.stop();
        int dt = s.read_ms();
        if (dt < 100) wait_ms(100 - dt);
    }
    uLCD.filled_rectangle(0,0,127,127,0x000000);
    uLCD.locate(5,5);
    uLCD.printf("You died!");
    uLCD.locate(0,8);
    uLCD.printf("Reset to try again");
    if(!fp) {
        pc.printf("Null\n");
        }
    pc.printf("HI\n");
    waver.play(fp);
    //FILE *fp = fopen("/sd/BasicGameSong.wav", "r");
    //waver.play(fp);
    //p = fopen("/sd/BasicGameSong.wav", "r");
    //if(fp == NULL) {
   //     printf("Could not open file for write\n");
    //}
    //printf("test");
    //waver.play(fp);
}

void updateEnemy(Enemy* enemy){
        int chance;
        enemy->px = enemy->x;
        enemy->py = enemy->y;
        map_erase(enemy->x, enemy->y);
        printf("X: %d, Y: %d\r\n", enemy->x, enemy->y);
        if(Player.difficulty == 1) {
            chance = 50;
        } else if (Player.difficulty == 2) {
            chance = 75;
        } else {
            chance = 90;
            }
        while(1){
        int dir = rand()%2;
        if(dir == 1) { //Move in the x direction
            if(rand()%100 < chance){ //Nonrandom movement
                if(enemy->x - Player.x > 0) {
                    enemy->x = enemy->x - 1;
                } else if(enemy->x - Player.x < 0) {
                    enemy->x = enemy->x + 1;
                  } 
            } else { //Random movement
                enemy->x = enemy->x + rand()%3 - 1; //Can either stay, move left, or right
              }
        } else {
            if(rand()%100 < chance){ //Nonrandom movement
                if(enemy->y - Player.y > 0) {
                    enemy->y = enemy->y - 1;
                } else if(enemy->y - Player.y < 0) {
                    enemy->y = enemy->y + 1;
                  } 
            } else { //Random movement
                enemy->y = enemy->y + rand()%3 - 1; //Can either stay, move left, or right
            }     
            }
            if(!(get_here(enemy->x, enemy->y))) {
                break;
            }
            }
            add_spider(enemy->x, enemy->y);
            printf("New X: %d, Y: %d\r\n", enemy->x, enemy->y);
            if(Player.x == enemy->x && Player.y == enemy->y) {
                Player.stuck = 10;
                map_erase(enemy->x, enemy->y);
                enemy->x = enemy->px = rand()%48+1;
                enemy->y = enemy->py = rand()%48+1;
                draw_nothing(Player.sx, Player.sy);
                Player.sx = Player.sy = -10;
                }
            
            
    }
