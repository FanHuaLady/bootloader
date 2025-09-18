#ifndef __W25Q64_H
#define __W25Q64_H
#include "stm32f10x.h"

void W25Q64_Init(void);
void W25Q64_WaitBusy(void);
void W25Q64_WriteEnable(void);
void W25Q64_Erase64K(uint8_t block_id);
void W25Q64_WritePage(uint8_t *buff, uint16_t page_id);
void W25Q64_Read(uint8_t *buff, uint32_t addr, uint32_t len);

#endif
