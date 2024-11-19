#include "implementation.h"

/*
where im at:

To do:

completed:

Build command (because im too lazy to make a makefile)
gcc -o main main.c getBoard.c -lncurses -lcurl
*/

int main(int argc, char* argv[]){
    struct boardState* state = initGame(SHOW_NOTES | SHOW_ERRORS | HIGHLIGHT_SIMILAR);
    if(state->error) return 0;

    //mainloop
    gameloop(state);    

    cleanup(state);
    return 0;
}