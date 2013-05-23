#include "LCD.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char ColorNames[9][10]= {"Off", "Red", "Green", "Yellow", "Blue", "Violet", "Teal", "White", "On" };

int main ( int argc, char *argv[] )
    {
    LCD_Init ( 1 );

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
        LCD_SendDataByte ( LCD_CLEARDISPLAY );
        LCD_SendDataByte ( LCD_RETURNHOME );
        LCD_SetBackgroundColor ( cont );
        LCD_PrintMessage ( "%s", ColorNames[cont] );
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
