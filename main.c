#include "implementation.h"

#include <stdio.h>
/*
where im at:

To do:

completed:

Build command (because im too lazy to make a makefile)
gcc -o main main.c getBoard.c -lncurses -lcurl
*/

int main(int argc, char* argv[]){
    unsigned int difficulty;
    printf("Enter difficulty:\n1 - Easy\n2 - Medium\n3 - Hard\n>");

    scanf("%d", &difficulty);
    difficulty--;

    struct boardState* state = initGame(difficulty, SHOW_NOTES | SHOW_ERRORS | HIGHLIGHT_SIMILAR);

    if(!state->error) gameloop(state);    

    cleanup(state);
    return 0;
}