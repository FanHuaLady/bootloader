#include "stm32f10x.h"
#include "OLED.h"
#include "Encoder.h"
#include "Key.h"
#include "Menu.h"
#include <string.h>

void Menu_Loop(struct option_class* option)
{
	int8_t encoder_num = 0;											// 获取编码器返回的值		1正转 2反转
	uint8_t key_num = 0;											// 返回按键返回的值			0未按 1按下
	
	int8_t Catch_i = 0;												// 实际上光标所在的选项索引
	int8_t Cursor_i = 0;											// 屏幕中光标所在的位置索引
	float Cursor_len_d0 = 0;										// 光标当前宽度
	float Cursor_len_d1 = 0;										// 光标目标宽度
	float Cursor_i_d0 = 0;  										// 光标当前y坐标
	float Cursor_i_d1 = 0;  										// 光标目标y坐标
	
	int8_t Show_i = 0; 												// 镜头所在的位置索引
	int8_t Show_i_temp = 0;											// 上次镜头所在的位置索引
	int8_t Show_d = 0;												// 镜头要移动的y长度
	
	int8_t Max = 0;													// 菜单有多少选项
	int8_t Speed = 8;												// 动画的速度

	while(option[Max].Name[0] != 'E') {Max++;}						// 计算菜单有多少选项
	while(1)
	{
		OLED_Clear();
		
		encoder_num = Encoder_Get();								// 获取编码器返回的值
		if(encoder_num == 1)									
		{
			encoder_num=0;											// 清空标志
			Cursor_i++;												// 屏幕中光标所在的位置增加
			Catch_i++;												// 实际上光标所在的位置增加
		}
		if(encoder_num == -1)									
		{
			encoder_num=0;											// 清空标志
			Cursor_i--;											
			Catch_i--;
		}
		
		if(Catch_i < 0) {Catch_i = 0;}								// 限制
		if(Catch_i > Max) {Catch_i = Max;}							// 限制
		if(Cursor_i < 0) {Cursor_i = 0;}							// 光标不能跑到屏幕外
		if(Cursor_i > 3) {Cursor_i = 3;}							// 光标不能跑到屏幕外
		if(Cursor_i > Max) {Cursor_i = Max;}						// 如果屏幕中的选项少于4，则需要此代码限制
		
		Show_i = Catch_i - Cursor_i;								// 计算镜头的位置
		
		if(Show_i - Show_i_temp != 0)								// 如果镜头的位置和上次不一样
		{
			Show_d = (Show_i - Show_i_temp) * 16;					// 计算镜头要移动的y坐标
			Show_i_temp = Show_i;									// 更新
		}
		
		if(Show_d != 0) 
		{
			Show_d /= 2;											// 每次只改变上一次的一半，这样比较丝滑
		}
		
		if(Show_d < 0) 												// 如果镜头是要向上移动，则需要显示一下第一个选项
		{
			OLED_ShowString(2, 0, option[Show_i - ((Show_d)/16)].Name, OLED_8X16);
		}
		
		for(int8_t i = 0; i < 5; i++)
		{			
			if(Show_i + i > Max ) {break;}							// 防止显示不存在的选项
			OLED_ShowString(2, (i* 16)+Show_d, option[Show_i + i].Name, OLED_8X16);
		}
		
		Cursor_i_d1 = Cursor_i * 16;								// 光标的目标y坐标 = 索引 * 16
		Cursor_len_d1 = strlen(option[Catch_i].Name) * 8 + 4;		// 光标的目标宽度 = 字符串长度 * 8 + 4
		
		if((Cursor_i_d1 - Cursor_i_d0) > 1) 						// 如果发现光标当前的y坐标不等于目标的y坐标，则小改变当前y坐标
		{
			Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed + 1;
		}
		else if((Cursor_i_d1 - Cursor_i_d0) < -1) 
		{
			Cursor_i_d0 += (Cursor_i_d1 - Cursor_i_d0) / Speed - 1;
		}
		if((Cursor_len_d1 - Cursor_len_d0) > 1) 					// 如果发现光标当前的宽度不等于目标的宽度，则小改变当前宽度
		{
			Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed + 1;
		}
		else if((Cursor_len_d1 - Cursor_len_d0) < -1) 
		{
			Cursor_len_d0 += (Cursor_len_d1 - Cursor_len_d0) / Speed - 1;
		}
		
		key_num = Key_GetNum();										// 如果按键被按下
		if(key_num == 1)										
		{
			key_num=0;
			if(option[Catch_i].func) {option[Catch_i].func();}		// 调用这个函数
			
			if(Catch_i == Max)										// 如果光标在最后一个位置，且按钮被点击
			{
				Catch_i=0;
				break;												// 退出菜单
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
