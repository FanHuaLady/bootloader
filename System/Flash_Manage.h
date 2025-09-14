#ifndef __FLASH_MANAGE_H
#define __FLASH_MANAGE_H
#include "stm32f10x.h"

void Flower_Erase_Flash(uint16_t start,uint16_t num);
void Flower_Write_Flash(uint32_t addr,uint32_t *data,uint32_t num);

#endif
