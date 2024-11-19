#include "implementation.h"

/*
where im at:

To do:

completed:

Build command (because im too lazy to make a makefile)
gcc -o main main.c getBoard.c -lncurses -lcurl
*/

int main(int argc, char* argv[]){
    int board[BOARDSIZE*BOARDSIZE]; //stores digits on the board
    unsigned int notes[BOARDSIZE*BOARDSIZE]; //stores notes as a bitmask
    bool givenDigits[BOARDSIZE*BOARDSIZE]; //remembers which digits are given by the puzzle
    int cursor; //index of cursor
    
    settingOn(SHOW_NOTES | SHOW_ERRORS | HIGHLIGHT_SIMILAR | AUTONOTE);
    if(!init(board, notes, givenDigits, &cursor)) return 0;

    //mainloop
    

    cleanup();
    return 0;
}