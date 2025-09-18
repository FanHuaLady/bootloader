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
#define STM32_A_START_PAGE      STM32_B_PAGE_NUM                                            // A区起始页编号
#define STM32_A_SADDR           STM32_FLASH_SADDR + STM32_A_START_PAGE * STM32_PAGE_SIZE    // A区起始地址

#define UPDATA_A_FLAG           0x00000001                                                  // B区更新A区标志
#define IAP_XMODEMC_FLAG        0x00000002                                                  // 发送C标志位
#define IAP_XMODEMD_FLAG        0x00000004                                                  // 

#define OTA_SET_FLAG            0xAABB1122                                                  // 有新固件标志

typedef struct{
    uint32_t OTA_Flag;                                  // 标志位，0xAABB1122表示有新固件
    uint32_t Firelen[11];                               // 固件长度，单位字节
}OTA_InfoCB;
#define OTA_INFOCB_SIZE      sizeof(OTA_InfoCB)

// B区更新A区的方案
// W25Q64，每一个块存1个应用程序
// 每次从W25Q64读取1024字节，写入A区
// 再次读取1024字节，写入A区
// 直到写完为止
typedef struct{
    uint8_t UpDataBuff[STM32_PAGE_SIZE];                // 用于存放从W25Q64读取的数据       // 一次更新1024字节
    uint32_t W25Q64_BlockID;                            // W25Q64块编号，0~10
    uint32_t XmodemTimer;                               // 延时
    uint32_t XmodemID;                                  // 保存当前接收了几个包
    uint32_t XmodemCRC;                                 // CRC校验
}UpDataA_CB;                                            // 用于存放更新数据的结构体

extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;
extern uint32_t BootStartFlag;

extern uint8_t DebugFlag;

#endif
