/*
File:		gui/font.c
Contains:	font management
Auther:		Hu Zicheng
Time:		2019/2/13
Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
E-mail:		2323168280@qq.com
*/
#include <sys/config.h>

#ifdef _CONFIG_GUI_BASIC_


#include <string.h>
#include <sys/mm.h>

#include <sys/gui.h>

struct font_info font_lib[MAX_FONT_NR];

struct font_info *current_font;

void init_font()
{
	int i;
	for(i = 0; i < MAX_FONT_NR; i++){
		font_lib[i].addr = NULL;
		font_lib[i].name = NULL;
		font_lib[i].copyright = NULL;
		font_lib[i].width = font_lib[i].height = 0;
	}
	current_font = NULL;
	
	registe_standard_font();
	registe_simsun_font();
	select_font("Standard Font");
	
}

int registe_font(struct font_info *font_info)
{
	int i;
	for(i = 0; i < MAX_FONT_NR; i++){
		if(font_lib[i].addr == NULL){
			font_lib[i] = *font_info;
			/*registe sucess*/
			return 0;
		}
	}
	/*registe failed*/
	return -1;
}

struct font_info *get_font_info(char *name)
{
	int i;
	for(i = 0; i < MAX_FONT_NR; i++){
		if(!strcmp(font_lib[i].name, name)){
			
			return &font_lib[i];
		}
	}
	return NULL;
}

int logout_font(struct font_info *font_info)
{
	int i;
	for(i = 0; i < MAX_FONT_NR; i++){
		if(&font_lib[i] == font_info){
			
			mm.free(font_lib[i].name);
			mm.free(font_lib[i].copyright);
			
			font_lib[i].width = 0;
			font_lib[i].height = 0;
			font_lib[i].addr = NULL;
			font_lib[i].name = NULL;
			font_lib[i].copyright = NULL;
			
			/*logout sucess*/
			return 0;
		}
	}
	/*logout failed*/
	return -1;
}

int select_font(char *name)
{
	int i;
	for(i = 0; i < MAX_FONT_NR; i++){
		if(!strcmp(font_lib[i].name, name)){
			current_font = &font_lib[i];
			return 0;
		}
	}
	current_font = NULL;
	/*registe failed*/
	return -1;
}


#endif //_CONFIG_GUI_BASIC_
