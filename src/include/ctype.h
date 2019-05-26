#ifndef _CTYPE_H_
#define _CTYPE_H_

int isspace(char c);
int isalnum(int ch);
int isxdigit (int c);
int isdigit( int ch );
unsigned long strtoul(const char *cp,char **endp,unsigned int base);
long strtol(const char *cp,char **endp,unsigned int base);
int isalpha(int ch);
double strtod(const char* s, char** endptr);
double atof(char *str);

#endif  /*_CTYPE_H_*/
