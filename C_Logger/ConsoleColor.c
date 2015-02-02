#include "ConsoleColor.h"

static char TextColorStrings[8][8][20];
static char TextColorForegroundStrings[8][20];
static char TextColorReset[20];

static uint8_t TextColorStringInitialized = 0;

void InitializeTextColorStrings ( void )
    {
    int b, f;
    for ( f = 0; f < 8; ++f )
        {
        for ( b = 0; b < 8; ++b )
            {
            sprintf ( TextColorStrings[f][b], "%c[%d;%dm", 0x1B, f+30, b+40 );
            }
        }

    for ( f = 0; f < 8; ++f )
        {
        sprintf ( TextColorForegroundStrings[f], "%c[0;%dm", 0x1B, f + 30 );
        }
    sprintf ( TextColorReset, "%c[0m", 0x1B );
    TextColorStringInitialized = 1;
    }

const char *TextColor ( int8_t Foreground, int8_t Background )
    {
    if ( TextColorStringInitialized == 0 )
        InitializeTextColorStrings();
    return ( TextColorStrings[Foreground][Background] );
    }

const char *NoTextColor ( void )
    {
    if ( TextColorStringInitialized == 0 )
        InitializeTextColorStrings();
    return TextColorReset;
    }

const char *TextColorF ( int8_t Foreground )
    {
    if ( TextColorStringInitialized == 0 )
        InitializeTextColorStrings();
    return TextColorForegroundStrings[Foreground];
    }
