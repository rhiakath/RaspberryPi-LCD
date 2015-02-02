#ifndef CONSOLECOLOR_INCLUDED
#define CONSOLECOLOR_INCLUDED

#include <stdio.h>
#include <stdint.h>

#define RESET		0
#define BOLD		1
#define DIM		2
#define UNDERLINE 	4
#define BLINK		5
#define REVERSE		7
//#define HIDDEN		8

#define BLACK 		0
#define RED		1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define CYAN		6
#define	WHITE		7

const char *TextColorF ( int8_t Foreground );
const char *TextColor ( int8_t Foreground, int8_t Background );
const char *NoTextColor ( void );

#endif
