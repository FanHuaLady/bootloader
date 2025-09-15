#ifndef __FLASH_MANAGE_H
#define __FLASH_MANAGE_H
#include "stm32f10x.h"

void Boot_Erase_Flash(uint16_t start,uint16_t num);
void Boot_Write_Flash(uint32_t addr,uint32_t *data,uint32_t num);

#endif
