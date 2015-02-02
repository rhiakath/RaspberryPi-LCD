#ifndef LCDSHIELD_INCLUDED
#define LCDSHIELD_INCLUDED

#include <C_I2C/I2CDevice.h>

typedef struct
    {
    SI2CDevice I2CDevice;
    uint8_t Registers[22];
    uint8_t RegisterBank;
    uint8_t DeviceAddress;
    uint8_t DisplayControl;
    } SLCDShield;


typedef enum
    {
    //# Flags for display/cursor shift
    LCD_DMC_DISPLAYMOVE = 0x08,
    LCD_DMC_CURSORMOVE  = 0x00,
    LCD_DMC_MOVERIGHT   = 0x04,
    LCD_DMC_MOVELEFT    = 0x00,
    } EDisplayMoveCursorFlags;
    
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
    LCD_DC_DISPLAYON           = 0x04,
    LCD_DC_DISPLAYOFF          = 0x00,
    LCD_DC_CURSORON            = 0x02,
    LCD_DC_CURSOROFF           = 0x00,
    LCD_DC_BLINKON             = 0x01,
    LCD_DC_BLINKOFF            = 0x00,
    } EDisplayControlFlags;

typedef enum
    {
    //# Flags for display entry mode
    LCD_EM_ENTRYRIGHT          = 0x00,
    LCD_EM_ENTRYLEFT           = 0x02,
    LCD_EM_ENTRYSHIFTINCREMENT = 0x01,
    LCD_EM_ENTRYSHIFTDECREMENT = 0x00,
    } EDisplayEntryModeFlags;

typedef enum
    {
    Select = 0x01,
    Right  = 0x02,
    Down   = 0x04,
    Up     = 0x08,
    Left   = 0x10,

    SelectPin = 0,
    RightPin  = 1,
    DownPin   = 2,
    UpPin     = 3,
    LeftPin   = 4
    } EButton;
    
typedef enum
    {
    Off,
    Red,
    Green,
    Yellow,
    Blue,
    Violet,
    Teal,
    White
    } EBacklight;

int8_t LCDShield_Initialize ( SLCDShield *In_LCDShield, uint8_t In_Address );
int8_t LCDShield_Close ( SLCDShield *In_LCDShield );
int8_t LCDShield_SetBacklight ( SLCDShield *In_LCDShield, uint8_t In_Mode );
int8_t LCDShield_GetBacklight ( SLCDShield *In_LCDShield, uint8_t *Out_Backlight );
int8_t LCDShield_GetButtonState ( SLCDShield *In_LCDShield, uint8_t *Out_ButtonState );
int8_t LCDShield_Print ( SLCDShield *In_LCDShield, char *In_Message, ... );
int8_t LCDShield_ReturnHome ( SLCDShield *In_LCDShield );
int8_t LCDShield_SetCursorPosition ( SLCDShield *In_LCDShield, uint8_t In_X, uint8_t In_Y );
int8_t LCDShield_Shift ( SLCDShield *In_LCDShield, EDisplayMoveCursorFlags In_Flag );
int8_t LCDShield_ClearDisplay ( SLCDShield *In_LCDShield );
int8_t LCDShield_EnableDisplay ( SLCDShield *In_LCDShield, uint8_t In_State );
int8_t LCDShield_EnableCursor ( SLCDShield *In_LCDShield, uint8_t In_State );
int8_t LCDShield_EnableCursorBlink ( SLCDShield *In_LCDShield, uint8_t In_State );
int8_t LCDShield_ScrollDisplay ( SLCDShield *In_LCDShield, uint8_t In_Direction );
int8_t LCDShield_SendDataByte ( SLCDShield *In_LCDShield, uint8_t In_Command );
int8_t LCDShield_SendData ( SLCDShield *In_LCDShield, uint8_t *In_Command, uint8_t In_Length, uint8_t In_IsText );
int8_t LCDShield_WaitForOutput ( SLCDShield *In_LCDShield );
int8_t LCDShield_WriteBufferToRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t *In_Buffer, uint8_t In_Length );
int8_t LCDShield_WriteRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t In_Value );
int8_t LCDShield_ReadRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t* Out_Value );
#endif


