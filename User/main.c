#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"

#include "Serial_Page.h"

#include <stdio.h>

int main(void)
{
	OLED_Init();
	Serial_Init();
	Serial_Page();
	while(1)
	{
		
	}
}
