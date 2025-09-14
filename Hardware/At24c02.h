#ifndef __AT24C02_H
#define __AT24C02_H
#include "stm32f10x.h"

#define AT24C02_WADDR 0xA0                          // AT24C02的写地址，最低位为0
#define AT24C02_RADDR 0xA1                          // AT24C02的读地址，最低位为1

void AT24C02_Init(void);
uint8_t AT24C02_WriteByte(uint8_t addr, uint8_t data);
uint8_t AT24C02_WritePage(uint8_t addr, uint8_t *data);
uint8_t AT24C02_Read(uint8_t addr, uint8_t *data, uint16_t len);

void AT24C02_Read_OtaFlag(void);                    // 读取OTA标志

#endif
