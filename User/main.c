#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"

#include "Serial_Page.h"

#include <stdio.h>

uint8_t wbuf[256];
uint8_t rbuf[256];

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
