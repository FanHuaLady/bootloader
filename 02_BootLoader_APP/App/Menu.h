#ifndef __MENU_H
#define __MENU_H
#include "stm32f10x.h"

//ѡ����
struct option_class{
	char Name[16];												//��ʾ�ַ���
	void (*func)(void);											//����ָ��
};

void Main_Menu(void);											//���˵�

#endif
