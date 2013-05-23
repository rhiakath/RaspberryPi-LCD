#ifndef I2C_INCLUDED
#define I2C_INCLUDED
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int I2C_Open ( uint8_t In_Bus );
int I2C_Close ( int In_FileDescriptor );
int I2C_SetSlaveAddress ( int In_FileDescriptor, uint8_t In_SlaveAddress );

int I2C_WriteByte ( int In_FileDescriptor, uint8_t In_Byte );
int I2C_WriteBuffer ( int In_FileDescriptor, uint8_t *In_Buffer, uint8_t In_Length );
int I2C_ReadByte ( int In_FileDescriptor, uint8_t *Out_Byte );
int I2C_ReadBuffer ( int In_FileDescriptor, uint8_t *Out_Buffer, uint8_t In_Length );

int I2C_WriteByteToAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t In_Byte );
int I2C_WriteBufferToAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *In_Buffer, uint8_t In_Length );
int I2C_ReadByteFromAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *Out_Byte );
int I2C_ReadBufferFromAddress ( int In_FileDescriptor, uint8_t In_DataAddress, uint8_t *Out_Buffer, uint8_t In_Length );

#endif
