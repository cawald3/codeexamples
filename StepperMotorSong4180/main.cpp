/*
ECE 4180 Project - Stepper motor songs
Conner Awald and Evan Zhang

TO-DO: The program does not exit gracefully when reaching the end of the file. 
We should add a character that indicates the end of the file to the program.


*/

#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

Thread motor;
Thread tpause;

SDFileSystem sd(p5, p6, p7, p8, "sd");
Serial blue(p13,p14);
Serial pc(USBTX, USBRX); // tx, rx

class Note { //I use this to create a note object which contains
            // The pitch, starting time, and duration(stop) of the note
public:
    int midiNote;
    float start;
    float stop;
    Note(int midi, float sta, float sto) {
        start = sta;
        stop = sto;
        midiNote = midi;
        }
    };
        
volatile int pause = 0;

DigitalOut led1(LED2); //Used simply for debugging, could add functionailty
                        // So the led lights up when the corresponding motor is playing
DigitalOut led2(LED3);
DigitalOut led3(LED4);

DigitalOut f1(p21);     //Flipper functions used to generate tones
DigitalOut f2(p22);
DigitalOut f3(p23);

DigitalOut m1e(p17);  //THIS ENABLES THE MOTOR DRIVER PINS. It is important to turn
                        // it off when the motor is not playing a note because
                        //the motor will waste power otherwise in the idle state
DigitalOut m2e(p18);
DigitalOut m3e(p19);

Ticker t1;          //These are the ticker functions that call the flip functions
Ticker t2;
Ticker t3;

float motor1stop; //These are the times each motor is going to stop at in seconds
float motor2stop;
float motor3stop;

char *targmatrix = "/sd/Zelda/notesmatrix.txt"; // testing
char *targstart = "/sd/Zelda/notesstart.txt";
char *targfinish = "/sd/Zelda/notesfinish.txt";

Timer motor1timer; //Used to keep track of the time
 void flip1() //These flip the output pins, generating a pulse
{
    f1 = !f1;
}
void flip2()
{
    f2 = !f2;
}
void flip3(){
    f3 = !f3;
}
/*
Takes the midi number in from the array and then converts it to a frequency in hz;
*/
float midi2freq(float midi){
    return 440.0*powf(2.0, (midi-69)/12);
}

// bluetooth pause thread

void Bpause(void const *arguments){
    while(1){
        char bnum = 0;
        char bhit = 0;
        if (blue.getc()=='!'){
            if (blue.getc()=='B'){
            bnum = blue.getc(); //button number
            bhit = blue.getc(); //1=hit, 0=release
            if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK? 
                switch (bnum) {
                    case '7': //button 7 left arrow play
                            pause = 0;
                            break;
                    case '8': //button 8 right arrow pause
                            pause = 1;
                            break;
                    default:
                            break;
                    }
            }
        }
        }
    Thread::wait(200);
    }
}

// bluetooth input parsing function

int bluehelp(){
    char bnum = 0;
    char bhit = 0;
    if (blue.getc()=='!'){
    if (blue.getc()=='B'){
        bnum = blue.getc(); //button number
        bhit = blue.getc(); //1=hit, 0=release
        if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK? 
            switch (bnum) {
                        case '1': //number button 1 zelda
                                targmatrix = "/sd/Zelda/notesmatrix.txt";
                                targstart = "/sd/Zelda/notesstart.txt";
                                targfinish = "/sd/Zelda/notesfinish.txt";
                                return 1;
                        case '2': //number button 2 meg
                                targmatrix = "/sd/meglovania/notesmatrix.txt";
                                targstart = "/sd/meglovania/notesstart.txt";
                                targfinish = "/sd/meglovania/notesfinish.txt";
                                return 1;
                        case '3': //number button 3 wii
                                targmatrix = "/sd/wii/notesmatrix.txt";
                                targstart = "/sd/wii/notesstart.txt";
                                targfinish = "/sd/wii/notesfinish.txt";
                                return 1;
                        case '4': //number button 4 fight
                                targmatrix = "/sd/FightSong/notesmatrix.txt";
                                targstart = "/sd/FightSong/notesstart.txt";
                                targfinish = "/sd/FightSong/notesfinish.txt";
                                return 1;
                        case '5': //number 5 reset
                                return 5;
                        default:
                            break;
                        }
        }
    }
    }
    return -1;    
}

int main() {
start:
    f1 = 0; //Initialize everything
    f2 = 0;
    f3 = 0;
    m1e = 0;
    m2e = 0;
    m3e = 0;
    motor1stop = 0;
    motor2stop = 0;
    motor3stop = 0;
    
    sd.disk_initialize();
    
    Thread Thread(Bpause); // may not work

     bool holdNotes = 0; //Stop reading notes file once we have the next note
     bool holdStart = 0; //Stop reading start times once we have parsed the starrt
     bool holdFinish = 0; //Stop reading in finish times
     bool fullStop = 0; //setting for finishing a song
     pc.baud(115200); //Baud rate used for debugging if needed
     const char * d;  //This is old when I was messing around with this stuff
                    //But im actually not sure if I used the names later, should probably check
     std::string str = "123.4567";
     d = str.c_str();
    // convert string to float
    float num_float = std::atof(d);

    // convert string to double
    //pc.printf("%f\n", num_float-1.0);
    pc.printf("Im Alive");
    int temp = 0;

    pc.printf("Hello World!");
    string output = "";
    
    // select the song with bluetooth
songselect:
    led2 = 1;
    int songret = bluehelp();
    if (songret != 1)
    {
        led2 = !led2;
        goto songselect;
    }
    // blue helper function should have selected a song for us by here
    pc.printf(targmatrix);
    FILE *fpn = fopen(targmatrix, "r"); //Fpn is a pointer to the notes file that contains the note values
    FILE *fpstart = fopen(targstart, "r"); //Fpstart is a pointer to the file on the sd card that contains the start times
    FILE *fpfinish = fopen(targfinish,"r"); //Fpfinish is a pointer to the file on the sd card that says how long each note should last
    unsigned char c;

    
    Note myNote = Note(0, 0, 0);        //Create a blank note
    int playtime = 0;
    motor1timer.start(); //Start the timer
    led3 = 1;
    while (!fullStop){                        // while not end of file, keep goig
        while(!pause){
            motor1timer.stop();
            m1e = 0;
            m2e = 0;
            m3e = 0;
            Thread::wait(100);
            playtime = 1;    
        }
        if(playtime){
            playtime = 0;
            motor1timer.start();
        }
        while(!holdNotes) { //Make sure we haven't completed the note section yet
           c = fgetc(fpn); // get a character/byte from the file
           if(c == '!'){
                fclose(fpn);
                holdNotes = 1;
                fullStop = 1;
            }                         
           else if(c == ','){                //If the character is a comma, we have parsed the full note
               stringstream degree(output);
               degree >> temp;         //These are weird names I got from the example code I copied
                                        // but basically this converts a string into a float
               //pc.printf("%d\n", temp);
                myNote.midiNote = temp;
               output = "";
               holdNotes = 1;
           } else {
          output += c;      //If this isn't the end of the file, lets add the character we read onto the end
          }
        }

    output = "";            //Reset the output
        while(!holdStart) { //Do the same thing as above but for the start times
            c = fgetc(fpstart);
            if(c == '!'){
                fclose(fpstart);
                holdStart = 1;
            }
            else if(c == ','){
                d = output.c_str();
                num_float = std::atof(d);
                //pc.printf("%f\n", num_float);
                output = "";  
                myNote.start = num_float;
                holdStart = 1;
            } else {
                output+= c;
             }
           
        }
    output = "";
    while(!holdFinish){         //Do the same thing as above with durations
        c = fgetc(fpfinish);
        if(c == '!'){
                fclose(fpfinish);
                holdFinish = 1;
            }
        else if(c == ','){
            d = output.c_str();
            num_float = std::atof(d);
            //pc.printf("%f\n", num_float);
            output = "";  
            myNote.stop = num_float;
            holdFinish = 1;
        } else {
           output+= c;
           }
           
    } //ONCE WE REACH THIS POINT ALL PARTS OF THE NOTE OBJECT SHOULD BE SET and USABLE
    
    /*
    The next three functions check to see if the motors stop time has past
    or will pass in the next ms. If so, it stops the motor from playing and
    disables the pin.
    */
    if((motor1stop-.001)< motor1timer.read()) {
        t1.detach();
        m1e = 0;
    }
    if((motor2stop-.001)< motor1timer.read()){
        t2.detach();
        m2e = 0;
    }
    if((motor3stop-.001)< motor1timer.read()) {
        t3.detach();
        m3e = 0;
    }
    
    
    /*
    This now looks at the note object we currently have parsed in and sees
    if it is time to try and assign it to a motor to be played. If it is not time,
    we are simply going to keep looping until it is time.
    */
    if(myNote.start <= motor1timer.read() && !fullStop){
            if(motor1stop-.002 < motor1timer.read()) { //Check to see if the motor has stopped playing or is going to stop playing
                t1.attach(&flip1, .5/midi2freq(myNote.midiNote)); //
                motor1stop = myNote.stop+motor1timer.read(); //Set the new stop time to be the current time plus the duration of the note
                m1e = 1; //Enable the note
                holdNotes = 0; //Reset the notes control to enable reading the next note
                holdStart = 0;
                holdFinish = 0;
            } else if(motor2stop-.002 < motor1timer.read()) {
                t2.attach(&flip2, .5/midi2freq(myNote.midiNote));
                motor2stop = myNote.stop+motor1timer.read();
                m2e = 1;
                holdNotes = 0; 
                holdStart = 0;
                holdFinish = 0;
            } else if(motor3stop-.002 < motor1timer.read()) {
                t3.attach(&flip3, .5/midi2freq(myNote.midiNote));
                motor3stop = myNote.stop+motor1timer.read();
                m3e = 1;
                holdNotes = 0; 
                holdStart = 0;
                holdFinish = 0;
            } else { //If all three motors are playing something and it is time to play the new note,
                    // We simply do nothing with it and skip it
                holdNotes = 0; 
                holdStart = 0;
                holdFinish = 0;
                }
    } //We have now finished assigning the note to be played
    led1 = !led1;      
    
    //if(!feof(fpn)) goto start;
    
    } //End of while loop for playing    
    motor1stop = 0;
    motor2stop = 0;
    motor3stop = 0;
    
    m1e = 0;
    m2e = 0;
    m3e = 0;
    
    motor1timer.stop();
    motor1timer.reset();
    // close all the files before trying reopen
    //pc.printf("attempt to close files\n");
    //pc.printf("files closed");
    while(bluehelp() != 5){
        Thread::wait(100);
        }
    goto start;
}
