#include "I2C.h"
#include <Platform.h>
#include <C_ATMega_TWI/ATMega_TWI.h>

uint8_t I2C_Initialize ( uint8_t In_Bus )
    {
    UNUSED ( In_Bus );
    TWIInit();
    return 0;
    }

uint8_t I2C_SetSlaveDeviceAddress ( SI2C_Device *In_Device, uint8_t In_SlaveDeviceAddress )
    {
    In_Device->SlaveDeviceAddress = In_SlaveDeviceAddress;
    return 0;
    }

uint8_t I2C_WriteByteToAddress ( SI2C_Device *In_Device, uint8_t In_Address, uint8_t In_Data )
    {
    return I2C_WriteDataToAddress ( In_Device, In_Address, &In_Data, 1 );
    }

uint8_t I2C_WriteDataToAddress ( SI2C_Device *In_Device, uint8_t In_Address, uint8_t *In_Data, uint8_t In_Length )
    {
    UNUSED ( In_Device );
    uint8_t Result = TWIStart ( In_Device->SlaveDeviceAddress, I2C_WRITE );
    if ( Result != 0 )
        return Result;
    Result = TWIWrite ( In_Address );
    if ( Result != 0 )
        return Result;
    uint8_t cont;
    for ( cont = 0; cont < In_Length; ++cont )
        {
        Result = TWIWrite ( In_Data [cont] );
        if ( Result )
            return Result;
        }
    Result = TWIStop ();
    if ( Result != 0 )
        return Result;
    return 0;
    }

uint8_t I2C_ReadByteFromAddress ( SI2C_Device *In_Device, uint8_t In_Address, uint8_t *Out_Data )
    {
    return I2C_ReadDataFromAddress ( In_Device, In_Address, Out_Data, 1 );
    }

uint8_t I2C_ReadDataFromAddress ( SI2C_Device *In_Device, uint8_t In_Address, uint8_t *Out_Data, uint8_t In_Length )
    {
    UNUSED ( In_Device );
    uint8_t Result = TWIStart ( In_Device->SlaveDeviceAddress, I2C_WRITE );
    if ( Result != 0 )
        return Result;
    Result = TWIWrite ( In_Address );
    if ( Result != 0 )
        return Result;
    TWIStop ( );
    Result = TWIStart ( In_Device->SlaveDeviceAddress, I2C_READ );
    if ( Result != 0 )
        return Result;
    unsigned cont;
    for ( cont = 0; cont < In_Length; ++cont )
        {
        Result = TWIRead ();
        Out_Data[cont] = Result;
//     LOG_MSG ( "Reading %d/%d. %02X %s", cont + 1, In_Length, Result, TWCRToString() );
        }
    Result = TWIStop ();
    if ( Result != 0 )
        return Result;
    return 0;
    }
