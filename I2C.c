#include "I2C.h"
#include <linux/i2c-dev.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int I2C_Open ( uint8_t In_Bus )
    {
    int FileDescriptor;

    char Filename[256];
    snprintf ( Filename, 256, "/dev/i2c-%d", In_Bus );
    if ( ( FileDescriptor = open ( Filename, O_RDWR ) ) < 0 )
        {
        printf ( "%s - Unable to open filename %s. \"%s\"\n", __FUNCTION__, Filename, strerror ( errno ) );
        return -1;
        }
    return FileDescriptor;
    }

int I2C_Close ( int In_FileDescriptor )
    {
    close ( In_FileDescriptor );
    return 0;
    }

int I2C_SetSlaveAddress ( int In_FileDescriptor, uint8_t In_SlaveAddress )
    {
    printf ( "%s - Setting slave address to 0x%02X\n", __FUNCTION__, In_SlaveAddress );
    if ( ioctl ( In_FileDescriptor, I2C_SLAVE, In_SlaveAddress ) < 0 )
        {
        printf ( "%s - Unable to do ioctl. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int I2C_WriteByte ( int In_FileDescriptor, uint8_t In_Byte )
    {
    return I2C_WriteBuffer ( In_FileDescriptor, &In_Byte, 1 );
    }

int I2C_WriteBuffer ( int In_FileDescriptor, uint8_t *In_Buffer, uint8_t In_Length )
    {
    printf ( "%s - Writing %d bytes - ", __FUNCTION__, In_Length );
    unsigned cont;
    for ( cont = 0; cont < In_Length; ++cont )
        printf ( "%02X ", In_Buffer[cont] );
    printf ( "\n" );
    int Result = write ( In_FileDescriptor, In_Buffer, In_Length );
    if ( Result != In_Length )
        {
        printf ( "%s - Unable to write to i2c buffer. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int I2C_ReadByte ( int In_FileDescriptor, uint8_t* Out_Byte )
    {
    return I2C_ReadBuffer ( In_FileDescriptor, Out_Byte, 1 );
    }

int I2C_ReadBuffer ( int In_FileDescriptor, uint8_t* Out_Buffer, uint8_t In_Length )
    {
    printf ( "%s - Reading %d bytes - ", __FUNCTION__, In_Length );
    int Result = read ( In_FileDescriptor, Out_Buffer, In_Length );
    if ( Result != In_Length )
        {
        printf ( "%s - Unable to read from i2c buffer. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    printf ( "%s - Read %d/%d bytes - ", __FUNCTION__, Result, In_Length );
    unsigned cont;
    for ( cont = 0; cont < In_Length; ++cont )
        printf ( "%02X ", Out_Buffer[cont] );
    printf ( "\n" );
    return 0;
    }

int I2C_WriteByteToAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t In_Byte )
    {
    return I2C_WriteBufferToAddress ( In_FileDescriptor, In_DataAddress, &In_Byte, 1 );
    }

int I2C_WriteBufferToAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *In_Buffer, uint8_t In_Length )
    {
    printf ( "%s - Writing %d bytes to data address %d - ", __FUNCTION__, In_Length, In_DataAddress );
    unsigned cont;
    for ( cont = 0; cont < In_Length; ++cont )
        printf ( "%02X ", In_Buffer[cont] );
    printf ( "\n" );
    uint8_t temp[1024];
    temp[0] = In_DataAddress;
    if ( In_Buffer != NULL ) // Para poder usar esta função só para enviar um endereço, sem dados...
        memcpy ( temp + 1, In_Buffer, In_Length );
    int Result = write ( In_FileDescriptor, temp, In_Length + 1 );
    if ( Result != In_Length + 1 )
        {
        printf ( "%s - Unable to write to i2c buffer. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    return 0;
    }

int I2C_ReadByteFromAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *Out_Byte )
    {
    return I2C_ReadBufferFromAddress ( In_FileDescriptor, In_DataAddress, Out_Byte, 1 );
    }

int I2C_ReadBufferFromAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *Out_Buffer, uint8_t In_Length )
    {
    int Result = write ( In_FileDescriptor, &In_DataAddress, sizeof ( In_DataAddress ) );
    if ( Result != sizeof ( In_DataAddress ) )
        {
        printf ( "%s - Unable to write to i2c buffer. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    Result = read ( In_FileDescriptor, Out_Buffer, In_Length );
    if ( Result != In_Length )
        {
        printf ( "%s - Unable to read from i2c buffer. \"%s\"\n", __FUNCTION__, strerror ( errno ) );
        return -1;
        }
    printf ( "%s - Read %d/%d bytes from data address %d - ", __FUNCTION__, Result, In_Length, In_DataAddress );
    int cont;
    for ( cont = 0; cont < Result; ++cont )
        printf ( "%02X ", Out_Buffer[cont] );
    printf ( "\n" );

    return 0;
    }

