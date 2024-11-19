#ifndef IMPLEMENTATION_H
#define IMPLEMENTATION_H

#include <stdbool.h>

extern const char* PUZZLEFILE;
extern const unsigned int BOARDSIZE; 

extern int ERROR_LOCATION;
extern unsigned int SETTINGS;
extern const unsigned int HIGHLIGHT_SIMILAR;
extern const unsigned int SHOW_NOTES;
extern const unsigned int SHOW_ERRORS;
extern const unsigned int AUTONOTE;
extern const unsigned int INPUT_MODE; //0 - direct insertion, 1 - notes

extern const unsigned int CELLSIZE;
extern const unsigned int SQUARESIZE;
extern const unsigned int LARGEBORDERSIZE;
extern const unsigned int FULLSIZE;

struct boardState;

void settingOn(unsigned int);
bool init(int*, unsigned int*, bool*, int*);
void gameloop(int*, unsigned int*, bool*, int*);
void cleanup();

#endif