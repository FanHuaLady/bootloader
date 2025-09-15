#ifndef __MAIN_H
#define __MAIN_H
#include "stm32f10x.h"

// STM32F103C8T6 		64KB
// 单扇区				1KB
// B区					20K		0~19
// A区					44K		20~63

#define STM32_FLASH_SADDR		0x08000000                                                  // FLASH起始地址
#define STM32_PAGE_SIZE			1024                                                        // FLASH单页大小
#define STM32_PAGE_NUM			64                                                          // FLASH总页数
#define STM32_B_PAGE_NUM		20                                                          // B区页数
#define STM32_A_PAGE_NUM		STM32_PAGE_NUM - STM32_B_PAGE_NUM                           // A区页数
#define STM32_A_SPAGE_PAGE      STM32_B_PAGE_NUM                                            // A区起始页编号
#define STM32_A_SADDR           STM32_FLASH_SADDR + STM32_A_SPAGE_PAGE * STM32_PAGE_SIZE    // A区起始地址

#define OTA_SET_FLAG            0xAABB1122

typedef struct{
    uint32_t OTA_Flag;
    uint32_t Firelen[11];                               // 0号成员固定对应OTA的大小
}OTA_InfoCB;
#define OTA_INFOCB_SIZE      sizeof(OTA_InfoCB)

typedef struct{
    uint8_t UpDataBuff[STM32_PAGE_SIZE];
    uint32_t W25Q64_BlockID;
}UpDataA_CB;

extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;

#endif
