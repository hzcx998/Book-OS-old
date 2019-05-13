#ifndef _MATH_H_
#define _MATH_H_
#include <types.h>

/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

int min(int a, int b);
int max(int a, int b);
int abs(int n);
int pow(int x, int y);

#endif
