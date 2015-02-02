#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Logger.h"
#include <sys/time.h>

#ifdef LOGGER_THREAD_SUPPORT
#include <pthread.h>
static pthread_mutex_t LoggerMutex;
#define LOCK_MUTEX pthread_mutex_lock ( &LoggerMutex );
#define UNLOCK_MUTEX pthread_mutex_unlock ( &LoggerMutex );
#else
#define LOCK_MUTEX
#define UNLOCK_MUTEX
#endif

static char LogFilename[1024]= {0};
static char LogPath[1024];
static int LogFD = -1;
static bool LogToFilename = false;
static bool Initialized = false;
static bool AllowColor = true;
static char *TemplateString = NULL;
void ( *ExternalLog ) ( const uint8_t *In_Buffer ) = NULL;

void Logger_GetLoggerFilename ( void );
void Logger_AddString ( const char *In_Function, const unsigned In_Line, const char *In_String, int In_Color );

void Logger_GetLoggerFilename ( void )
    {
    char date[11];
    // Vai buscar a data e hora
    struct timeval Now;
    struct tm *tm;

    gettimeofday ( &Now, NULL );
    tm = localtime ( &Now.tv_sec );

    sprintf ( date, "%04u%02u%02u", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );
    sprintf ( LogFilename, "%s/%s/%04u%02u%02u.txt", getenv ( "HOME" ), LogPath, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday );
    }

void Logger_AddString ( const char *In_Function, const unsigned In_Line, const char *In_String, int In_Color )
    {
    char *FullBuffer;
    static char TimeString[25] = "";
    static char ThreadString[20] = "";
    // Vai buscar a data e hora
    struct timeval Now;
    struct tm *tm;

    gettimeofday ( &Now, NULL );
    tm = localtime ( &Now.tv_sec );

    snprintf ( TimeString, sizeof ( TimeString ), "%02u-%02u-%04u %02u:%02u:%02u:%03u", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, ( unsigned ) Now.tv_usec/1000 );
#ifdef LOGGER_THREAD_SUPPORT
    snprintf ( ThreadString, sizeof ( ThreadString ), "%8lX ", pthread_self() );
#endif
    asprintf ( &FullBuffer, TemplateString, TimeString, ThreadString, In_Function, In_Line, In_String );
//      if ( AllowColor )
//          asprintf ( &FullBuffer, "%s%s %s%s (%u) - %s%s\n", TextColorF ( YELLOW ), TimeString, TextColorF ( GREEN ), In_Function, In_Line, ( In_Color == -1?NoTextColor() :TextColorF ( In_Color ) ), In_String );
//      else
//          asprintf ( &FullBuffer, "%s %s (%u) - %s\n", TimeString, In_Function, In_Line, In_String );
    // Envia a mensagem a todos que a quiserem ver
    if ( ExternalLog != NULL )
        ExternalLog ( FullBuffer );
    else
//         fprintf ( stderr, FullBuffer );
        fputs ( FullBuffer, stderr );
    if ( LogToFilename )
        {
        if ( LogFD == -1 )
            LogFD = open ( LogFilename, O_WRONLY );
        if ( LogFD != -1 )
            write ( LogFD, FullBuffer, strlen ( FullBuffer ) );
        }

    free ( FullBuffer );
    }

void Logger_Initialize ( void )
    {
    if ( Initialized == true )
        return;
#ifdef LOGGER_THREAD_SUPPORT
    pthread_mutex_init ( &LoggerMutex, NULL );
#endif
    Logger_EnableColorOutput(AllowColor);
    Initialized = true;
    }

void Logger_SetLogToFile ( uint8_t In_Enable, const char *In_Path, const char *In_Filename )
    {
    LOCK_MUTEX;
    LogToFilename = In_Enable;

    strncpy ( LogFilename, In_Filename, sizeof ( LogFilename ) );
    strncpy ( LogPath, In_Path, sizeof ( LogPath ) );
    UNLOCK_MUTEX;
    }

void Logger_SetExternalLog ( void ( *In_ExternalLog ) ( const uint8_t *In_Buffer ) )
    {
    ExternalLog = In_ExternalLog;
    }

void Logger_EnableColorOutput ( uint8_t In_Enable )
    {
    AllowColor = In_Enable;
    if ( TemplateString )
        free ( TemplateString );

    if ( In_Enable )
        asprintf ( &TemplateString, "%s%%s %s%%s %%s (%%lu) - %s%%s\n", TextColorF ( YELLOW ), TextColorF ( GREEN ), TextColorF ( WHITE ) );
    else
        asprintf ( &TemplateString, "%%s %%s %%s (%%lu) - %%s\n" );
    }

void Logger_Log ( const char *In_Function, const unsigned In_Line, const char *In_String, ... )
    {
    if ( Initialized == false ) Logger_Initialize();
    LOCK_MUTEX;
    // Forma a mensagem
    char *LogMessage;

    va_list args;
    va_start ( args, In_String );
    vasprintf ( &LogMessage, In_String, args );
    va_end ( args );
    if ( LogMessage[strlen ( LogMessage ) - 1] == '\n' )
        LogMessage[strlen ( LogMessage ) - 1] = 0;

    Logger_AddString ( In_Function, In_Line, LogMessage, -1 );
    // Limpa recursos
    free ( LogMessage );
    UNLOCK_MUTEX;
    }

void Logger_Error ( const char *In_Function, const unsigned In_Line, const char *In_String, ... )
    {
    if ( Initialized == false ) Logger_Initialize();
    LOCK_MUTEX;
    // Forma a mensagem
    char *LogMessage;

    va_list args;
    va_start ( args, In_String );
    vasprintf ( &LogMessage, In_String, args );
    va_end ( args );
    if ( LogMessage[strlen ( LogMessage ) - 1] == '\n' )
        LogMessage[strlen ( LogMessage ) - 1] = 0;

    Logger_AddString ( In_Function, In_Line, LogMessage, RED );
    // Limpa recursos
    free ( LogMessage );
    UNLOCK_MUTEX;
    }

void Logger_AddBuffer ( const char *In_Function, const unsigned In_Line, const uint8_t *In_Buffer, uint16_t In_Size )
    {
    if ( Initialized == false ) Logger_Initialize();
    LOCK_MUTEX;
    char Temp[10];
    unsigned cont;
    char FinalString[1024]= {0};
    for ( cont = 0; cont < In_Size; ++cont )
        {
        snprintf ( Temp, 10, "%02X ", In_Buffer[cont] );
        strcat ( FinalString, Temp );
        }
    Logger_AddString ( In_Function, In_Line, FinalString, -1 );
    UNLOCK_MUTEX;
    }

