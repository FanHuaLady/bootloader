#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"

#include "Serial_Page.h"

#include <stdio.h>

uint8_t buf[256];

int main(void)
{
	OLED_Init();
	Serial_Init();
	// Serial_Page();
	AT24C02_Init();

	for(uint16_t i = 0; i < 256; i++)
	{
		AT24C02_WriteByte(i, 255-i);
		Delay_ms(5);
	}

	AT24C02_Read(0, buf, 256);

	for(uint16_t i = 0; i < 256; i++)
	{
		printf("%d = %x\r\n", i, buf[i]);
		Delay_ms(10);
	}

	while(1)
	{
		
	}
}
