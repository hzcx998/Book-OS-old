#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>
#include <stdint.h>

char * itoa(char * str, int num);
int atoi(const char *src);

void *memset(void* src, uint8_t value, uint32_t size);
void memcpy(void* dst_, const void* src_, uint32_t size);
char* strcpy(char* dst_, const char* src_);
uint32_t strlen(const char* str);
int8_t strcmp (const char *a, const char *b); 
int memcmp(const void * s1, const void *s2, int n);
char* strrchr(char* str, int c);
char* strcat(char* strDest , const char* strSrc);
int strncmp (const char * s1, const char * s2, int n);
int strpos(char *str, char ch);
char* strncpy(char* dst_, char* src_, int n) ;
char *strncat(char *dst, const char *src, int n);

void *memset16(void* src, uint16 value, uint32_t size);

#endif

