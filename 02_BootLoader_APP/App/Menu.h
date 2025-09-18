#ifndef __MENU_H
#define __MENU_H
#include "stm32f10x.h"

//选项类
struct option_class{
	char Name[16];												//显示字符串
	void (*func)(void);											//函数指针
};

void Main_Menu(void);											//主菜单

#endif
