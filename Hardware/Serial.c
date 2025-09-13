#include "stm32f10x.h"
#include "Serial.h"
#include <stdio.h>

UCB_CB U0CB;                                                                // 管理变量
uint8_t U0_RxBuff[U0_RX_SIZE];                                              // 数据缓冲区2048

void U0Rx_PtrInit(void)
{
    U0CB.URxDataIN = &U0CB.URxDataPtr[0];
    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
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
    
    // 配置所需时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // 配置GPIO口
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // USART1 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // USART1 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 配置串口
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    // 配置DMA
    DMA_DeInit(DMA1_Channel5);  // USART1_RX使用DMA1通道5
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)U0_RxBuff;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = U0_RX_MAX;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);
    
    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_Cmd(USART1, ENABLE);
    
    // 配置NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 配置串口空闲中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 配置DMA中断（可选）
    // NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    // NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    // DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); // 如需DMA传输完成中断则启用
    
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
void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        volatile uint16_t temp = USART1->SR;
        temp = USART1->DR;
        
        uint16_t remaining = DMA_GetCurrDataCounter(DMA1_Channel5);
        uint16_t received = (U0_RX_MAX+1) - remaining;
        
        uint8_t currentIndex = U0CB.URxDataIN - U0CB.URxDataPtr;
        U0CB.packetValid[currentIndex] = 1;
        
        U0CB.URxDataIN->end = &U0_RxBuff[(U0CB.URxCounter + received - 1) % U0_RX_SIZE];
        
        U0CB.URxCounter = (U0CB.URxCounter + received) % U0_RX_SIZE;
        
        // 检查下一个数据包位置是否已经被使用
        uint8_t nextIndex = (currentIndex + 1) % NUM;
        if(U0CB.packetValid[nextIndex])
        {
            // 如果下一个位置已经被使用，标记它为无效
            U0CB.packetValid[nextIndex] = 0;
            
            // 如果这个被覆盖的数据包是URxDataOUT指向的，需要移动URxDataOUT
            if(nextIndex == (U0CB.URxDataOUT - U0CB.URxDataPtr))
            {
                U0CB.URxDataOUT++;
                if(U0CB.URxDataOUT > U0CB.URxDataEND)
                    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
            }
        }
        
        U0CB.URxDataIN++;
        if(U0CB.URxDataIN > U0CB.URxDataEND)
            U0CB.URxDataIN = &U0CB.URxDataPtr[0];
        
        U0CB.nextPacketIndex = U0CB.URxDataIN - U0CB.URxDataPtr;
        
        U0CB.URxDataIN->start = &U0_RxBuff[U0CB.URxCounter];
        
        // 重置DMA
        DMA_Cmd(DMA1_Channel5, DISABLE);
        DMA1_Channel5->CMAR = (uint32_t)&U0_RxBuff[U0CB.URxCounter];
        DMA_SetCurrDataCounter(DMA1_Channel5, U0_RX_MAX+1);
        DMA_ClearFlag(DMA1_FLAG_TC5);
        DMA_Cmd(DMA1_Channel5, ENABLE);
    }
}

// DMA中断服务函数（如需使用）
//void DMA1_Channel5_IRQHandler(void)
//{
//    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
//    {
//        DMA_ClearITPendingBit(DMA1_IT_TC5);  // 修改这里：使用正确的函数
//        // 处理传输完成中断
//    }
//}
