#ifndef __C_H__
#define __C_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>

#define IS_LITTLE_ENDIAN (*(char*)&(int){1} == 1)

#ifndef likely
#define likely(x) __builtin_expect((x) != 0, 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect((x) != 0, 0)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef int bool;

#ifndef HAVE_INT8
typedef signed char int8;   /* == 8 bits */
typedef signed short int16; /* == 16 bits */
typedef signed int int32;   /* == 32 bits */
#endif                      /* not HAVE_INT8 */

#ifndef HAVE_UINT8
typedef unsigned char uint8;   /* == 8 bits */
typedef unsigned short uint16; /* == 16 bits */
typedef unsigned int uint32;   /* == 32 bits */
typedef unsigned int uint;
#endif                         /* not HAVE_UINT8 */

typedef long int int64;
typedef unsigned long int uint64;

#endif
