# RPG GAME

This was a project for a class I took that focused on low level design. This project used a mbed [LPC1768](https://os.mbed.com/platforms/mbed-LPC1768/ "LPC1768") to run a very simple 2-D RPG game. The user interacted with the game through pushbuttons and tilt controls implemented using an accelerometer. The graphics for the game were displayed on a color LCD. A soundtrack was also provided for the game by using a SD card and speaker to store and play music while the user was playing the game. 

Game objects on the map were stored in a hash table implementation using a doubly linked list that was written and debugged as part of an earlier project. This project was tested extensively using GTest and memory leaks were diagnosed and fixed with Memcheck by Valgrind.


Some of the h and cpp files were provided as empty templates but I completed most of the implementations in these files.
