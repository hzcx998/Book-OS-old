#ifndef _GUI_BASIC_FONT_H
#define _GUI_BASIC_FONT_H

#include <stdint.h>


#define MAX_FONT_NR 10

#define FONT_NAME_LEN 32
#define COPYRIGHT_NAME_LEN 64


struct font_info
{
	char *name;	/*name of font*/
	char *copyright;	/*copyright of font*/
	uint8 *addr;					/*bitmap addr of font*/
	int width, height;			/*char size*/
};

extern struct font_info *current_font;

void init_font();
int registe_font(struct font_info *font_info);
struct font_info *get_font_info(char *name);
int logout_font(struct font_info *font_info);

void registe_standard_font();
int select_font(char *name);
void registe_simsun_font();

/*font*/
extern uint8 standard_font[];
extern uint8 simsun_font[];
#endif //_GUI_BASIC_FONT_H
