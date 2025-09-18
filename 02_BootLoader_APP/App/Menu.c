#include "stm32f10x.h"
#include "OLED.h"
#include "Encoder.h"
#include "Key.h"
#include "Menu.h"
#include <string.h>

void Menu_Loop(struct option_class* option)
{
	int8_t encoder_num = 0;											// ��ȡ���������ص�ֵ		1��ת 2��ת
	uint8_t key_num = 0;											// ���ذ������ص�ֵ			0δ�� 1����
	
	int8_t Catch_i = 0;												// ʵ���Ϲ�����ڵ�ѡ������
	int8_t Cursor_i = 0;											// ��Ļ�й�����ڵ�λ������
	float Cursor_len_d0 = 0;										// ��굱ǰ���
	float Cursor_len_d1 = 0;										// ���Ŀ����
	float Cursor_i_d0 = 0;  										// ��굱ǰy����
	float Cursor_i_d1 = 0;  										// ���Ŀ��y����
	
	int8_t Show_i = 0; 												// ��ͷ���ڵ�λ������
	int8_t Show_i_temp = 0;											// �ϴξ�ͷ���ڵ�λ������
	int8_t Show_d = 0;												// ��ͷҪ�ƶ���y����
	
	int8_t Max = 0;													// �˵��ж���ѡ��
	int8_t Speed = 8;												// �������ٶ�

	while(option[Max].Name[0] != 'E') {Max++;}						// ����˵��ж���ѡ��
	while(1)
	{
		OLED_Clear();
		
		encoder_num = Encoder_Get();								// ��ȡ���������ص�ֵ
		if(encoder_num == 1)									
		{
			encoder_num=0;											// ��ձ�־
			Cursor_i++;												// ��Ļ�й�����ڵ�λ������
			Catch_i++;												// ʵ���Ϲ�����ڵ�λ������
		}
		if(encoder_num == -1)									
		{
			encoder_num=0;											// ��ձ�־
			Cursor_i--;											
			Catch_i--;
		}
		
		if(Catch_i < 0) {Catch_i = 0;}								// ����
		if(Catch_i > Max) {Catch_i = Max;}							// ����
		if(Cursor_i < 0) {Cursor_i = 0;}							// ��겻���ܵ���Ļ��
		if(Cursor_i > 3) {Cursor_i = 3;}							// ��겻���ܵ���Ļ��
		if(Cursor_i > Max) {Cursor_i = Max;}						// �����Ļ�е�ѡ������4������Ҫ�˴�������
		
		Show_i = Catch_i - Cursor_i;								// ���㾵ͷ��λ��
		
		if(Show_i - Show_i_temp != 0)								// �����ͷ��λ�ú��ϴβ�һ��
		{
			Show_d = (Show_i - Show_i_temp) * 16;					// ���㾵ͷҪ�ƶ���y����
			Show_i_temp = Show_i;									// ����
		}
		
		if(Show_d != 0) 
		{
			Show_d /= 2;											// ÿ��ֻ�ı���һ�ε�һ�룬�����Ƚ�˿��
		}
		
		if(Show_d < 0) 												// �����ͷ��Ҫ�����ƶ�������Ҫ��ʾһ�µ�һ��ѡ��
		{
			OLED_ShowString(2, 0, option[Show_i - ((Show_d)/16)].Name, OLED_8X16);
		}
		
		for(int8_t i = 0; i < 5; i++)
		{			
			if(Show_i + i > Max ) {break;}							// ��ֹ��ʾ�����ڵ�ѡ��
			OLED_ShowString(2, (i* 16)+Show_d, option[Show_i + i].Name, OLED_8X16);
		}
		
		Cursor_i_d1 = Cursor_i * 16;								// ����Ŀ��y���� = ���� * 16
		Cursor_len_d1 = strlen(option[Catch_i].Name) * 8 + 4;		// ����Ŀ���� = �ַ������� * 8 + 4
		
		if((Cursor_i_d1 - Cursor_i_d0) > 1) 						// ������ֹ�굱ǰ��y���겻����Ŀ���y���꣬��С�ı䵱ǰy����
		{
			Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed + 1;
		}
		else if((Cursor_i_d1 - Cursor_i_d0) < -1) 
		{
			Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed - 1;
		}
		if((Cursor_len_d1 - Cursor_len_d0) > 1) 					// ������ֹ�굱ǰ�Ŀ�Ȳ�����Ŀ��Ŀ�ȣ���С�ı䵱ǰ���
		{
			Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed + 1;
		}
		else if((Cursor_len_d1 - Cursor_len_d0) < -1) 
		{
			Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed - 1;
		}
		
		key_num = Key_GetNum();										// �������������
		if(key_num == 1)										
		{
			key_num=0;
			if(option[Catch_i].func) {option[Catch_i].func();}		// �����������
			
			if(Catch_i == Max)										// �����������һ��λ�ã��Ұ�ť�����
			{
				Catch_i=0;
				break;												// �˳��˵�
			}
			
		}
		
		OLED_ReverseArea(0, Cursor_i_d0, Cursor_len_d0, 16);
		OLED_Update();
	}
}

void Game(void)
{
	struct option_class option[] = {
		{"Break_Bricks"},
		{"Dinosaur_Running"},
		{"CarEvader"},
		{"FlowerWorld"},
		{"Exti"}
	};
	
	Menu_Loop(option);
}

void Main_Menu(void)
{
	struct option_class option[]=
	{
		{"Time"},
		{"Set"},
		{"Game",Game},
		{"User"},
		{"Gyroscope"},
		{"Sleep"},
		{"Calculator"},
		{"Exti"}
	};
	Menu_Loop(option);
}
