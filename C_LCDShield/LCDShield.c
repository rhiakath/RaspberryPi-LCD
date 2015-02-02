// #define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "LCDShield.h"
#ifdef LOGGER_SUPPORT
#include <C_Logger/Logger.h>
#endif

#define I2C_LCD_ADDRESS 0x20
#define IOCON_BANK1 0x15
#define RS_PIN 0x0F
#define RW_PIN 0x0E
#define PIN_D7 0x10

enum
    {
    IODIRA = 0,
    IODIRB,
    IPOLA,
    IPOLB,
    GPINTENA,
    GPINTENB,
    DEFVALA,
    DEFVALB,
    INTCONA,
    INTCONB,
    IOCONA,
    IOCONB,
    GPPUA,
    GPPUB,
    INTFA,
    INTFB,
    INTCAPA,
    INTCAPB,
    GPIOA,
    GPIOB,
    OLATA,
    OLATB,
    } EPins;

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>


/* TODO
 * verificar se com iopol dá para inverter a logica dos botões e do backlight
 */

static uint8_t Indexes[2][22]=
    {
        {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15},
        {0x00,0x10,0x01,0x11,0x02,0x12,0x03,0x13,0x04,0x14,0x05,0x15,0x06,0x16,0x07,0x17,0x08,0x18,0x09,0x19,0x0a,0x1a}
    };
/**
 * @brief Small table to convert from one value to the equivalent to send to MCP23017
 * The LCD data pins (D4-D7) connect to MCP pins 12-9 (PORTB4-1), in that order.  Because this sequence is 'reversed,' a direct shift
 * won't work.  This table remaps 4-bit data values to MCP PORTB outputs, incorporating both the reverse and shift.
 **/
uint8_t FlipTable[] = { 0b00000000, 0b00010000, 0b00001000, 0b00011000,
                        0b00000100, 0b00010100, 0b00001100, 0b00011100,
                        0b00000010, 0b00010010, 0b00001010, 0b00011010,
                        0b00000110, 0b00010110, 0b00001110, 0b00011110
                      };

/**
 * @brief Converts 1 byte to the correct buffer
 * To write to the LCD, you must toggle a strobe. This function takes a byte, and inserts it into a 4 byte buffer with flipped values and strobes included
 *
 * @param In_Bitmask ...
 * @param In_Value ...
 * @param Out_Result ...
 * @return void
 **/
void Get4Bytes ( uint8_t In_Bitmask, uint8_t In_Value, uint8_t *Out_Result )
    {
    uint8_t hi = In_Bitmask | FlipTable[In_Value >> 4];
    uint8_t lo = In_Bitmask | FlipTable[In_Value & 0x0F];

    Out_Result[0] = hi | 0b00100000;
    Out_Result[1] = hi;
    Out_Result[2] = lo | 0b00100000;
    Out_Result[3] = lo;
    }

/**
 * @brief ...
 *
 * @param In_Address ...
 * @return int8_t
 **/
int8_t LCDShield_Initialize ( SLCDShield *In_LCDShield, uint8_t In_Address )
    {
    In_LCDShield->DeviceAddress = I2C_LCD_ADDRESS | ( In_Address % 8 );
    LOG_MSG ( "Initializing LCD shield at device address %02X", In_LCDShield->DeviceAddress );
//     if ( Initialize() != 0 )
//         {
//         Close();
//         return -1;
//         }

    In_LCDShield->DeviceAddress = I2C_LCD_ADDRESS | ( In_Address % 8 );

    if ( I2CDevice_OpenDevice ( & ( In_LCDShield->I2CDevice ),  1, In_LCDShield->DeviceAddress ) != 0 )
        {
        LCDShield_Close ( In_LCDShield );
        return -1;
        }

    // Initialize registers. By default, set all pins to input.
    memset ( In_LCDShield->Registers, 0, sizeof ( In_LCDShield->Registers ) );
    In_LCDShield->Registers[IODIRA] = In_LCDShield->Registers[IODIRB] = 0xFF;

    if ( ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][IOCONA], 0b00100000 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][IODIRA], 0b00111111 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][IODIRB], 0b00010000 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][OLATA], 0b11000000 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][OLATB], 0b00000001 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][GPIOA], 0b11000000 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][GPIOB], 0b00000001 ) != 0 ) ||
            ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][GPPUA], 0b00111111 ) != 0 ) )
        {
        LCDShield_Close ( In_LCDShield );
        return -1;
        }

    if ( LCDShield_WriteRegister ( In_LCDShield, Indexes[In_LCDShield->RegisterBank][IOCONA], 0b10100000 ) != 0 )
        {
        LCDShield_Close ( In_LCDShield );
        return -1;
        }
    In_LCDShield->RegisterBank = 1;

    LOG_MSG ( "Initialization complete" );

    LCDShield_SendDataByte ( In_LCDShield, 0x33 );
    LCDShield_SendDataByte ( In_LCDShield, 0x32 );
    LCDShield_SendDataByte ( In_LCDShield, 0x28 );
    LCDShield_SendDataByte ( In_LCDShield, LCD_CLEARDISPLAY );
    LCDShield_SendDataByte ( In_LCDShield, LCD_CURSORSHIFT | LCD_DMC_CURSORMOVE );
    LCDShield_SendDataByte ( In_LCDShield, LCD_ENTRYMODESET | LCD_EM_ENTRYLEFT | LCD_EM_ENTRYSHIFTDECREMENT );
    In_LCDShield->DisplayControl = LCD_DC_DISPLAYON;
//     In_LCDShield->DisplayControl |= LCD_DC_BLINKON | LCD_DC_CURSORON;
    LCDShield_SendDataByte ( In_LCDShield, LCD_DISPLAYCONTROL | In_LCDShield->DisplayControl );
    LCDShield_SendDataByte ( In_LCDShield, LCD_RETURNHOME );
    LCDShield_SetBacklight ( In_LCDShield, 0 );

    return 0;
    }

/**
 * @brief ...
 *
 * @param  ...
 * @return int8_t
 **/
int8_t LCDShield_Close ( SLCDShield *In_LCDShield )
    {
    return I2CDevice_CloseDevice ( & ( In_LCDShield->I2CDevice ) );
    }

/**
 * @brief ...
 *
 * @param In_Mode ...
 * @return int8_t
 **/
int8_t LCDShield_SetBacklight ( SLCDShield *In_LCDShield, uint8_t In_Mode )
    {
    LOG_MSG ( "Settings backlight to %d", In_Mode );
//     return LCD_SetBackgroundColor ( &MyData, In_Mode );
    In_Mode = ~In_Mode;
    uint8_t GPIOs[2];
    if ( ( LCDShield_ReadRegister ( In_LCDShield, GPIOA, GPIOs ) != 0 ) || ( LCDShield_ReadRegister ( In_LCDShield, GPIOB, GPIOs + 1 ) != 0 ) )
        return -1;

    GPIOs[0] = ( GPIOs[0] & 0b00111111 ) | ( ( In_Mode & 0b011 ) << 6 ); // Get the RED and GREEN bits in the appropriate place
    GPIOs[1] = ( GPIOs[1] & 0b11111110 ) | ( ( In_Mode & 0b100 ) >> 2 ); // Get the Blue bit in the appropriate place
    if ( ( LCDShield_WriteRegister ( In_LCDShield, GPIOA, GPIOs[0] ) != 0 ) || ( LCDShield_WriteRegister ( In_LCDShield, GPIOB, GPIOs[1] ) != 0 ) )
        return -1;
    return 0;
    }

/**
 * @brief ...
 *
 * @param Out_Backlight ...
 * @return int8_t
 **/
int8_t LCDShield_GetBacklight ( SLCDShield *In_LCDShield, uint8_t *Out_Backlight )
    {
    if ( Out_Backlight == NULL )
        return -1;

    uint8_t GPIOs[2];
    if ( ( LCDShield_ReadRegister ( In_LCDShield, GPIOA, GPIOs ) != 0 ) || ( LCDShield_ReadRegister ( In_LCDShield, GPIOB, GPIOs + 1 ) != 0 ) )
        return -1;

    *Out_Backlight = ( GPIOs[0]>> 6 ) & 0b011;
    *Out_Backlight |= ( GPIOs[1] & 1 ) << 2;
    *Out_Backlight = ~ ( *Out_Backlight );
    *Out_Backlight &= 0b111;
    return 0;
    }

/**
 * @brief ...
 *
 * @param Out_ButtonState ...
 * @return int8_t
 **/
int8_t LCDShield_GetButtonState ( SLCDShield *In_LCDShield, uint8_t *Out_ButtonState )
    {
    if ( Out_ButtonState == NULL )
        return -1;

    uint8_t ButtonState = 0;
    if ( LCDShield_ReadRegister ( In_LCDShield, GPIOA, &ButtonState ) != 0 )
        return -1;
    ButtonState &= 0b00011111;
    *Out_ButtonState = ~ButtonState;

    return 0;
    }

/**
 * @brief ...
 *
 * @param In_Message ...
 * @param  ...
 * @return int8_t
 **/
int8_t LCDShield_Print ( SLCDShield *In_LCDShield, char *In_Message, ... )
    {
    char *Buffer;
    va_list args;
    va_start ( args, In_Message );
    vasprintf ( &Buffer, In_Message, args );
    va_end ( args );

//     int8_t Result = LCD_SendData ( &MyData, ( uint8_t * ) Buffer, strlen ( Buffer ), 1 );
    int8_t Result = LCDShield_SendData ( In_LCDShield, ( uint8_t * ) Buffer, strlen ( Buffer ), 1 );
    free ( Buffer );
    return Result;
    }

/**
 * @brief ...
 *
 * @param  ...
 * @return int8_t
 **/
int8_t LCDShield_ReturnHome ( SLCDShield *In_LCDShield )
    {
    return LCDShield_SendDataByte ( In_LCDShield, LCD_RETURNHOME );
    }

/**
 * @brief ...
 *
 * @param In_X ...
 * @param In_Y ...
 * @return int8_t
 **/
int8_t LCDShield_SetCursorPosition ( SLCDShield *In_LCDShield, uint8_t In_X, uint8_t In_Y )
    {
    uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if ( In_Y > 1 ) In_Y = 1;
    return LCDShield_SendDataByte ( In_LCDShield, LCD_SETDDRAMADDR | In_X + row_offsets[In_Y] );
    }

/**
 * @brief ...
 *
 * @param In_Flag ...
 * @return int8_t
 **/
int8_t LCDShield_Shift ( SLCDShield *In_LCDShield, EDisplayMoveCursorFlags In_Flag )
    {
    return LCDShield_SendDataByte ( In_LCDShield, LCD_CURSORSHIFT | LCD_DMC_DISPLAYMOVE | In_Flag );
    }


int8_t LCDShield_ClearDisplay ( SLCDShield *In_LCDShield )
    {
    return LCDShield_SendDataByte ( In_LCDShield, LCD_CLEARDISPLAY );
    }

int8_t LCDShield_EnableDisplay ( SLCDShield *In_LCDShield, uint8_t In_State )
    {
    if ( In_State )
        In_LCDShield->DisplayControl |= LCD_DC_DISPLAYON;
    else
        In_LCDShield->DisplayControl &= ~LCD_DC_DISPLAYON;
    return LCDShield_SendDataByte ( In_LCDShield, LCD_DISPLAYCONTROL | In_LCDShield->DisplayControl );
    }

int8_t LCDShield_EnableCursor ( SLCDShield *In_LCDShield, uint8_t In_State )
    {
    if ( In_State )
        In_LCDShield->DisplayControl |= LCD_DC_CURSORON;
    else
        In_LCDShield->DisplayControl &= ~LCD_DC_CURSORON;
    return LCDShield_SendDataByte ( In_LCDShield, LCD_DISPLAYCONTROL | In_LCDShield->DisplayControl );
    }

int8_t LCDShield_EnableCursorBlink ( SLCDShield *In_LCDShield, uint8_t In_State )
    {
    if ( In_State )
        In_LCDShield->DisplayControl |= LCD_DC_BLINKON;
    else
        In_LCDShield->DisplayControl &= ~LCD_DC_BLINKON;
    return LCDShield_SendDataByte ( In_LCDShield, LCD_DISPLAYCONTROL | In_LCDShield->DisplayControl );
    }

int8_t LCDShield_ScrollDisplay ( SLCDShield *In_LCDShield, uint8_t In_Direction )
    {
    uint8_t Command = LCD_CURSORSHIFT | LCD_DMC_DISPLAYMOVE;
    if ( In_Direction )
        Command |= LCD_DMC_MOVERIGHT;
    return LCDShield_SendDataByte ( In_LCDShield, Command );
    }

/**
 * @brief ...
 *
 * @param In_Command ...
 * @return int8_t
 **/
int8_t LCDShield_SendDataByte ( SLCDShield *In_LCDShield, uint8_t In_Command )
    {
    return LCDShield_SendData ( In_LCDShield, &In_Command, 1, 0 );
    }

/**
 * @brief Sends a command to the LCD, text or data
 *
 * @param In_Command ...
 * @param In_Length ...
 * @param In_IsText 1 if text. 0 if command
 * @return int8_t
 **/
int8_t LCDShield_SendData ( SLCDShield *In_LCDShield, uint8_t *In_Command, uint8_t In_Length, uint8_t In_IsText )
    {
    int Result;
    if ( LCDShield_WaitForOutput ( In_LCDShield ) != 0 )
        return -1;

    LOG_MSG ( "Writing data" );
    uint8_t Bitmask = In_LCDShield->Registers[GPIOB] & 1;
    if ( In_IsText )
        Bitmask |= 0b10000000;

    uint8_t TXBuffer[32];
    uint8_t TXBufferUsed = 0;
    uint8_t cont;
    for ( cont = 0; cont < In_Length; ++cont )
        {
        Get4Bytes ( Bitmask, In_Command[cont], TXBuffer + TXBufferUsed );
        TXBufferUsed+=4;

        // There's a 32 byte limit to each command. So, every 32 bytes, we send what we've got so far
        if ( ( TXBufferUsed == 32 ) || ( cont == In_Length -1 ) )
            {
            // Save last value in our cache
            In_LCDShield->Registers[GPIOB] = TXBuffer[TXBufferUsed-1];

            // Writes it
            Result = LCDShield_WriteBufferToRegister ( In_LCDShield, GPIOB, TXBuffer, TXBufferUsed );
            if ( Result != 0 )
                {
                LOG_MSG ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
                return -1;
                }

            TXBufferUsed = 0;
            }
        }

    // These two commands MUST have the D7 pin set as input afterwards. otherwise weird stuff happens ( you may try commenting this, and try to write some string, to see what happens )
    if ( ( In_IsText == 0 ) && ( ( In_Command[0] == LCD_CLEARDISPLAY ) || ( In_Command[0] == LCD_RETURNHOME ) ) )
        {
        In_LCDShield->Registers[IODIRB] |= 0b00010000;
        Result = LCDShield_WriteRegister ( In_LCDShield, IODIRB, In_LCDShield->Registers[IODIRB] );
        if ( Result != 0 )
            {
            LOG_MSG ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
            return -1;
            }

        }
    return 0;
    }

/**
 * @brief ...
 *
 * @param  ...
 * @return int8_t
 **/
int8_t LCDShield_WaitForOutput ( SLCDShield *In_LCDShield )
    {
    int Result;
    if ( LCDShield_ReadRegister ( In_LCDShield, IODIRB, In_LCDShield->Registers + IODIRB ) != 0 )
        return -1;
    // While pin D7 is up, it means it's in input state. Since we want to write, we must poll until we can use it
    if ( In_LCDShield->Registers[IODIRB] & PIN_D7 )
        {
        LOG_MSG ( "Waiting" );
        uint8_t Low, High;
        Low = ( In_LCDShield->Registers[GPIOB] & 0b00000001 ) | 0b01000000;
        High = Low | 0b00100000;
        Result = LCDShield_WriteRegister ( In_LCDShield, GPIOB, Low );
        if ( Result != 0 )
            {
            LOG_ERROR ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
            return -1;
            }

        while ( 1 )
            {
            Result = LCDShield_WriteRegister ( In_LCDShield, GPIOB, High );
            if ( Result != 0 )
                {
                LOG_ERROR ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
                return -1;
                }
            uint8_t Value;
            Result = LCDShield_ReadRegister ( In_LCDShield, GPIOB, &Value );
            if ( Result != 0 )
                {
                LOG_MSG ( "Unable to read from i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
                return -1;
                }
            uint8_t TempBuffer[3] = { Low, High, Low };
            Result = LCDShield_WriteBufferToRegister ( In_LCDShield, GPIOB, TempBuffer, sizeof ( TempBuffer ) );
            if ( Result != 0 )
                {
                LOG_MSG ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
                return -1;
                }
            if ( ( Value & 2 ) == 0 )
                break;
            };
        In_LCDShield->Registers[GPIOB] = Low;

        // Ok, all set. Set pin to output
        In_LCDShield->Registers[IODIRB] &= 0b11101111;
        Result = LCDShield_WriteRegister ( In_LCDShield, IODIRB, In_LCDShield->Registers[IODIRB] );
        if ( Result != 0 )
            {
            LOG_MSG ( "Unable to write to i2c buffer. %x \"%s\"", Result, strerror ( errno ) );
            return -1;
            }
        }
    return 0;
    }

/**
 * @brief ...
 *
 * @param In_Register ...
 * @param In_Buffer ...
 * @param In_Length ...
 * @return int8_t
 **/
int8_t LCDShield_WriteBufferToRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t *In_Buffer, uint8_t In_Length )
    {
    int Result = I2CDevice_WriteDataToAddress ( & ( In_LCDShield->I2CDevice ), Indexes[In_LCDShield->RegisterBank][In_Register], In_Buffer, In_Length );
    if ( Result != 0 )
        return Result;

    In_LCDShield->Registers[In_Register] = In_Buffer[In_Length-1];
    return 0;
    }
/**
 * @brief ...
 *
 * @param In_Register ...
 * @param In_Value ...
 * @return int8_t
 **/
int8_t LCDShield_WriteRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t In_Value )
    {
    int Result = I2CDevice_WriteByteToAddress ( & ( In_LCDShield->I2CDevice ), Indexes[In_LCDShield->RegisterBank][In_Register], In_Value );
    if ( Result != 0 )
        return Result;

    In_LCDShield->Registers[In_Register] = In_Value;
    return 0;
    }

/**
 * @brief ...
 *
 * @param In_Register ...
 * @param Out_Value ...
 * @return int8_t
 **/
int8_t LCDShield_ReadRegister ( SLCDShield *In_LCDShield, uint8_t In_Register, uint8_t* Out_Value )
    {
    if ( Out_Value == NULL )
        return -1;
    if ( I2CDevice_ReadByteFromAddress ( & ( In_LCDShield->I2CDevice ), Indexes[In_LCDShield->RegisterBank][In_Register], Out_Value ) != 0 )
        return -1;
    In_LCDShield->Registers[In_Register] = *Out_Value;
    return 0;
    }
