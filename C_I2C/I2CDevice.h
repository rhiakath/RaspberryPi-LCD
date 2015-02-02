#ifndef I2C_INCLUDED
#define I2C_INCLUDED

#include <stdint.h>
#include "Macros.h"

typedef struct
    {
#ifndef ATMEGA
    int BusFileDescriptor;
#endif
    uint8_t DeviceAddress;
    uint8_t BusAddress;
    } SI2CDevice;

void I2CDevice_Constructor ( SI2CDevice *In_Device );
void I2CDevice_Destructor ( SI2CDevice *In_Device );
int8_t I2CDevice_OpenDevice ( SI2CDevice *In_Device, uint8_t In_I2CBus, uint8_t In_DeviceAddress );
int8_t I2CDevice_CloseDevice ( SI2CDevice *In_Device );
int I2CDevice_GetDeviceFD ( SI2CDevice *In_Device );
int8_t I2CDevice_SetSlaveDeviceAddress ( SI2CDevice *In_Device, uint8_t In_DeviceAddress );
int8_t I2CDevice_GetSlaveDeviceAddress ( SI2CDevice *In_Device );
int8_t I2CDevice_ProbeDevice ( SI2CDevice *In_Device );
int8_t I2CDevice_WriteByte ( SI2CDevice *In_Device, uint8_t In_Data );
int8_t I2CDevice_WriteByteToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t In_Data );
int8_t I2CDevice_ReadByteFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *Out_Data );
int8_t I2CDevice_WriteWordToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint16_t In_Data );
int8_t I2CDevice_ReadWordFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint16_t* Out_Data );
int8_t I2CDevice_WriteDataToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *In_Data, uint8_t In_Length );
int8_t I2CDevice_ReadDataFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *Out_Data, uint8_t In_Length );
#endif
