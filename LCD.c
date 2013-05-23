#include "LCD.h"
#include "I2C.h"

int I2CFD = 0;
uint8_t Registers[22];
uint8_t CachedIODIRB = 0, CachedGPIOA = 0, CachedGPIOB = 0;

#define LCD_ADDRESS 0x20

#define PIN_D7 0x10

// IOCON when Bank 0 active
#define MCP23017_IOCON_BANK0 0x0A
// IOCON when Bank 1 active
#define MCP23017_IOCON_BANK1 0x15

// To use only after bank is 1
#define MCP23017_GPIOA 0x09
#define MCP23017_GPIOB 0x19
#define MCP23017_IODIRB 0x10

// Offsets for init
#define IODIRA_OFFSET 0
#define IODIRB_OFFSET 1
#define IPOLA_OFFSET 2
#define IPOLB_OFFSET 3
#define GPINTENA_OFFSET 4
#define GPINTENB_OFFSET 5
#define DEFVALA_OFFSET 6
#define DEFVALB_OFFSET 7
#define INTCONA_OFFSET 8
#define INTCONB_OFFSET 9
#define IOCON1_OFFSET 10
#define IOCON2_OFFSET 11
#define GPPUA_OFFSET 12
#define GPPUB_OFFSET 13
#define INTFA_OFFSET 14
#define INTFB_OFFSET 15
#define INTCAPA_OFFSET 16
#define INTCAPB_OFFSET 17
#define GPIOA_OFFSET 18
#define GPIOB_OFFSET 19
#define OLATA_OFFSET 20
#define OLATB_OFFSET 21

/**
 * @brief Small table to convert from one value to the equivalent to send to MCP23017
 * The LCD data pins (D4-D7) connect to MCP pins 12-9 (PORTB4-1), in that order.  Because this sequence is 'reversed,' a direct shift
 * won't work.  This table remaps 4-bit data values to MCP PORTB outputs, incorporating both the reverse and shift.
 **/
uint8_t Flip[] = { 0b00000000, 0b00010000, 0b00001000, 0b00011000,
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
    uint8_t hi = In_Bitmask | Flip[In_Value >> 4];
    uint8_t lo = In_Bitmask | Flip[In_Value & 0x0F];

    Out_Result[0] = hi | 0b00100000;
    Out_Result[1] = hi;
    Out_Result[2] = lo | 0b00100000;
    Out_Result[3] = lo;
    }

/**
 * @brief Sends a command to the LCD
 *
 * @param In_Command ...
 * @param In_Length ...
 * @param In_IsText ...
 * @return int
 **/
int LCD_SendData ( uint8_t *In_Command, uint8_t In_Length, uint8_t In_IsText )
    {
    // While pin D7 is up, it means it's in input state. Since we want to write, we must poll until we can use it
    if ( CachedIODIRB & PIN_D7 )
        {
        uint8_t Low, High;
        Low = ( CachedGPIOB & 0b00000001 ) | 0b01000000;
        High = Low | 0b00100000;
        I2C_WriteByteToAddress ( I2CFD, MCP23017_GPIOB, Low );

        while ( 1 )
            {
            I2C_WriteByte ( I2CFD, High );
            uint8_t Value;
            I2C_ReadByte ( I2CFD, &Value );
            uint8_t TempBuffer[3] = { Low, High, Low };
            I2C_WriteBufferToAddress ( I2CFD, MCP23017_GPIOB, TempBuffer, sizeof ( TempBuffer ) );
            if ( ( Value & 2 ) == 0 )
                break;
            };
        CachedGPIOB = Low;

        // Ok, all set. Set pin to output
        CachedIODIRB &= 0b11101111;
        I2C_WriteByteToAddress ( I2CFD, MCP23017_IODIRB, CachedIODIRB );
        }

    uint8_t Bitmask = CachedGPIOB & 1;
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
            CachedGPIOB = TXBuffer[TXBufferUsed-1];

            // Writes it
            I2C_WriteBufferToAddress ( I2CFD, MCP23017_GPIOB, TXBuffer, TXBufferUsed );
            TXBufferUsed = 0;
            }
        }

    // These two commands MUST have the D7 pin set as input afterwards. otherwise weird stuff happens ( you may try commenting this, and try to write some string, to see what happens )
    if ( ( In_IsText == 0 ) && ( ( In_Command[0] == LCD_CLEARDISPLAY ) || ( In_Command[0] == LCD_RETURNHOME ) ) )
        {
        CachedIODIRB |= 0b00010000;
        I2C_WriteByteToAddress ( I2CFD, MCP23017_IODIRB, CachedIODIRB );
        }
    return 0;
    }

int LCD_SendDataByte ( uint8_t In_Byte )
    {
    return LCD_SendData ( &In_Byte, 1, 0 );
    }

/**
 * @brief Initializes the ports to access the LCD
 *
 * @param In_Bus I2C bus on which to find the LCD
 * @return int
 **/
int LCD_Init ( uint8_t In_Bus )
    {
    I2CFD = I2C_Open ( In_Bus );
    if ( I2CFD <= 0 )
        {
        printf ( "%s - Unable to open i2c bus %d\n", __FUNCTION__, In_Bus );
        return -1;
        }

    I2C_SetSlaveAddress ( I2CFD, LCD_ADDRESS );
    I2C_WriteByteToAddress ( I2CFD, MCP23017_IOCON_BANK1, 0 );

    memset ( Registers, 0, sizeof ( Registers ) );
    CachedIODIRB = 0b00010000;
    CachedGPIOB = 0;
    CachedGPIOA = 0;

    Registers[IODIRA_OFFSET] = 0b00111111; // Since Red + Green are leds, they're set as output ( 0 ). The rest is a left over pin and the buttons, so input ( 1 )
    Registers[IODIRB_OFFSET] = CachedIODIRB; // Pin D7 enabled as line input, and the last pin is LCD background component Blue
    Registers[IPOLA_OFFSET] = 0b00111111; // Set polarity for the inputs/outputs. The buttons have inverse polarity
    Registers[IPOLB_OFFSET] = 0b00000000; //
    Registers[GPINTENA_OFFSET] = 0b00000000; // Don't have any idea what this is...
    Registers[GPINTENB_OFFSET] = 0b00000000; // ditto
    Registers[DEFVALA_OFFSET] = 0b00000000; //
    Registers[DEFVALB_OFFSET] = 0b00000000; //
    Registers[INTCONA_OFFSET] = 0b00000000; //
    Registers[INTCONB_OFFSET] = 0b00000000; //
    Registers[IOCON1_OFFSET] = 0b00000000; //
    Registers[IOCON2_OFFSET] = 0b00000000; //
    Registers[GPPUA_OFFSET] = 0b00111111; // pull up on the buttons
    Registers[GPPUB_OFFSET] = 0b00000000; //
    Registers[INTFA_OFFSET] = 0b00000000; //
    Registers[INTFB_OFFSET] = 0b00000000; //
    Registers[INTCAPA_OFFSET] = 0b00000000; //
    Registers[INTCAPB_OFFSET] = 0b00000000; //
    Registers[GPIOA_OFFSET] = CachedGPIOA; //
    Registers[GPIOB_OFFSET] = CachedGPIOB; //
    Registers[OLATA_OFFSET] = 0b11000000; // Red + Green ( Initial values )
    Registers[OLATB_OFFSET] = 0b00000001; // Blue ( initial values )

    I2C_WriteBufferToAddress ( I2CFD, 0, ( uint8_t * ) &Registers, sizeof ( Registers ) );
    I2C_WriteByteToAddress ( I2CFD, MCP23017_IOCON_BANK0, 0b10100000 );

    LCD_SendDataByte ( 0x33 );
    LCD_SendDataByte ( 0x32 );
    LCD_SendDataByte ( 0x28 );
    LCD_SendDataByte ( LCD_CLEARDISPLAY );
    LCD_SendDataByte ( LCD_CURSORSHIFT | LCD_CURSORMOVE | LCD_MOVERIGHT );
    LCD_SendDataByte ( LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT );
    LCD_SendDataByte ( LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON );
    LCD_SendDataByte ( LCD_RETURNHOME );

    return 0;
    }

/**
 * @brief Return the state for a specific button
 *
 * @param In_Button From the Enum
 * @return int
 **/
int LCD_IsButtonPressed ( EExpanderInputPin In_Button )
    {
    uint8_t State;
    if ( I2C_ReadByteFromAddress ( I2CFD, MCP23017_GPIOA, &State ) != 0 )
        return -1;
    return ( ( State >> In_Button ) & 1 );
    }

/**
 * @brief Return the state of all the buttons
 *
 * @param Out_State The state mask. Must be ANDed with the correct enums
 * @return int
 **/
int LCD_GetButtonPressState ( uint8_t *Out_State )
    {
    if ( I2C_ReadByteFromAddress ( I2CFD, MCP23017_GPIOA, Out_State ) != 0 )
        return -1;
    *Out_State &= 0b11111;
    return 0;
    }

/**
 * @brief Prints a message at the current position
 *
 * @param In_Message ...
 * @param  ...
 * @return int
 **/
int LCD_PrintMessage ( char *In_Message, ... )
    {
    char *Buffer;
    va_list args;
    va_start ( args, In_Message );
    vasprintf ( &Buffer, In_Message, args );
    va_end ( args );

    int Result = LCD_SendData ( ( uint8_t * ) Buffer, strlen ( Buffer ), 1 );
    free ( Buffer );
    return Result;
    }

/**
 * @brief Changes the background color of the LCD
 * The LCD allows for RGB specification of the background color ( 1 bit each ). Red and Green are bits 7 and 6 of GPIOA, while Blue is bit 0 of GPIOB.
 * So we have to do some bit shifting and masking to get these values to the right place...
 *
 * @param In_Color ...
 * @return int
 **/
int LCD_SetBackgroundColor ( ELCDBackgroundColor In_Color )
    {
    // Since the background color seems to be a pull-up, we turn a color component by setting it to 0. Here we invert the bits to reflect that.
    uint8_t TempColor = ~In_Color;
    CachedGPIOA = ( CachedGPIOA & 0b00111111 ) | ( ( TempColor & 0b011 ) << 6 ); // Get the RED and GREEN bits in the appropriate place
    CachedGPIOB = ( CachedGPIOB & 0b11111110 ) | ( ( TempColor & 0b100 ) >> 2 ); // Get the Blue bit in the appropriate place

    // Since we had to disable address incrementing to the MCP23017, we have to write these two bytes in two calls.
    int Result = I2C_WriteByteToAddress ( I2CFD, MCP23017_GPIOA, CachedGPIOA );
    if ( Result != 0 )
        return Result;
    Result = I2C_WriteByteToAddress ( I2CFD, MCP23017_GPIOB, CachedGPIOB );
    return Result;
    }
