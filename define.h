/*----------------------------------------------------------------------
 * define.h - header to be included in all C source files
 *	      symbolic constants, useful macros, typedefs
 *
 * National Snow & Ice Data Center, University of Colorado, Boulder
 *----------------------------------------------------------------------*/
#ifndef define_h_
#define define_h_

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

#endif
