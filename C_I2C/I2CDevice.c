#include "I2CDevice.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#ifdef LOGGER_SUPPORT
#include <C_Logger/Logger.h>
#endif

void I2CDevice_Constructor ( SI2CDevice *In_Device )
    {
    In_Device->BusFileDescriptor = -1;
    }

void I2CDevice_Destructor ( SI2CDevice *In_Device )
    {
    I2CDevice_CloseDevice ( In_Device );
    }

int8_t I2CDevice_OpenDevice ( SI2CDevice *In_Device, uint8_t In_I2CBus, uint8_t In_DeviceAddress )
    {
    I2CDevice_CloseDevice ( In_Device );

    char Temp[50];
    snprintf ( Temp, sizeof ( Temp ), "/dev/i2c-%d", In_I2CBus );
    errno = 0;
    if ( ( In_Device->BusFileDescriptor = open ( Temp, O_RDWR ) ) < 0 )
        {
        LOG_MSG ( "Unable to open filename %s. \"%s\"",Temp, strerror ( errno ) );
        In_Device->BusFileDescriptor = -1;
        return -1;
        }
    if ( ioctl ( In_Device->BusFileDescriptor, I2C_SLAVE, In_DeviceAddress ) < 0 )
        {
        LOG_MSG ( "Unable to do ioctl. \"%s\"", strerror ( errno ) );
        return -2;
        }
    In_Device->DeviceAddress = In_DeviceAddress;
    In_Device->BusAddress = In_I2CBus;
    return 0;
    }

int8_t I2CDevice_CloseDevice ( SI2CDevice *In_Device )
    {
    if ( In_Device->BusFileDescriptor != -1 )
        close ( In_Device->BusFileDescriptor );
    In_Device->BusFileDescriptor = -1;
    return 0;
    }

int I2CDevice_GetDeviceFD ( SI2CDevice *In_Device )
    {
    return In_Device->BusFileDescriptor;
    }

int8_t I2CDevice_SetSlaveDeviceAddress ( SI2CDevice *In_Device, uint8_t In_DeviceAddress )
    {
    if ( ioctl ( In_Device->BusFileDescriptor, I2C_SLAVE, In_DeviceAddress ) < 0 )
        {
        LOG_MSG ( "Unable to do ioctl. \"%s\"", strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int8_t I2CDevice_GetSlaveDeviceAddress ( SI2CDevice *In_Device )
    {
    return In_Device->DeviceAddress;
    }

int8_t I2CDevice_ProbeDevice ( SI2CDevice *In_Device )
    {
    return i2c_smbus_write_byte ( In_Device->BusFileDescriptor, In_Device->DeviceAddress );
    }

int8_t I2CDevice_WriteByte ( SI2CDevice *In_Device, uint8_t In_Data )
    {
    int Result = i2c_smbus_write_byte ( In_Device->BusFileDescriptor, In_Data );
    if ( Result != 0 )
        {
        LOG_MSG ( "Unable to write byte. \"%s\"", strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int8_t I2CDevice_WriteByteToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t In_Data )
    {
    int Result = i2c_smbus_write_byte_data ( In_Device->BusFileDescriptor, In_Address, In_Data );
    if ( Result != 0 )
        {
        LOG_MSG ( "Unable to write byte. \"%s\"", strerror ( errno ) );
        return -1;
        }
    return 0;
    }
int8_t I2CDevice_ReadByteFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *Out_Data )
    {
    int Result = i2c_smbus_read_byte_data ( In_Device->BusFileDescriptor, In_Address );
    if ( Result < 0 )
        {
        LOG_MSG ( "Unable to write byte. \"%s\"", strerror ( errno ) );
        return -1;
        }
    *Out_Data = Result & 0xFF;
    return 0;
    }
int8_t I2CDevice_WriteWordToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint16_t In_Data )
    {
    int Result = i2c_smbus_write_word_data ( In_Device->BusFileDescriptor, In_Address, In_Data );
    if ( Result != 0 )
        {
        LOG_MSG ( "Unable to write word. \"%s\"", strerror ( errno ) );
        return -1;
        }
    return 0;
    }
int8_t I2CDevice_ReadWordFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint16_t* Out_Data )
    {
    int Result = i2c_smbus_read_word_data ( In_Device->BusFileDescriptor, In_Address );
    if ( Result < 0 )
        {
        LOG_MSG ( "Unable to write word. \"%s\"", strerror ( errno ) );
        return -1;
        }
    *Out_Data = Result & 0xFFFF;
    return 0;
    }
int8_t I2CDevice_WriteDataToAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *In_Data, uint8_t In_Length )
    {
    int Result = i2c_smbus_write_i2c_block_data ( In_Device->BusFileDescriptor, In_Address, In_Length, In_Data );
    if ( Result != 0 )
        {
        LOG_MSG ( "Unable to write byte. \"%s\"", strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int8_t I2CDevice_ReadDataFromAddress ( SI2CDevice *In_Device, uint8_t In_Address, uint8_t *Out_Data, uint8_t In_Length )
    {
    int Result = i2c_smbus_read_i2c_block_data ( In_Device->BusFileDescriptor, In_Address, In_Length, Out_Data );
    if ( Result < 0 )
        {
        LOG_MSG ( "Unable to write byte. \"%s\"", strerror ( errno ) );
        }
    return ( Result == In_Length?0:-1 );
    }


