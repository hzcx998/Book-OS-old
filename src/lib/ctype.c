#include <ctype.h>

int isspace(char c)
{
	char comp[] = {' ', '\t', '\n', '\r', '\v', '\f'};
	int i;
	const int len = 6; //comp数组的长度，这个你也可以用strlen()来求，但是要包括string.h头文件
	// 也可以使用宏来定义
	for (i = 0; i < len; i++) {
		if (c == comp[i])
			return 1;
	}
	return 0;
}

//测试参数ch是否是字母(A-Z,大小写均可)或数字（0-9）
int isalnum(int ch)
{
	return (unsigned int)((ch | 0x20) - 'a') < 26u ||
		(unsigned int)( ch - '0') < 10u;
}

/*
判断是否是16进制字母 0123456789abcdefABCDEF
*/
int isxdigit (int c)
{
	if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) {
		return 1;
	}
	return 0;
}

//判断字符c是否为数字
int isdigit( int ch )
{
    return (unsigned int)(ch - '0') < 10u;
}


#define TOLOWER(x) ((x) | 0x20)
/*
#define isxdigit(c)    (('0' <= (c) && (c) <= '9') /
             || ('a' <= (c) && (c) <= 'f') /
             || ('A' <= (c) && (c) <= 'F'))
 
#define isdigit(c)    ('0' <= (c) && (c) <= '9')
*/
unsigned long strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;
 
    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((TOLOWER(*cp) == 'x') && isxdigit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    } else if (base == 16) {
        if (cp[0] == '0' && TOLOWER(cp[1]) == 'x')
            cp += 2;
    }
    while (isxdigit(*cp) &&
           (value = isdigit(*cp) ? *cp-'0' : TOLOWER(*cp)-'a'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}
long strtol(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -strtoul(cp+1,endp,base);
    return strtoul(cp,endp,base);
}

int isalpha(int ch)
{
	if (('a' <= ch && ch <= 'z') || ('a' <= ch && ch <= 'Z')) {
		return 1;
	}
	return 0;
}

/*

   这个函数是把浮点数字符串转换为浮点数的函数。

   函数将会跳过字符串中的空格字符和不是'+'、'-'、'.'、

   数字的字符。如果字符串是空的或者都是由空格组成，将不会

   做任何转换，仅仅是把字符串的结束地址赋给endptr。如果字

   符串合法，将会进行转换，并把字符串最后的NULL的地址给

   endptr。如果你想使用endptr参数，那么赋一个NULL值就

   可以了。

*/

double strtod(const char* s, char** endptr)
{
	/*register const char*  p     = s;
	register long double  value = 0.L;
	int                   sign  = 0;
	long double           factor;

	unsigned int          expo;

	while ( isspace(*p) )//跳过前面的空格
		p++;
	    if(*p == '-' || *p == '+')
   		sign = *p++;//把符号赋给字符sign，指针后移。

   //处理数字字符
    while ( (unsigned int)(*p - '0') < 10u )//转换整数部分
      value = value*10 + (*p++ - '0');
   //如果是正常的表示方式（如：1234.5678）
   if ( *p == '.' )
   {
        factor = 1.;
        p++;
        while ( (unsigned int)(*p - '0') < 10u )
      {
         factor *= 0.1;
            value  += (*p++ - '0') * factor;
        }
    }
   //如果是IEEE754标准的格式（如：1.23456E+3）
    if ( (*p | 32) == 'e' )
   {
      expo   = 0;
        factor = 10.L;
        switch (*++p)
      {
        case '-':
           factor = 0.1;
        case '+':
           p++;
           break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
           break;
        default :
           value = 0.L;
           p     = s;
           goto done;
        }
        while ( (unsigned int)(*p - '0') < 10u )
            expo = 10 * expo + (*p++ - '0');
        while ( 1 )
      {
        if ( expo & 1 )
           value *= factor;
            if ( (expo >>= 1) == 0 )
                break;
            factor *= factor;
        }
    }
done:
    if ( endptr != 0 )
        *endptr = (char*)p;
    return (sign == '-' ? -value : value);*/
}

double atof(char *str)
{
	return strtod(str,0);
}
