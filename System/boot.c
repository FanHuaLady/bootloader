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
    if (BootLoader_Enter(20) == 0)
    {
        if(OTA_Info.OTA_Flag == OTA_SET_FLAG)                                       // 有新固件
        {
            printf("Jump to BootLoader...\r\n");
            BootStartFlag |= UPDATA_A_FLAG;                                         // 设置更新标志
            UpDataA.W25Q64_BlockID = 0;                                             // 默认更新第0块
        }
        else
        {
            printf("Jump to UserApp...\r\n");
            LOAD_A(STM32_A_SADDR);                                                  // 跳转到用户程序
        }
    }
    printf("Jump to CommandLine...\r\n");
    BootLoader_Info();
}

// __asm：关键字，告诉编译器这是一段内嵌汇编代码
// MSR：将寄存器的值写入到特殊寄存器中(一条汇编指令)
// MSP：主堆栈指针寄存器
// r0：函数的第一个参数默认通过通用寄存器r0传递
// BX：跳转指令
// r14：链接寄存器，存放函数调用的返回地址

// 将寄存器 r0 中存放的值（也就是函数传入的参数 addr），直接写入到 CPU 的 MSP（主栈指针）寄存器中
// BX r14 指令用于从当前函数返回到调用该函数的地方(没有的话，就执行不了下一条指令了)
__asm void MSR_SP(uint32_t addr)
{
    MSR MSP, r0                                                                     
    BX r14
}

void LOAD_A(uint32_t addr)
{   
    // 栈顶地址必须是 “有效 SRAM 地址”
    // 0x20000000 ~ 0x20004FFF (STM32F103C8T6的SRAM空间范围)

    // 这里的 *(uint32_t *)addr 是一种类型转换和解引用的结合使用
    // C 语言中 “直接访问指定内存地址处的数据” 的标准写法

    if (*(uint32_t *)addr >= 0x20000000 && (*(uint32_t *)addr <= 0x20004FFF))
    {
        MSR_SP(*(uint32_t *)addr);
        load_A = (load_a)*(uint32_t *)(addr + 4);
        load_A();
    }
    else
    {
        printf("Jump to Partition A failed\r");
    }
}

void BootLoader_Clear(void)
{
    USART_DeInit(USART1);
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
}

uint8_t BootLoader_Enter(uint8_t timeout)
{
    printf("Enter the lowercase letter 'w' within %dms to enter BootLoader\r\n",timeout * 100);
    while (timeout--)
    {
        Delay_ms(100);
        if(U0_RxBuff[0] == 'w')
        {
            return 1;
        }
    }
    return 0;
}

void BootLoader_Info(void)
{
    printf("\r\n");
    printf("[1]Erase Area A\r\n");
    printf("[2]UART IAP Download Program to Area A\r\n");
    printf("[3]Set OTA Version Number\r\n");
    printf("[4]Query OTA Version Number\r\n");
    printf("[5]Download Program to External Flash\r\n");
    printf("[6]Use Program in External Flash\r\n");
    printf("[7]Reboot\r\n");
}

void BootLoader_Event(uint8_t *data, uint16_t len)
{
    if (BootStartFlag == 0)
    {
        if(len == 1 && data[0] == '1')
        {
            printf("Erase Area A\r\n");
            Boot_Erase_Flash(STM32_A_START_PAGE,STM32_A_PAGE_NUM);
        }
        else if(len == 1 && data[0] == '2')
        {
            printf("Please use a bin format file for UART IAP\r\n");
            Boot_Erase_Flash(STM32_A_START_PAGE,STM32_A_PAGE_NUM);      // 擦除A区
            BootStartFlag |= IAP_XMODEMC_FLAG;
            UpDataA.XmodemTimer = 0;
        }
        else if(len == 1 && data[0] == '7')
        {
            printf("Reboot\r\n");
            Delay_ms(100);
            NVIC_SystemReset();
        }
    }
}

uint16_t Xmodem_CRC16(uint8_t *data, uint16_t len)
{
    uint16_t Crcinit = 0x0000;
    uint16_t Crcipoly = 0x1021;

    while(len--)
    {
        Crcinit = (*data << 8) ^ Crcinit;
        for(uint8_t i = 0; i < 8; i++)
        {
            if (Crcinit & 0x8000)
            {
                Crcinit = (Crcinit << 1) ^ Crcipoly;
            }
            else
            {
                Crcinit = (Crcinit << 1);
            }
        }
        data++;
    }
    return Crcinit;
}
