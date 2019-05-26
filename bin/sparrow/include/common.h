#ifndef _INCLUDE_COMMON_H
#define _INCLUDE_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

typedef struct vm VM;
typedef struct parser Parser;
typedef struct class Class;

#ifndef bool
   #define bool char
#endif

#ifndef true
   #define true   1
#endif

#ifndef false
   #define false  0
#endif


#define UNUSED __attribute__ ((unused))

#ifdef DEBUG
   #define ASSERT(condition, errMsg) \
      do {\
	 if (!(condition)) {\
	    fprintf(stderr, "ASSERT failed! %s:%d In function %s(): %s\n", \
	       __FILE__, __LINE__, __func__, errMsg); \
	    abort();\
	 }\
      } while (0);
#else
   #define ASSERT(condition, errMsg) ((void)0)
#endif

#define NOT_REACHED()\
   do {\
      while (1);\
   } while (0);

/*
#define NOT_REACHED()\
   do {\
      fprintf(stderr, "NOT_REACHED: %s:%d In function %s()\n", \
	 __FILE__, __LINE__, __func__);\
      while (1);\
   } while (0);

#endif
*/

#endif
