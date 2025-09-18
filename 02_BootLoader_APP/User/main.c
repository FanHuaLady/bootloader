#include "stm32f10x.h"
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "Encoder.h"
#include "Key.h"
#include "Menu.h"

int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x5000);
	LED_Init();
	OLED_Init();
	Encoder_Init();
	Key_Init();
	
	Main_Menu();
	while (1)
	{
		
	}
}
