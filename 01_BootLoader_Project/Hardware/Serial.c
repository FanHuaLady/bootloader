#include "stm32f10x.h"
#include "Serial.h"
#include <stdio.h>

UCB_CB U0CB;                                                                // 管理变量
uint8_t U0_RxBuff[U0_RX_SIZE];                                              // 数据缓冲区2048

uint32_t last_dma_pos = 0;

void U0Rx_PtrInit(void)
{
    U0CB.URxDataIN = &U0CB.URxDataPtr[0];									// 指向第一个数据包
    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];									// 指向第一个数据包
    U0CB.URxDataEND = &U0CB.URxDataPtr[NUM-1];
    U0CB.URxCounter = 0;
    U0CB.nextPacketIndex = 0;
    
    // 初始化所有数据包为无效状态
    for(int i = 0; i < NUM; i++)
    {
        U0CB.URxDataPtr[i].start = NULL;
        U0CB.URxDataPtr[i].end = NULL;
        U0CB.packetValid[i] = 0;
    }
    
    // 设置第一个数据包的起始位置
    U0CB.URxDataIN->start = U0_RxBuff;
}

void MarkPacketProcessed(uint8_t index)
{
    if(index < NUM)
    {
        U0CB.packetValid[index] = 0;
        
        // 如果这是当前最早的数据包，移动URxDataOUT指针
        if(index == (U0CB.URxDataOUT - U0CB.URxDataPtr))
        {
            U0CB.URxDataOUT++;
            if(U0CB.URxDataOUT > U0CB.URxDataEND)
                U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
        }
    }
}

void Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  								// USART1 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 								// USART1 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = 921600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    DMA_DeInit(DMA1_Channel5);  											// USART1_RX使用DMA1通道5
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)U0_RxBuff;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = U0_RX_SIZE;  							// 整个缓冲区大小2048
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;  							// 循环模式
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);
    
    last_dma_pos = 0;

    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_Cmd(USART1, ENABLE);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    U0Rx_PtrInit();
}

// 重写fputc函数，实现printf到串口的重定向
int fputc(int ch, FILE *f) 
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

// 串口空闲中断
// 在全局变量中添加

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        volatile uint16_t temp = USART1->SR;
        temp = USART1->DR;
        
        DMA_Cmd(DMA1_Channel5, DISABLE);
		// 若缓冲区 存在 10字节 		则remaining = 2038字节
		// 若缓冲区 存在 2048字节	则remaining = 0字节
		// 若缓冲区 存在 2058字节 	则remaining = 2038字节
        uint16_t remaining = DMA_GetCurrDataCounter(DMA1_Channel5);			// remaining表示 【缓冲区还能写多少字节】
        DMA_Cmd(DMA1_Channel5, ENABLE);
		
		// current_dma_pos表示 【下一个即将被 DMA 写入数据的空位置的索引】
		// last_dma_pos表示【上一次即将被 DMA 写入数据的空位置的索引】
		
		// 若缓冲区 存在 10字节		则(2048 - 2038) % 2048 	= 10
		// 若缓冲区 存在 2048字节	则(2048 - 0) % 2048 	= 0	 ---------->// 有这个 % U0_RX_SIZE可以让这种情况回到开头
		// 若缓冲区 存在 2058字节	则(2048 - 2038) % 2048 	= 10
        uint32_t current_dma_pos = (U0_RX_SIZE - remaining) % U0_RX_SIZE;	
        
        uint16_t received;													// received表示 【此次接收多少字节】
		// ---------------------------------------------------
		// 如果 current_dma_pos = 5 并且 last_dma_pos = 3
		// last_dma_pos		[1] [1] [1] [0] [0] [0] [0] ...
		//              				 ↑
		// current_dma_pos	[1] [1] [1] [1] [1] [0] [0] ...
		//                      				 ↑
		// 5 - 3 = 2
		// 此次接收2个字节
		// ---------------------------------------------------
		// 如果 current_dma_pos = 2 并且 last_dma_pos = 2046
		// last_dma_pos		... [1] [1] [1] [1] [0] [0]
		//              						 ↑
		// current_dma_pos	[2] [2] [1] [1] [1] [1] [1] ...
		//          				 ↑
		// 2048 - 2046 + 2 = 4
		// 此次接收4个字节
		// ---------------------------------------------------
        if(current_dma_pos >= last_dma_pos)									// 如果这次DMA位置 > 上次DMA位置 说明没有回卷
		{
            received = current_dma_pos - last_dma_pos;						// 接收数据的长度 = 这次DMA位置 - 上次DMA位置
		}
        else																// 发生了回滚
		{
            received = U0_RX_SIZE - last_dma_pos + current_dma_pos;			// 
        }
		
        if(received == 0)													// 说明没有接收到数据包
        {
            last_dma_pos = current_dma_pos;
            return;															// 函数退出 说明下面的代码接收到数据包了
        }
        
		// -----------------------------------------------------------------// 成功接收数据包 ---> 进行数据标记
		
		// U0CB.URxDataPtr表示	【数据包结构体数组】
		// U0CB.URxDataPtr指向 	【数据包结构体数组的第一个结构体】
		// U0CB.URxDataIN指向 	【下一个即将用来存储新接收数据包的结构体】
		// currentIndex表示 	【当前正在存储新接收数据包的结构体在数组中的索引】
		
		// U0CB.URxDataIN指向下一个即将用来存储新接收数据包的结构体
		// 是上一次中断运行或者初始化时标记的
		
        uint8_t currentIndex = U0CB.URxDataIN - U0CB.URxDataPtr;			// currentIndex表示 【当前正在存储新接收数据包的结构体在数组中的索引】
        U0CB.packetValid[currentIndex] = 1;									// 数据包被标记为已经使用
        
		// last_dma_pos表示【上一次即将被 DMA 写入数据的空位置的索引】
		// 即这次来的数据的第一个字节在缓冲区数组的索引						// 】*****【
        U0CB.URxDataIN->start = &U0_RxBuff[last_dma_pos];					// U0CB.URxDataIN->start表示 【此次数据包标记的数据的第一个字节】
        
        uint32_t end_pos = (last_dma_pos + received - 1) % U0_RX_SIZE;		// 
        U0CB.URxDataIN->end = &U0_RxBuff[end_pos];							// U0CB.URxDataIN->start表示 【此次数据包标记的数据的最后一个字节】
		
		// -----------------------------------------------------------------// 进行其他操作
		
        // 更新全局计数器
        U0CB.URxCounter = (U0CB.URxCounter + received) % U0_RX_SIZE;
        
        // 检查下一个数据包位置是否已经被使用
        uint8_t nextIndex = (currentIndex + 1) % NUM;
        if(U0CB.packetValid[nextIndex])
        {
            U0CB.packetValid[nextIndex] = 0;
            
            if(nextIndex == (U0CB.URxDataOUT - U0CB.URxDataPtr))
            {
                U0CB.URxDataOUT++;
                if(U0CB.URxDataOUT > U0CB.URxDataEND)
                    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
            }
        }
        
        U0CB.URxDataIN++;													// 标记下一个数据包 下次使用它
        if(U0CB.URxDataIN > U0CB.URxDataEND)
            U0CB.URxDataIN = &U0CB.URxDataPtr[0];
        
        U0CB.nextPacketIndex = U0CB.URxDataIN - U0CB.URxDataPtr;
        
        last_dma_pos = current_dma_pos;
        
        DMA_ClearFlag(DMA1_FLAG_TC5);
    }
}
