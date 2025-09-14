#include "main.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"
#include "Flash_Manage.h"

#include "boot.h"

#include "Serial_Page.h"

#include <stdio.h>

OTA_InfoCB OTA_Info;

int main(void)
{
	OLED_Init();
	Serial_Init();
	AT24C02_Init();
	W25Q64_Init();

	AT24C02_Read_OtaFlag();    // 读取OTA标志
	BootLoader_Branch();

	while(1)
	{

	}
}
