#include "LCD.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main ( int argc, char *argv[] )
    {
    LCD_Init();

    char string[] = "test string!!!";
    unsigned cont;
    for ( cont = 0; cont < strlen ( string ); ++cont )
        {
        usleep ( 500000 );
        LCD_PrintMessage ( "%c", string[cont] );
        }

    for ( cont = 0; cont < 8; ++cont )
        {
        usleep ( 500000 );
        LCD_SetBackgroundColor ( cont );
        }
    /*while ( 1 )
        {
        sleep ( 1 );
        if ( LCD_IsButtonPressed ( SELECT ) )
            printf ( "Select\n" );
        if ( LCD_IsButtonPressed ( UP ) )
            printf ( "Up\n" );
        if ( LCD_IsButtonPressed ( DOWN ) )
            printf ( "Down\n" );
        if ( LCD_IsButtonPressed ( LEFT ) )
            printf ( "Left\n" );
        if ( LCD_IsButtonPressed ( RIGHT ) )
            printf ( "Right\n" );
        }*/
    return 0;
    }
