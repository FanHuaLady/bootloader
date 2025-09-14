#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"
#include "Flash_Manage.h"

#include "Serial_Page.h"

#include <stdio.h>

uint32_t wbuff[1024];

int main(void)
{
	OLED_Init();
	Serial_Init();
	// Serial_Page();
	AT24C02_Init();
	W25Q64_Init();

	while(1)
	{
		
	}
}
