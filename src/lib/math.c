#include <math.h>

int max(int a, int b)
{
	return a>b?a:b;
}

int min(int a, int b)
{
	return a<b?a:b;
}

int abs(int n)
{
	return n<0?-n:n;
}
/*
I make the python code to c code from net.
So I don't known how does it work, but it runs very well!
2019.4.13 ^.^
return x^y
*/
int pow(int x, int y)
{
	int r = 1;
	while(y > 1){
		if((y&1) == 1){
			r *= x;
		}
		x *= x;
		y >>= 1;
	}
	return r * x;
}
