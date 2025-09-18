#include "boot.h"
#include "main.h"
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "At24c02.h"
#include "W25Q64.h"
#include "Flash_Manage.h"

#include <stdio.h>
#include <string.h>

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
		BootLoader_Clear();
		
        MSR_SP(*(uint32_t *)addr);
		
		/*
		uint32_t offset = addr - STM32_FLASH_SADDR;
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, offset);
		*/
		
        load_A = (load_a)*(uint32_t *)(addr + 4);
        load_A();
    }
    else
    {
        printf("Jump to Partition A failed\r");
		BootLoader_Clear();
    }
}

void BootLoader_Clear(void)
{
    // 1. 反初始化USART1相关
    USART_DeInit(USART1);
    NVIC_DisableIRQ(USART1_IRQn);  // 禁用USART1中断
    
    // 2. 反初始化SPI1（W25Q64使用）
    SPI_I2S_DeInit(SPI1);
    
    // 3. 反初始化DMA1通道5（USART1接收用）
    DMA_DeInit(DMA1_Channel5);
    
    // 4. 反初始化GPIO
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    
    // 5. 关闭外设时钟，避免持续占用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_SPI1 | 
                          RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, DISABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
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
    if (BootStartFlag == 0 && DebugFlag == 0)                           // 未开始OTA
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
            BootStartFlag |= (IAP_XMODEMC_FLAG | IAP_XMODEMD_FLAG);     // 开始发送C + 
            UpDataA.XmodemTimer = 0;
            UpDataA.XmodemID = 0;
        }
		else if(len == 1 && data[0] == '3')
        {
            printf("Debug\r\n");
			DebugFlag = 1;
            
        }
        else if(len == 1 && data[0] == '7')
        {
            printf("Reboot\r\n");
            Delay_ms(100);
            NVIC_SystemReset();
        }
    }

    if (BootStartFlag & IAP_XMODEMD_FLAG)
    {
        if(len == 133 && data[0] == 0x01)                               // 接收到长度为133的包 且首字节为0x01
        {
            BootStartFlag &=~ IAP_XMODEMC_FLAG;                         // 停止发送C
            UpDataA.XmodemCRC = Xmodem_CRC16(&data[3],128);             // 对数据包有效的部分进行校验
            if (UpDataA.XmodemCRC == data[131] * 256 + data[132])		// CRC校验通过
            {
                UpDataA.XmodemID++;										// 进行下一包
                memcpy(&UpDataA.UpDataBuff[((UpDataA.XmodemID - 1) % (STM32_PAGE_SIZE / 128)) * 128],&data[3],128);
                if ((UpDataA.XmodemID % (STM32_PAGE_SIZE / 128)) == 0)  // 发现成功凑了8个包
                {
                    Boot_Write_Flash(STM32_A_SADDR + ((UpDataA.XmodemID / (STM32_PAGE_SIZE / 128)) - 1) * STM32_PAGE_SIZE,
                                    (uint32_t *)UpDataA.UpDataBuff, STM32_PAGE_SIZE);
                }
                printf("\x06");											// 能连续接包,说明校验通过了
            }
            else
            {
                printf("\x15");                                         // 表示校验未通过
            }
        }
        if (len == 1 && data[0] == 0x04)
        {
            printf("\x06");
            if ((UpDataA.XmodemID % (STM32_PAGE_SIZE / 128)) != 0)
            {
                Boot_Write_Flash(STM32_A_SADDR + ((UpDataA.XmodemID / (STM32_PAGE_SIZE / 128))) * STM32_PAGE_SIZE,
                                (uint32_t *)UpDataA.UpDataBuff, 
                                (UpDataA.XmodemID % (STM32_PAGE_SIZE / 128)) * 128);
            }
            BootStartFlag &=~ IAP_XMODEMD_FLAG;                         // 不用发数据了
            Delay_ms(100);
            NVIC_SystemReset();
        }
	}
	
	if (DebugFlag == 1)
	{
		PrintReceivedData(data, len);
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

void PrintReceivedData(uint8_t *data, uint16_t len)
{
    printf("Received %d bytes:\r\n", len);
    printf("Offset    Hex Data                              ASCII\r\n");
    printf("--------  ------------------------------------  ----------------\r\n");
    
    for(uint16_t i = 0; i < len; i++)
    {
        // 每行显示16个字节
        if(i % 16 == 0)
        {
            // 如果不是第一行，先打印上一行的ASCII表示
            if(i != 0)
            {
                printf("  ");
                for(uint16_t j = i - 16; j < i; j++)
                {
                    if(data[j] >= 32 && data[j] <= 126) // 可打印字符
                        printf("%c", data[j]);
                    else
                        printf(".");
                }
                printf("\r\n");
            }
            
            // 打印新行的偏移量
            printf("%08X  ", i);
        }
        
        // 打印十六进制值
        printf("%02X ", data[i]);
        
        // 在8字节后添加额外空格，提高可读性
        if(i % 8 == 7)
            printf(" ");
    }
    
    // 处理最后一行可能不满16字节的情况
    uint16_t remaining = len % 16;
    if(remaining != 0)
    {
        // 填充空格使ASCII列对齐
        for(uint16_t i = remaining; i < 16; i++)
        {
            printf("   ");
            if(i % 8 == 7)
                printf(" ");
        }
        
        printf("  ");
        for(uint16_t i = len - remaining; i < len; i++)
        {
            if(data[i] >= 32 && data[i] <= 126) // 可打印字符
                printf("%c", data[i]);
            else
                printf(".");
        }
    }
    else
    {
        // 处理完整的最后一行
        printf("  ");
        for(uint16_t i = len - 16; i < len; i++)
        {
            if(data[i] >= 32 && data[i] <= 126) // 可打印字符
                printf("%c", data[i]);
            else
                printf(".");
        }
    }
    
    printf("\r\n--------  ------------------------------------  ----------------\r\n");
}
