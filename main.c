#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <C_LCDShield/LCDShield.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifdef LOGGER_SUPPORT
#include <C_Logger/Logger.h>
#endif

char ColorNames[9][10]= {"Off", "Red", "Green", "Yellow", "Blue", "Violet", "Teal", "White", "On" };

int main ( int argc, char **argv )
    {
    Logger_EnableColorOutput ( 0 );
    SLCDShield LCD;
    int8_t Result = LCDShield_Initialize ( &LCD, 0 );
    if ( Result == 0 )
        {
        unsigned cont;
        for ( cont = 0; cont <= 7; ++cont )
            {
            LCDShield_SetBacklight ( &LCD,cont );
            uint8_t Backlight;
            LCDShield_GetBacklight ( &LCD, &Backlight );
            LCDShield_ReturnHome ( &LCD );
            LCDShield_Print ( &LCD, "%s", ColorNames[cont] );
            LOG_MSG ( "Backlight is %02X", Backlight );
            usleep ( 500000 );
            }
        }
    LCDShield_SetBacklight ( &LCD, 0 );

    // Test cursor
    LOG_MSG ( "Cursor on" );
    LCDShield_EnableCursor ( &LCD,1 );
    sleep ( 2 );
    LOG_MSG ( "Cursor off" );
    LCDShield_EnableCursor ( &LCD,0 );
    sleep ( 2 );

    // Test cursor blink
    LCDShield_EnableCursor ( &LCD,1 );
    LOG_MSG ( "Cursor blink on" );
    LCDShield_EnableCursorBlink ( &LCD,1 );
    sleep ( 2 );
    LOG_MSG ( "Cursor blink off" );
    LCDShield_EnableCursorBlink ( &LCD,0 );
    sleep ( 2 );

    // Test display
    LOG_MSG ( "Display off" );
    LCDShield_EnableDisplay ( &LCD,0 );
    sleep ( 2 );
    LOG_MSG ( "Display on" );
    LCDShield_EnableDisplay ( &LCD,1 );
    sleep ( 2 );

    // Scroll display
    LOG_MSG ( "Scroll left" );
    LCDShield_ScrollDisplay ( &LCD,0 );
    sleep ( 2 );
    LOG_MSG ( "Scroll right" );
    LCDShield_ScrollDisplay ( &LCD,1 );
    sleep ( 2 );

    uint8_t Changes = 0;
    uint8_t LastState, CurrentState;
    LCDShield_GetButtonState ( &LCD,&CurrentState );
    LastState = CurrentState;

    LCDShield_EnableCursor ( &LCD,0 );
    struct timeval Now, Previous;
    struct tm *tm;
    while ( Changes < 10 )
        {
        gettimeofday ( &Now, NULL );
        if ( Now.tv_sec != Previous.tv_sec )
            {
            Previous = Now;
            tm = localtime ( &Now.tv_sec );
            char TimeString[20], DateString[20];
            snprintf ( DateString, sizeof ( DateString ), "%02u-%02u-%04u", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900 );
            snprintf ( TimeString, sizeof ( TimeString ), "%02u:%02u:%02u", tm->tm_hour, tm->tm_min, tm->tm_sec );
            LCDShield_ReturnHome ( &LCD );
            LCDShield_Print ( &LCD,"Es bue de gira!" );
//            LCDShield_Print ( &LCD,DateString );
            LCDShield_SetCursorPosition ( &LCD,0, 1 );
            LCDShield_Print ( &LCD,TimeString );
            LCDShield_SetBacklight ( &LCD, tm->tm_sec%8 );
            }

        LCDShield_GetButtonState ( &LCD,&CurrentState );
        if ( LastState != CurrentState )
            {
            ++Changes;
            LastState = CurrentState;
            LOG_MSG ( "%d %d %d %d %d", IS_BIT_ENABLED ( CurrentState, 0 ), IS_BIT_ENABLED ( CurrentState, 1 ), IS_BIT_ENABLED ( CurrentState, 2 ), IS_BIT_ENABLED ( CurrentState, 3 ), IS_BIT_ENABLED ( CurrentState, 4 ) );
            }

        }

    LCDShield_Close ( &LCD );
    }
