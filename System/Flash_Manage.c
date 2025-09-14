#include "Flash_Manage.h"

void Flower_Erase_Flash(uint16_t start,uint16_t num)
{
    FLASH_Unlock();
    for (uint16_t i = 0; i < num; i++)
    {
        FLASH_ErasePage(0x08000000 + (start + i) * 1024);
    }
    FLASH_Lock();
}

void Flower_Write_Flash(uint32_t addr,uint32_t *data,uint32_t num)
{
    FLASH_Unlock();
    while (num)
    {
        FLASH_ProgramWord(addr, *data);
        addr += 4;
        data++;
        num -= 4;
    }
    FLASH_Lock();
}
