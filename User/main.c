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
uint32_t BootStartFlag;

int main(void)
{
	uint32_t i;

	// 初始化外设
	OLED_Init();
	Serial_Init();
	AT24C02_Init();
	W25Q64_Init();

	// BootLoader操作
	AT24C02_Read_OtaFlag();													// 读取OTA标志
	BootLoader_Branch();													// 判断跳转还是更新

	while(1)
	{
		if(BootStartFlag & UPDATA_A_FLAG)									// 需要更新A区
		{
			printf("长度%d字节\r\n", OTA_Info.Firelen[UpDataA.W25Q64_BlockID]);
			// 因为Flash是按字(4字节)写入的，所以长度必须是4的整数倍
			if (OTA_Info.Firelen[UpDataA.W25Q64_BlockID] % 4 == 0)			// 长度是4的整数倍，说明数据是正确的
			{
				Boot_Erase_Flash(STM32_A_START_PAGE, STM32_A_PAGE_NUM);		// 擦除A区
				// 每有1024字节就循环1次
				for(i = 0; i < OTA_Info.Firelen[UpDataA.W25Q64_BlockID] / STM32_PAGE_SIZE; i++)	
				{
					W25Q64_Read(UpDataA.UpDataBuff, 						// 从W25Q64读取数据
						// 若使用W25Q64的第0块,i * 1024表示在这一块中，第一次地址是0，第二次是1024，第三次是2048
						// 若使用W25Q64的第1块,需要加上UpDataA.W25Q64_BlockID * 64 * 1024让地址偏移
						i * 1024 + UpDataA.W25Q64_BlockID * 64 * 1024, 		// 计算读取地址
						STM32_PAGE_SIZE);									// 一次读1024字节
					Boot_Write_Flash(STM32_A_SADDR + i * STM32_PAGE_SIZE,	// 写入A区
						 (uint32_t *)UpDataA.UpDataBuff,					// 按照函数要求需要强转
						  STM32_PAGE_SIZE);									// 写入1024字节
				}
				if(OTA_Info.Firelen[UpDataA.W25Q64_BlockID] % 1024 != 0)	// 还有剩余的
				{
					W25Q64_Read(UpDataA.UpDataBuff, 						// 从W25Q64读取数据
						i * 1024 + UpDataA.W25Q64_BlockID * 64 * 1024, 		// 计算读取地址
						OTA_Info.Firelen[UpDataA.W25Q64_BlockID] % 1024);	// 写入剩余的量
					Boot_Write_Flash(STM32_A_SADDR + i * STM32_PAGE_SIZE, 	// 写入A区
						(uint32_t *)UpDataA.UpDataBuff, 					// 按照函数要求需要强转
						OTA_Info.Firelen[UpDataA.W25Q64_BlockID] % 1024);	// 写入剩余部分
				}
				if (UpDataA.W25Q64_BlockID == 0)
				{
					OTA_Info.OTA_Flag = 0;									// 清除更新标志位
					AT24C02_WriteOTAInfo();									// 更新AT24C02中的标志
				}
				NVIC_SystemReset();											// 
			}
			else
			{
				printf("长度不是4的整数倍\r\n");
				BootStartFlag &= ~UPDATA_A_FLAG;
			}
		}
	}
}
