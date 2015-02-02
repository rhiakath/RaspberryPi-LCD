#ifndef MACROS_INCLUDED
#define MACROS_INCLUDED

#include <stdint.h>
#ifndef NULL
#define NULL 0
#endif

#ifndef LOWORD
#define LOWORD(X)   (X & 0xFFFF)
#endif
#ifndef HIWORD
#define HIWORD(X)   ((X>>16) & 0xFFFF)
#endif
#ifndef LOBYTE
#define LOBYTE(X)   (X & 0xFF)
#endif
#ifndef HIBYTE
#define HIBYTE(X)   ((X>>8) & 0xFF)
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define SAFE_DEL(pointer) {if (pointer!=NULL) {delete pointer;pointer=NULL;}}
#define SAFE_DEL_ARRAY(pointer) {if (pointer!=NULL) {delete[] pointer;pointer=NULL;}}
#define SAFE_DEL_C(pointer) {if (pointer!=NULL) {free(pointer);pointer=NULL;}}

#define UNUSED(x) (void)(x)
#ifndef MAX
#define MAX(a,b)    (a>b?a:b)
#endif
#ifndef MIN
#define MIN(a,b)    (a<b?a:b)
#endif
#define MAX_EQ(a,b) (a>=b?a:b)
#define MIN_EQ(a,b) (a<=b?a:b)

//#define limit_frand(min,max) ((static_cast<float>(rand()%((max-min)*10000))/10000)+min)
#define limit_rand(min,max) ((rand()%max)+min)
#define limit_frand(min,max) (min + ( max - min ) * static_cast<float> ( rand() ) / static_cast<float> ( RAND_MAX ))

#define IS_BIT_ENABLED(_Value,_Bit) (( (_Value) >> (_Bit) ) & 1)
#define ENABLE_BIT(_Value,_Bit) ( (_Value) |= (1<< (_Bit) ))
#define DISABLE_BIT(_Value,_Bit) ( (_Value) &= ~(1 << (_Bit) ))
#define BIT_VALUE(_Bit) ( 1 << _Bit )
//(_Value ^= (1<<_Bit))

#ifndef LOGGER_SUPPORT
#define LOG_MSG(...) (void)1;
#define LOG_ERROR(...) (void)1;
#define LOG_BUFFER(...) (void)1;
#endif
#endif
