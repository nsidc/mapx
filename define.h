/*----------------------------------------------------------------------
 * define.h - operating system dependent stuff
 *----------------------------------------------------------------------*/
#ifndef define_h_
#define define_h_

#include <assert.h>
#include <limits.h>

#ifdef DEBUG_MALLOC
#include "dbmalloc.h"
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef PI
#define PI 3.141592653589793
#endif

#define radians(t) ( (t) * PI / 180.0)
#define degrees(t) ( (t) * 180.0 / PI)

#define nint(x) ((int)((x)>=0 ? (x)+.5 : (x)-.5))

#define streq(s1,s2) (strcmp(s1,s2) == 0)

#define ABORT -1

#define error_exit(msg) {fprintf(stderr,"%s\n",msg); exit(ABORT);}

#define repeat do
#define until(condition) while(!(condition))

#define MAX_STRING 256

typedef unsigned char byte1;
typedef unsigned short int byte2;
typedef unsigned long int byte4;

typedef char int1;
typedef short int int2;
typedef long int int4;

#define BYTE1_BITS CHAR_BIT
#define BYTE1_MAX UCHAR_MAX
#define BYTE2_MAX USHRT_MAX
#define BYTE4_MAX ULOANG_MAX

#define INT1_MAX SCHAR_MAX
#define INT2_MAX SHRT_MAX
#define INT4_MAX LONG_MAX

#endif
