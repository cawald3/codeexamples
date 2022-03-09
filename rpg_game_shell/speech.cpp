#include "speech.h"
#include "globals.h"
#include "hardware.h"

/**
 * Draw the speech bubble background.
 */
static void draw_speech_bubble();

/**
 * Erase the speech bubble.
 */
static void erase_speech_bubble();

/**
 * Draw a single line of the speech bubble.
 * @param line The text to display
 * @param which If TOP, the first line; if BOTTOM, the second line.
 */
#define TOP    0
#define BOTTOM 1
static void draw_speech_line(const char* line, int which);

/**
 * Delay until it is time to scroll.
 */
static void speech_bubble_wait();

void draw_speech_bubble()
{

    uLCD.line(0,100,127,100,0xFFFF00);
    uLCD.filled_rectangle(0, 101, 127, 127,0x00000);
    uLCD.line(0,122,127,122, 0xFFFF00);
}

void erase_speech_bubble()
{
    uLCD.filled_rectangle(0, 101, 127, 127,0x00000);
}

void draw_speech_line(const char* line, int which)
{
    uLCD.locate(0, 13+ which);
    uLCD.printf("%s", line);
}

void speech_bubble_wait()
{
    GameInputs input;
    wait(.5);
    while(1) {
        input = read_inputs();
        if (input.b2 == 0) {
            break;
            }
    }
}

void speech(const char* line1, const char* line2)
{
    draw_speech_bubble();
    draw_speech_line(line1, TOP);
    draw_speech_line(line2, BOTTOM);
    speech_bubble_wait();
    erase_speech_bubble();
}

void long_speech(const char* lines[], int n)
{
    for(int i = 0; i < n; i+=2) {
        speech(lines[i], lines[i+1]);
        }
}
