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
UpDataA_CB UpDataA;

int main(void)
{
	OLED_Init();
	Serial_Init();
	AT24C02_Init();
	W25Q64_Init();

	OTA_Info.OTA_Flag = 0xAABB1122;
	for(uint8_t i=0; i<11; i++)
	{
		OTA_Info.Firelen[i] = 0x55;
	}

	AT24C02_WriteOTAInfo();
	AT24C02_Read_OtaFlag();

	printf("%x\r\n", OTA_Info.OTA_Flag);

	for(uint8_t i=0; i<11; i++)
	{
		printf("%x\r\n", OTA_Info.Firelen[i]);
	}

	BootLoader_Branch();

	while(1)
	{

	}
}
