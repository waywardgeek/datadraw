/*
 * This file was written by Bill Cox, originally in 1991, and maintained since.  It is hereby
 * placed into the public domain.
 */

/*============================================================================
   Module : Type Definitions
   Purpose: Provide basic types used to build all objects.  These typedefs
            clarify what we mean when we use an integer type, and make the
            code more portable.
============================================================================*/

#ifndef UTTYPES_H
#define UTTYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

#ifndef UINT64_MAX
#define UINT64_MAX ((uint64)0xffffffffffffffffLL)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX ((uint32)0xffffffff)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX ((uint16)0xffff)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX ((uint8)0xff)
#endif
#ifndef INT64_MAX
#define INT64_MAX ((int64)0x7fffffffffffffffLL)
#endif
#ifndef INT32_MAX
#define INT32_MAX ((int32)0x7fffffff)
#endif
#ifndef INT16_MAX
#define INT16_MAX ((int16)0x7fff)
#endif
#ifndef INT8_MAX
#define INT8_MAX ((int8)0x7f)
#endif
#ifndef INT64_MIN
#define INT64_MIN ((int64)(-1 - INT64_MAX))
#endif
#ifndef INT32_MIN
#define INT32_MIN ((int32)(-1 - INT32_MAX))
#endif
#ifndef INT16_MIN
#define INT16_MIN ((int16)(-1 - INT16_MAX))
#endif
#ifndef INT8_MIN
#define INT8_MIN ((int8)(-1 - INT8_MAX))
#endif
#ifndef DOUBLE_MAX
#define DOUBLE_MAX ((double)1.7e308)
#endif

#if (defined(_WINDOWS) || defined(_WIN32)) && !defined(__GNUC__)
#define UTDIRSEP '\\'
#define UTDIRSEP_STRING "\\"
#define UTPATHSEP ';'
#else
#define UTDIRSEP '/'
#define UTDIRSEP_STRING "/"
#define UTPATHSEP ':'
#endif                                                                         

/*--------------------------------------------------------------------------------------------------
  Compiler optimization hints          example: if(utUnlikely(error)) {utExit("err");} 
--------------------------------------------------------------------------------------------------*/

#if defined(__GNUC__) && __GNUC__ > 3
#define utLikely(x)          __builtin_expect((x),1)    /* assumes gcc version >= 3.0 */
#define utUnlikely(x)        __builtin_expect((x),0)
#define utExpected(x,y)      __builtin_expect((x),(y))
/* Prefetch doesn't fault if addr is invalid. We don't use temporal locality parameter */
#define utPrefetchRead(x)    __builtin_prefetch((x), 0) 
#define utPrefetchWrite(x)   __builtin_prefetch((x), 1) 
/* also add markers for functions that never return (exit, error) using __attribute__ */
#else
#define utLikely(x)          (x)               /* unknown if ms c compiler supports this */
#define utUnlikely(x)        (x)
#define utExpected(x,y)      (x)
#define utPrefetchRead(x)    (x)
#define utPrefetchWrite(x)   (x)
#endif
#define utPrefetch(x)        utPrefetchRead(x)           

/*--------------------------------------------------------------------------------------------------
  handy macros
--------------------------------------------------------------------------------------------------*/
#define utMin(x, y) ((x) <= (y)? (x) : (y))
#define utMax(x, y) ((x) >= (y)? (x) : (y))
#define utAbs(x) ((x) >= 0? (x) : -(x))
#define utUint32ToVoidp(x) ((void *)((uint32)(x) + (char *)NULL))
#define utVoidpToUint32(x) (uint32)((char *)x - (char *)NULL)

/* This loop structure allows loop initialization code to be written
   once (between utDo and utWhile), rather than having to be duplicated */
#define utDo do {
#define utWhile(cond) if(utUnlikely(!(cond))) break;
#define utRepeat } while(true);

/*
  This is needed because MS VS doesn't know "inline" (it's not a C-keyword!).
  GCC is more tolerant.
*/
#ifdef _MSC_VER
#  define utInlineC static __inline
#else
#  define utInlineC static inline
#endif

#endif

