#ifndef LCD_INCLUDED
#define LCD_INCLUDED
#define _GNU_SOURCE

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
    {
    LED_COLOR_OFF	= 0x00,
    LED_COLOR_RED	= 0x01,
    LED_COLOR_GREEN	= 0x02,
    LED_COLOR_BLUE	= 0x04,
    LED_COLOR_YELLOW	= ( LED_COLOR_RED + LED_COLOR_GREEN ),
    LED_COLOR_TEAL	= ( LED_COLOR_BLUE + LED_COLOR_GREEN ),
    LED_COLOR_VIOLET	= ( LED_COLOR_RED + LED_COLOR_BLUE ),
    LED_COLOR_WHITE	= ( LED_COLOR_RED + LED_COLOR_GREEN + LED_COLOR_BLUE ),
    LED_COLOR_ON	= LED_COLOR_WHITE,
    } ELCDBackgroundColor;

typedef enum
    {
    SELECT = 0,
    RIGHT,
    DOWN,
    UP,
    LEFT
    } EExpanderInputPin;

typedef enum
    {
    // # LCD Commands
    LCD_CLEARDISPLAY        = 0x01,
    LCD_RETURNHOME          = 0x02,
    LCD_ENTRYMODESET        = 0x04,
    LCD_DISPLAYCONTROL      = 0x08,
    LCD_CURSORSHIFT         = 0x10,
    LCD_FUNCTIONSET         = 0x20,
    LCD_SETCGRAMADDR        = 0x40,
    LCD_SETDDRAMADDR        = 0x80,
    } ELCDCommand;

typedef enum
    {
    // # Flags for display on/off control
    LCD_DISPLAYON           = 0x04,
    LCD_DISPLAYOFF          = 0x00,
    LCD_CURSORON            = 0x02,
    LCD_CURSOROFF           = 0x00,
    LCD_BLINKON             = 0x01,
    LCD_BLINKOFF            = 0x00,
    } EDisplayControlFlags;

typedef enum
    {
    //# Flags for display entry mode
    LCD_ENTRYRIGHT          = 0x00,
    LCD_ENTRYLEFT           = 0x02,
    LCD_ENTRYSHIFTINCREMENT = 0x01,
    LCD_ENTRYSHIFTDECREMENT = 0x00,
    } EDisplayEntryModeFlags;

typedef enum
    {
    //# Flags for display/cursor shift
    LCD_DISPLAYMOVE = 0x08,
    LCD_CURSORMOVE  = 0x00,
    LCD_MOVERIGHT   = 0x04,
    LCD_MOVELEFT    = 0x00,
    } EDisplayMoveCursorFlags;

int LCD_Init ( void );
int ReadButtonState ( uint8_t *Out_State );
int LCD_IsButtonPressed ( EExpanderInputPin In_Button );
int LCD_PrintMessage ( char *In_Message, ... );
int LCD_SetBackgroundColor ( ELCDBackgroundColor In_Color );
#endif

