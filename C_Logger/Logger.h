#ifndef C_LOGGER_INCLUDED
#define C_LOGGER_INCLUDED

#include "ConsoleColor.h"
#define LOGGER_HERE_AND_NOW __FUNCTION__,__LINE__

void Logger_Initialize ( void );
void Logger_SetLogToFile ( uint8_t In_Enable, const char *In_Path, const char *In_Filename );
void Logger_SetExternalLog ( void ( *In_ExternalLog ) ( const uint8_t *In_Buffer ) );
void Logger_EnableColorOutput ( uint8_t In_Enable );
void Logger_Log ( const char *In_Function, const unsigned In_Line, const char *In_String, ... );
void Logger_Error ( const char *In_Function, const unsigned In_Line, const char *In_String, ... );
void Logger_AddBuffer ( const char *In_Function, const unsigned In_Line, const uint8_t *In_Buffer, uint16_t In_Size );

#define LOG_MSG(...) Logger_Log ( LOGGER_HERE_AND_NOW, __VA_ARGS__ );
#define LOG_ERROR(...) Logger_Error ( LOGGER_HERE_AND_NOW, __VA_ARGS__ );
#define LOG_BUFFER(...) Logger_AddBuffer ( LOGGER_HERE_AND_NOW, __VA_ARGS__ );
#endif
