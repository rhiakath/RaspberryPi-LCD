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

// Pinos só quando o banco for 1
#define MCP23017_GPIOA 0x09
#define MCP23017_GPIOB 0x19
#define MCP23017_IODIRB 0x10

// Só para a inicialização
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
 * @brief Os pinos de dados do LCD D4-D7 estão mapeados aos pinos do MCP 12-9 ( PORTB4-1 ), nesta ordem. Como está invertido, um shift não funciona
 * Uso esta tabela para mapear valores de 4 bits para valores directos para a porta, com a inversão e o shift.
 **/
uint8_t Flip[] = { 0b00000000, 0b00010000, 0b00001000, 0b00011000,
                   0b00000100, 0b00010100, 0b00001100, 0b00011100,
                   0b00000010, 0b00010010, 0b00001010, 0b00011010,
                   0b00000110, 0b00010110, 0b00001110, 0b00011110
                 };

/**
 * @brief Para escrever no LCD, tem de se alternar dados com um strobe. Esta função aceita um byte, e mapeia para 4 já "flipados" e com os strobes.
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

int LCD_SendData ( uint8_t *In_Command, uint8_t In_Length, uint8_t In_IsText )
    {
    /*
     * # If pin D7 is in input state, poll LCD busy flag until clear.
        if self.ddrb & 0b00010000:
            lo = (self.portb & 0b00000001) | 0b01000000
            hi = lo | 0b00100000 # E=1 (strobe)
            self.i2c.bus.write_byte_data(
              self.i2c.address, self.MCP23017_GPIOB, lo)
            while True:
                # Strobe high (enable)
                self.i2c.bus.write_byte(self.i2c.address, hi)
                # First nybble contains busy state
                bits = self.i2c.bus.read_byte(self.i2c.address)
                # Strobe low, high, low.  Second nybble (A3) is ignored.
                self.i2c.bus.write_i2c_block_data(
                  self.i2c.address, self.MCP23017_GPIOB, [lo, hi, lo])
                if (bits & 0b00000010) == 0: break # D7=0, not busy
            self.portb = lo
            */
    // Enquanto D7 estiver em modo input, espera...
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

        /*
         *            # Polling complete, change D7 pin to output
                    self.ddrb &= 0b11101111
                    self.i2c.bus.write_byte_data(self.i2c.address,
                      self.MCP23017_IODIRB, self.ddrb)
        */
        // Já está liberto, muda para output
        CachedIODIRB &= 0b11101111;
        I2C_WriteByteToAddress ( I2CFD, MCP23017_IODIRB, CachedIODIRB );
        }

    /*
     *            bitmask = self.portb & 0b00000001   # Mask out PORTB LCD control bits
            if char_mode: bitmask |= 0b10000000 # Set data bit if not a command
    */

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

        // Existe um limite de 32 bytes a serem enviados. Como tal, de 32 em 32 bytes, ou caso esteja já a terminar, despacho o que já tiver...
        if ( ( TXBufferUsed == 32 ) || ( cont == In_Length -1 ) )
            {
            // Guarda estado do ultimo byte enviado na cache.
            CachedGPIOB = TXBuffer[TXBufferUsed-1];

            // Escreve
            I2C_WriteBufferToAddress ( I2CFD, MCP23017_GPIOB, TXBuffer, TXBufferUsed );
            TXBufferUsed = 0;
            }
        }

    /*
    # If a poll-worthy instruction was issued, reconfigure D7
    # pin as input to indicate need for polling on next call.
    if (not char_mode) and (value in self.pollables):
        self.ddrb |= 0b00010000
        self.i2c.bus.write_byte_data(self.i2c.address,
          self.MCP23017_IODIRB, self.ddrb)
    */

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

int LCD_Init ( void )
    {
    I2CFD = I2C_Open ( 1 );
    if ( I2CFD <= 0 )
        {
        printf ( "%s - Unable to open i2c bus\n", __FUNCTION__ );
        return -1;
        }

    I2C_SetSlaveAddress ( I2CFD, LCD_ADDRESS );
    I2C_WriteByteToAddress ( I2CFD, MCP23017_IOCON_BANK1, 0 );

    memset ( Registers, 0, sizeof ( Registers ) );
    CachedIODIRB = 0b00010000;
    CachedGPIOB = 0;
    CachedGPIOA = 0;

    Registers[IODIRA_OFFSET] = 0b00111111; // Red + Green como são leds, são de output. O resto, como são botões, são inputs
    Registers[IODIRB_OFFSET] = CachedIODIRB; // LCD D7 para saber o estado da linha, e o ultimo pino é o Blue, como tal é output
    Registers[IPOLA_OFFSET] = 0b00111111; // Os botões vão ter polaridade inversa
    Registers[IPOLB_OFFSET] = 0b00000000; // Aqui, tudo normal
    Registers[GPINTENA_OFFSET] = 0b00000000; // Desligar interrupt-on-change em tudo
    Registers[GPINTENB_OFFSET] = 0b00000000; // Idem
    Registers[DEFVALA_OFFSET] = 0b00000000; // Desconheço
    Registers[DEFVALB_OFFSET] = 0b00000000; //
    Registers[INTCONA_OFFSET] = 0b00000000; //
    Registers[INTCONB_OFFSET] = 0b00000000; //
    Registers[IOCON1_OFFSET] = 0b00000000; //
    Registers[IOCON2_OFFSET] = 0b00000000; //
    Registers[GPPUA_OFFSET] = 0b00111111; // Pull ups só para os botões
    Registers[GPPUB_OFFSET] = 0b00000000; // Idem
    Registers[INTFA_OFFSET] = 0b00000000;
    Registers[INTFB_OFFSET] = 0b00000000;
    Registers[INTCAPA_OFFSET] = 0b00000000;
    Registers[INTCAPB_OFFSET] = 0b00000000;
    Registers[GPIOA_OFFSET] = CachedGPIOA;
    Registers[GPIOB_OFFSET] = CachedGPIOB;
    Registers[OLATA_OFFSET] = 0b11000000; // Red + Green + 6xlixo
    Registers[OLATB_OFFSET] = 0b00000001; // 7xlixo + Blue

    I2C_WriteBufferToAddress ( I2CFD, 0, ( uint8_t * ) &Registers, sizeof ( Registers ) );


    /*

        # Switch to Bank 1 and disable sequential operation.
        # From this point forward, the register addresses do NOT match
        # the list immediately above.  Instead, use the constants defined
        # at the start of the class.  Also, the address register will no
        # longer increment automatically after this -- multi-byte
        # operations must be broken down into single-byte calls.
        self.i2c.bus.write_byte_data(
          self.i2c.address, self.MCP23017_IOCON_BANK0, 0b10100000)

        */

    I2C_WriteByteToAddress ( I2CFD, MCP23017_IOCON_BANK0, 0b10100000 );


    /* self.displayshift   = (self.LCD_CURSORMOVE |
                               self.LCD_MOVERIGHT)
        self.displaymode    = (self.LCD_ENTRYLEFT |
                               self.LCD_ENTRYSHIFTDECREMENT)
        self.displaycontrol = (self.LCD_DISPLAYON |
                               self.LCD_CURSOROFF |
                               self.LCD_BLINKOFF)
        self.write(0x33) # Init
            self.write(0x32) # Init
            self.write(0x28) # 2 line 5x8 matrix
            self.write(self.LCD_CLEARDISPLAY)
            self.write(self.LCD_CURSORSHIFT    | self.displayshift)
            self.write(self.LCD_ENTRYMODESET   | self.displaymode)
            self.write(self.LCD_DISPLAYCONTROL | self.displaycontrol)
            self.write(self.LCD_RETURNHOME)*/

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

int ReadButtonState ( uint8_t *Out_State )
    {
    /*# Read state of single button
    def buttonPressed(self, b):
        return (self.i2c.readU8(self.MCP23017_GPIOA) >> b) & 1


    # Read and return bitmask of combined button state
    def buttons(self):
        return self.i2c.readU8(self.MCP23017_GPIOA) & 0b11111
    */

    I2C_ReadByteFromAddress ( I2CFD, MCP23017_GPIOA, Out_State );
    *Out_State &= 0b11111;
    return 0;
    }

int LCD_IsButtonPressed ( EExpanderInputPin In_Button )
    {
    uint8_t State;
    if ( I2C_ReadByteFromAddress ( I2CFD, MCP23017_GPIOA, &State ) != 0 )
        return -1;
    return ( ( State >> In_Button ) & 1 );
    }

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

int LCD_SetBackgroundColor ( ELCDBackgroundColor In_Color )
    {
    /*          c          = ~color
            self.porta = (self.porta & 0b00111111) | ((c & 0b011) << 6)
            self.portb = (self.portb & 0b11111110) | ((c & 0b100) >> 2)
            # Has to be done as two writes because sequential operation is off.
            self.i2c.bus.write_byte_data(
              self.i2c.address, self.MCP23017_GPIOA, self.porta)
            self.i2c.bus.write_byte_data(
              self.i2c.address, self.MCP23017_GPIOB, self.portb)
    */

    uint8_t TempColor = ~In_Color;
    CachedGPIOA = (CachedGPIOA & 0b00111111) | (( TempColor & 0b011 ) << 6 );
    CachedGPIOB = (CachedGPIOB & 0b11111110) | (( TempColor & 0b100 ) >> 2 );
    I2C_WriteByteToAddress( I2CFD, MCP23017_GPIOA, CachedGPIOA);
    I2C_WriteByteToAddress( I2CFD, MCP23017_GPIOB, CachedGPIOB);
    return 0;
    }
