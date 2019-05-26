#include <string.h>

int strncmp (const char * s1, const char * s2, int n)
{ 
	if(!n)return(0);

	while (--n && *s1 && *s1 == *s2)
	{ 
		s1++;
		s2++;
	}
	return( *s1 - *s2 );
}

char * itoa(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;

	*p++ = '0';
	*p++ = 'x';
	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){	
			ch = (num >> i) & 0xF;
			
			ch += '0';			
			if(ch > '9'){	
				ch += 7;		
			}
			*p++ = ch;			
			
		}
	}
	*p = 0;					
	return str;
}


int atoi(const char *src)
{
    int s = 0;
    char is_minus = 0;
  
    while (*src == ' ') {
			src++; 
		}
  
	if (*src == '+' || *src == '-') {
        if (*src == '-') {
           is_minus = 1;
        }
        src++;
    } else if (*src < '0' || *src > '9') {
		 s = 2147483647;
        return s;
    }
  
    while (*src != '\0' && *src >= '0' && *src <= '9') {
        s = s * 10 + *src - '0';
        src++;
    }
    return s * (is_minus ? -1 : 1);
}

void *memset(void* src, uint8_t value, uint32_t size) 
{
	uint8_t* s = (uint8_t*)src;
	while (size-- > 0){
		*s++ = value;
	}
	return src;
}

void *memset16(void* src, uint16 value, uint32_t size) 
{
	uint16* s = (uint16*)src;
	while (size-- > 0){
		*s++ = value;
	}
	return src;
}

void *memset32(void* src, uint32_t value, uint32_t size) 
{
	uint32_t* s = (uint32*)src;
	while (size-- > 0){
		*s++ = value;
	}
	return src;
}

void memcpy(void* dst_, const void* src_, uint32_t size) {
 
   uint8_t* dst = dst_;
   const uint8_t* src = src_;
   while (size-- > 0)
      *dst++ = *src++;
}

char* strcpy(char* dst_, const char* src_) {
  
   char* r = dst_;		       
   while((*dst_++ = *src_++));
   return r;
}

char* strncpy(char* dst_, char* src_, int n) 
{
  
   char* r = dst_;		      
   while((*dst_++ = *src_++) && n > 0) n--;
   return r;
}

uint32_t strlen(const char* str) {
  
   const char* p = str;
   while(*p++);
   return (p - str - 1);
}

int8_t strcmp (const char* a, const char* b) {
  
   while (*a != 0 && *a == *b) {
      a++;
      b++;
   }
   return *a < *b ? -1 : *a > *b;
}

int memcmp(const void * s1, const void *s2, int n)
{
	if ((s1 == 0) || (s2 == 0)) { /* for robustness */
		return (s1 - s2);
	}

	const char * p1 = (const char *)s1;
	const char * p2 = (const char *)s2;
	int i;
	for (i = 0; i < n; i++,p1++,p2++) {
		if (*p1 != *p2) {
			return (*p1 - *p2);
		}
	}
	return 0;
}
char* strrchr(char* str, int c)
{
   
    char* ret = NULL;
    while (*str)
    {
        if (*str == (char)c)
            ret = (char *)str;
        str++;
    }
    if ((char)c == *str)
        ret = (char *)str;

    return ret;
}

char* strcat(char* strDest , const char* strSrc)
{
    char* address = strDest;
    while(*strDest)
    {
        strDest++;
    }
    while((*strDest++=*strSrc++));
    return (char* )address;
}

int strpos(char *str, char ch)
{
	int i = 0;
	int flags = 0;
	while(*str){
		if(*str == ch){
			flags = 1;	//find ch
			break;
		}
		i++;
		str++;
	}
	if(flags){
		return i;
	}else{
		return -1;	//str over but not found
	}
}


char *strncat(char *dst, const char *src, int n)
{
	char *ret = dst;
	while(*dst != '\0'){
		dst++;
	}
	while(n && (*dst++ = *src++) != '\0'){
		n--;
	}
	dst = '\0';
	return ret;
}

char *strchr(const char *s, int c)
{
    if(s == NULL)
    {
        return NULL;
    }

    while(*s != '\0')
    {
        if(*s == (char)c )
        {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

void* memmove(void* dst,const void* src,uint32_t count)
{
    //ASSERT(NULL !=src && NULL !=dst);
    char* tmpdst = (char*)dst;
    char* tmpsrc = (char*)src;

    if (tmpdst <= tmpsrc || tmpdst >= tmpsrc + count)
    {
        while(count--)
        {
            *tmpdst++ = *tmpsrc++; 
        }
    }
    else
    {
        tmpdst = tmpdst + count - 1;
        tmpsrc = tmpsrc + count - 1;
        while(count--)
        {
            *tmpdst-- = *tmpsrc--;
        }
    }

    return dst; 
}
