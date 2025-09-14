#include "boot.h"
#include "main.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"
#include "Flash_Manage.h"

#include <stdio.h>

load_a load_A;

void BootLoader_Branch(void)
{
    if(OTA_Info.OTA_Flag == OTA_SET_FLAG)
    {
        printf("Jump to BootLoader...\r\n");
    }
    else
    {
        printf("Jump to UserApp...\r\n");
        LOAD_A(STM32_A_SADDR);
    }
}

__asm void MSR_SP(uint32_t addr)
{
    MSR MSP, r0
    BX r14
}

void LOAD_A(uint32_t addr)
{
    if (*(uint32_t *)addr >= 0x20000000 && (*(uint32_t *)addr <= 0x20004FFF))
    {
        MSR_SP(*(uint32_t *)addr);
        load_A = (load_a)*(uint32_t *)(addr + 4);
        load_A();
    }
}

void BootLoader_Clear(void)
{
    USART_DeInit(USART1);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
}
