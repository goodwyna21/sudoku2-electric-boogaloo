#ifndef IMPLEMENTATION_C
#define IMPLEMENTATION_C

#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include "keymap.h"
#include "getBoard.h"

const unsigned int BOARDSIZE = 9;
const char* PUZZLEFILE = "board.txt";

int ERROR_LOCATION = -1;
unsigned int SETTINGS = 0;
const unsigned int HIGHLIGHT_SIMILAR = 1;
const unsigned int SHOW_NOTES = 1 << 1;
const unsigned int SHOW_ERRORS = 1 << 2;
const unsigned int AUTONOTE = 1 << 3;
const unsigned int INPUT_MODE = 1 << 4;

const unsigned int CELLSIZE = 4;
const unsigned int SQUARESIZE = 3;
const unsigned int LARGEBORDERSIZE = 1;
const unsigned int FULLSIZE = CELLSIZE*(BOARDSIZE-1) + LARGEBORDERSIZE*(BOARDSIZE/SQUARESIZE - 1) + 1;

struct boardState{
    int* board;
    unsigned int* notes;
    bool* givenDigits;
    int cursor;
    bool error;
};

//turns on setting, e.g. showing notes
void settingOn(unsigned int options){ SETTINGS |= options; }
//turns off setting, e.g. showing notes
void settingOff(unsigned int options){ SETTINGS &= ~options; }
//returns the state of setting, e.g. showing notes
bool settingGet(unsigned int option){ return SETTINGS & option; }
//toggles the state of a setting
void settingToggle(unsigned int option){ SETTINGS ^= option; }

//loads in board from file, expects just 81 digits in a row, with 0 being empty squares
//e.g. 006750000093406070005132800807361020009804000362075080028503010004017200751008043
//not used currently
char loadboard(int* board){
    FILE* fptr = fopen(PUZZLEFILE, "r");
    if(fptr == NULL){
        return 0;
    }
    for(int i = 0; i < BOARDSIZE*BOARDSIZE; i++){
        fscanf(fptr, "%1d", &board[i]);
    }
    fclose(fptr);
    return 1;
}

#define COL_DEFAULT 1
#define COL_HLIGHT  2
#define COL_ERROR   3
#define COL_SELECT  4
#define COL_SIMILAR 5
//start up curses and initializes color pairs
void initCurses(){
    initscr();
    keypad(stdscr,true); //keyboard mapping
    cbreak();   //non breaking input
    noecho();   //dont display input
    curs_set(0);//hide cursor

    if(has_colors()){
        start_color();
        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
         * be redefined.  This example uses the same value for the color
         * pair as for the foreground color, though of course that is not
         * necessary:
         */
        init_pair(COL_DEFAULT, COLOR_WHITE, COLOR_BLACK);
        init_pair(COL_HLIGHT,  COLOR_BLACK, COLOR_CYAN);
        init_pair(COL_ERROR,   COLOR_BLACK, COLOR_RED);
        init_pair(COL_SELECT,  COLOR_BLACK, COLOR_YELLOW);
        init_pair(COL_SIMILAR, COLOR_BLACK, COLOR_MAGENTA);
/*
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
*/
    }
}

//draws the board and refreshes the screen
void drawBoard(struct boardState* state){
    attron(COLOR_PAIR(COL_DEFAULT));

    unsigned int large_border_pos[] = {11,23};//{11,12,24,25};

    //main lines
    for(int i = 0; i < sizeof(large_border_pos)/sizeof(large_border_pos[0]); i++){
        mvvline(0,large_border_pos[i],ACS_VLINE,FULLSIZE);
        mvhline(large_border_pos[i],0,ACS_HLINE,FULLSIZE);
    }
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            mvaddch(large_border_pos[i], large_border_pos[j], ACS_PLUS);
        }
    }

    int px, py;
    for(int x = 0; x < BOARDSIZE; x++){
        for(int i = x, y = 0; y < BOARDSIZE; i=++y*BOARDSIZE+x){
            px = x*CELLSIZE + (LARGEBORDERSIZE-1)*x/SQUARESIZE;
            py = y*CELLSIZE + (LARGEBORDERSIZE-1)*y/SQUARESIZE;
            if(state->board[i]){ //cell is known
                if(state->givenDigits[i]) attron(A_BOLD);
                attron((i==state->cursor) ? COLOR_PAIR(COL_SELECT) : COLOR_PAIR(COL_HLIGHT));
                if(settingGet(HIGHLIGHT_SIMILAR) && !(i==state->cursor) && state->board[state->cursor]){
                    attron((state->board[i]==state->board[state->cursor]) ? COLOR_PAIR(COL_SIMILAR) : COLOR_PAIR(COL_HLIGHT));
                }
                if(settingGet(SHOW_ERRORS) && ERROR_LOCATION==i){
                    attron(COLOR_PAIR(COL_ERROR));
                }
                mvaddstr(py  , px, "   ");
                mvaddstr(py+1, px, "   ");
                mvaddstr(py+2, px, "   ");
                mvaddch(py+1,px+1,48+state->board[i]);
                if(state->givenDigits[i]) attroff(A_BOLD);
                attron(COLOR_PAIR(COL_DEFAULT));
            } else if(settingGet(SHOW_NOTES)){ //cell is unknown
                if(i==state->cursor){
                    attron(COLOR_PAIR(COL_SELECT));
                }
                mvaddstr(py  , px, "   ");
                mvaddstr(py+1, px, "   ");
                mvaddstr(py+2, px, "   ");
                for(int j = 0; j < BOARDSIZE; j++){
                    if(state->notes[i] & 1<<j){
                        mvaddch(py + 2-j/(CELLSIZE-1), px + j%(CELLSIZE-1), 49+j);
                    }
                }
                if(i==state->cursor){
                    attron(COLOR_PAIR(COL_DEFAULT));
                }
            }
        }
    }

    mvaddstr(FULLSIZE + 2, FULLSIZE / 2 - 8, "Input Mode: ");
    addstr(settingGet(INPUT_MODE) ? "Notes " : "Digits");

    refresh();
}

//draws keymap information to screen
void drawControls(){
    unsigned int starty = FULLSIZE + 4;
    mvaddstr(starty++,0, "Quit:           ");
    addch(QUIT);
    mvaddstr(starty++,0, "UP:             ");
    addch(UP);
    mvaddstr(starty++,0, "DOWN:           ");
    addch(DOWN);
    mvaddstr(starty++,0, "LEFT:           ");
    addch(LEFT);
    mvaddstr(starty++,0, "RIGHT:          ");
    addch(RIGHT);
    mvaddstr(starty++,0, "SWITCHMODE:     Space");
//    addch(SWITCHMODE);
    mvaddstr(starty++,0, "AUTOFILL:       ");
    addch(AUTOFILL);
    mvaddstr(starty++,0, "AUTONOTE:       ");
    addch(TOGGLE_AUTONOTE);
    mvaddstr(starty++,0, "SHOW HIGHLIGHT: ");
    addch(TOGGLE_SHOWHIGHLIGHT);
    mvaddstr(starty++,0, "SHOW ERROR:     ");
    addch(TOGGLE_SHOWERROR);
    mvaddstr(starty++,0, "SOLVE PUZZLE    ");
    addch(SOLVE_PUZZLE);
    mvaddstr(starty++,0, "TEST VALID      ");
    addch(TEST_VALID);
    mvaddstr(starty++,0, "ALT_ONE:        ");
    addch(ALT_ONE);
    mvaddstr(starty++,0, "ALT_TWO:        ");
    addch(ALT_TWO);
    mvaddstr(starty++,0, "ALT_THREE:      ");
    addch(ALT_THREE);
    mvaddstr(starty++,0, "ALT_FOUR:       ");
    addch(ALT_FOUR);
    mvaddstr(starty++,0, "ALT_FIVE:       ");
    addch(ALT_FIVE);
    mvaddstr(starty++,0, "ALT_SIX:        ");
    addch(ALT_SIX);
    mvaddstr(starty++,0, "ALT_SEVEN:      ");
    addch(ALT_SEVEN);
    mvaddstr(starty++,0, "ALT_EIGHT:      ");
    addch(ALT_EIGHT);
    mvaddstr(starty++,0, "ALT_NINE:       ");
    addch(ALT_NINE);
}

//fills neighbors with the indices of all squares with influence on posx, posy
//neighbors must have length BOARDSIZE*3, and will contain duplicates
void getNeighbors(unsigned int neighbors[], int posx, int posy){
    for(int i = 0; i < BOARDSIZE; i++){
        neighbors[i] = posy*BOARDSIZE + i;
        neighbors[BOARDSIZE+i] = i*BOARDSIZE + posx;
    }
    for(int i = 0; i < SQUARESIZE; i++){
        for(int j = 0; j < SQUARESIZE; j++){
            neighbors[2*BOARDSIZE + SQUARESIZE*i + j] = (SQUARESIZE*(posy/SQUARESIZE) + j)*BOARDSIZE + (SQUARESIZE*(posx/SQUARESIZE) + i);
        }
    }
}

//fills in the state of notes based on whats on the board
void fillNotes(struct boardState* state){
    unsigned int neighbors[BOARDSIZE*3];
    for(int x = 0; x < BOARDSIZE; x++){
        for(int i = x, y = 0; y < BOARDSIZE; i=++y*BOARDSIZE+x){
            state->notes[i] = ~0;
            getNeighbors(neighbors, x, y);
            for(int k = 0; k < BOARDSIZE; k++){
                if(!(state->notes[i] & 1<<k)) continue;
                for(int j = 0; j < BOARDSIZE*3; j++){
                    if(state->board[neighbors[j]] == k+1){
                        state->notes[i] &= ~(1 << k);
                        break;
                    }
                }
            }
        }
    }
}

//fills in which numbers are given by the problem
void fillGiven(struct boardState* state){
    for(int i = 0; i < BOARDSIZE*BOARDSIZE; i++){
        if(state->board[i]){
            state->givenDigits[i] = true;
        }
    }
}

//handles adding a digit to the board in either notes mode or direct insertion
void addDigit(struct boardState* state, int digit){
    if(state->givenDigits[state->cursor]){
        return;
    }
    if(settingGet(INPUT_MODE)){ //notes mode
        state->notes[state->cursor] ^= 1 << (digit-1);
    } else { //direct insertion mode
        state->board[state->cursor] = digit;
    }
    if(settingGet(AUTONOTE)){
        fillNotes(state);
    }
}

//fill in a cell if it has only one note in it (NOT COMPLETE)
void autoCompleteCell(int* board, unsigned int* notes, int cursor){
    
}

void sleep_ms(int milliseconds)
{
    #ifdef WIN32
        Sleep(milliseconds);
    #elif _POSIX_C_SOURCE >= 199309L
        struct timespec ts;
        ts.tv_sec = milliseconds / 1000;
        ts.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&ts, NULL);
    #else
        usleep(milliseconds * 1000);
    #endif
}

//check if the board is valid
//returns -1 if valid, index of error otherwise (NOT WORKING)
int validBoard(struct boardState* state){
    unsigned int neighbors[BOARDSIZE*3];
    fillNotes(state);
    for(int i = 0; i < BOARDSIZE*BOARDSIZE; i++){
        if(state->board[i]){
            getNeighbors(neighbors, i/BOARDSIZE, i%BOARDSIZE);
            for(int j = 0; j < BOARDSIZE*3; j++){
                if(i == j) continue;
                if(state->board[i] == state->board[j]) return i;
            }
        } else {
            if(!state->notes[i]) return i;
        }
    }
    mvaddch(0, FULLSIZE+10, '#');
    return -1;
}

//solve board with a backtracking algorithm (NOT WORKING)
bool backtracking(struct boardState* state){
    //mvaddch(0, FULLSIZE+10, '$');
    for(int i = 0; i < BOARDSIZE*BOARDSIZE; i++){
        if(!state->board[i]){
            for(int n = 0; n < BOARDSIZE; n++){
                state->board[i] = n;
                drawBoard(state);
                if(backtracking(state)){
                    return true;
                }
            }
            return false;
        }
    }
    //mvaddch(0, FULLSIZE+11, '$');
    drawBoard(state);
    return true;
}

//handle keypresses to move cursor, place digit, etc.
bool handleInput(struct boardState* state, int c){
    switch(c){
        case QUIT:
            return false;
        case UP:
            state->cursor-=BOARDSIZE;
            if(state->cursor<0){
                state->cursor+=BOARDSIZE*BOARDSIZE;
            }
            break;
        case DOWN:
            state->cursor+=BOARDSIZE;
            if(state->cursor>=BOARDSIZE*BOARDSIZE){
                state->cursor-=BOARDSIZE*BOARDSIZE;
            }
            break;
        case LEFT:
            if(state->cursor%BOARDSIZE==0){
                state->cursor+=BOARDSIZE-1;
            } else {
                state->cursor--;
            }
            break;
        case RIGHT:
            state->cursor++;
            if(state->cursor%BOARDSIZE==0){
                state->cursor-=BOARDSIZE;
            }
            break;
        case 48 ... 57:
            //number input
            addDigit(state, c-48);
            break;
        case ALT_ONE:
            addDigit(state, 1);
            break;
        case ALT_TWO:
            addDigit(state, 2);
            break;
        case ALT_THREE:
            addDigit(state, 3);
            break;
        case ALT_FOUR:
            addDigit(state, 4);
            break;
        case ALT_FIVE:
            addDigit(state, 5);
            break;
        case ALT_SIX:
            addDigit(state, 6);
            break;
        case ALT_SEVEN:
            addDigit(state, 7);
            break;
        case ALT_EIGHT:
            addDigit(state, 8);
            break;
        case ALT_NINE:
            addDigit(state, 9);
            break;
        case SWITCHMODE:
            settingToggle(INPUT_MODE);
            break;
        case AUTOFILL:
            //todo
            break;
        case TOGGLE_AUTONOTE:
            settingToggle(AUTONOTE);
            break;
        case TOGGLE_SHOWHIGHLIGHT:
            settingToggle(HIGHLIGHT_SIMILAR);
            break;
        case TOGGLE_SHOWERROR:
            //todo
            break;
        case SOLVE_PUZZLE:
            //todo
            break;
        case TEST_VALID:
            ERROR_LOCATION = validBoard(state);
            break;
    }

    drawBoard(state);
    return true;
}

//initialize curses, settings, and boardstate
struct boardState* initGame(){
    struct boardState* state = malloc(sizeof(struct boardState));
    state->board = (int*) calloc(BOARDSIZE*BOARDSIZE, sizeof(int));
    state->notes = (unsigned int*) calloc(BOARDSIZE*BOARDSIZE, sizeof(unsigned int));
    state->givenDigits = (bool*) calloc(BOARDSIZE*BOARDSIZE, sizeof(bool));

    if(state->board == NULL || state->notes == NULL || state->givenDigits == NULL){
        printf("Unable to allocate memory\n");
        state->error = true;
        return state;
    }

    state->cursor = 0;
    state->error = false;

    //fetch board from api
    if(!getBoard(state->board)){
        printf("Unable to fetch board\n");
        state->error = true;
        return state;
    }
  
    initCurses();

    //fill out the board and draw starting info
    fillGiven(state);
    if(settingGet(AUTONOTE)) fillNotes(state);
    drawControls();
    drawBoard(state);
    return state;
}

//run gameloop
void gameloop(struct boardState* state){
    while(handleInput(state, getch()));
}

//might add timer and stuff later
void cleanup(struct boardState* state){
    endwin();
    free(state->board);
    free(state->notes);
    free(state->givenDigits);
    free(state);
}
#endif