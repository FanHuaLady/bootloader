#include "stm32f10x.h"
#include "Serial.h"
#include <stdio.h>

UCB_CB U0CB;                                                                // �������
uint8_t U0_RxBuff[U0_RX_SIZE];                                              // ���ݻ�����2048

uint32_t last_dma_pos = 0;

void U0Rx_PtrInit(void)
{
    U0CB.URxDataIN = &U0CB.URxDataPtr[0];
    U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
    U0CB.URxDataEND = &U0CB.URxDataPtr[NUM-1];
    U0CB.URxCounter = 0;
    U0CB.nextPacketIndex = 0;
    
    // ��ʼ���������ݰ�Ϊ��Ч״̬
    for(int i = 0; i < NUM; i++)
    {
        U0CB.URxDataPtr[i].start = NULL;
        U0CB.URxDataPtr[i].end = NULL;
        U0CB.packetValid[i] = 0;
    }
    
    // ���õ�һ�����ݰ�����ʼλ��
    U0CB.URxDataIN->start = U0_RxBuff;
}

void MarkPacketProcessed(uint8_t index)
{
    if(index < NUM)
    {
        U0CB.packetValid[index] = 0;
        
        // ������ǵ�ǰ��������ݰ����ƶ�URxDataOUTָ��
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
    
    // ��������ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // ����GPIO��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // USART1 TX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // USART1 RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // ���ô���
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);
    
    // ����DMA
    DMA_DeInit(DMA1_Channel5);  // USART1_RXʹ��DMA1ͨ��5
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)U0_RxBuff;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_BufferSize = U0_RX_SIZE;  // ��Ϊ������������С
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;  // ѭ��ģʽ
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);
    
    last_dma_pos = 0;

    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_Cmd(USART1, ENABLE);
    
    // ����NVIC
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // ���ô��ڿ����ж�
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ����DMA�жϣ���ѡ��
    // NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    // NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    // NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    // DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); // ����DMA��������ж�������
    
    U0Rx_PtrInit();
}

// ��дfputc������ʵ��printf�����ڵ��ض���
int fputc(int ch, FILE *f) 
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

// ���ڿ����ж�
// ��ȫ�ֱ��������

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
    {
        volatile uint16_t temp = USART1->SR;
        temp = USART1->DR;
        
        // ����DMA��ȷ����ȫ��ȡ������
        DMA_Cmd(DMA1_Channel5, DISABLE);
        uint16_t remaining = DMA_GetCurrDataCounter(DMA1_Channel5);
        DMA_Cmd(DMA1_Channel5, ENABLE);
        
        // ���㵱ǰDMAλ��
        uint32_t current_dma_pos = (U0_RX_SIZE - remaining) % U0_RX_SIZE;
        
        // ������յ������ݳ���
        uint16_t received;
        if(current_dma_pos >= last_dma_pos)
            received = current_dma_pos - last_dma_pos;
        else
            received = U0_RX_SIZE - last_dma_pos + current_dma_pos;
        
        // ������յ������ݳ���Ϊ0��ֱ�ӷ���
        if(received == 0)
        {
            last_dma_pos = current_dma_pos;
            return;
        }
        
        uint8_t currentIndex = U0CB.URxDataIN - U0CB.URxDataPtr;
        U0CB.packetValid[currentIndex] = 1;
        
        // �������ݰ���ʼ�ͽ���λ��
        U0CB.URxDataIN->start = &U0_RxBuff[last_dma_pos];
        
        // �������λ��
        uint32_t end_pos = (last_dma_pos + received - 1) % U0_RX_SIZE;
        U0CB.URxDataIN->end = &U0_RxBuff[end_pos];
        
        // ����ȫ�ּ�����
        U0CB.URxCounter = (U0CB.URxCounter + received) % U0_RX_SIZE;
        
        // �����һ�����ݰ�λ���Ƿ��Ѿ���ʹ��
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
        
        // �ƶ�����ָ��
        U0CB.URxDataIN++;
        if(U0CB.URxDataIN > U0CB.URxDataEND)
            U0CB.URxDataIN = &U0CB.URxDataPtr[0];
        
        U0CB.nextPacketIndex = U0CB.URxDataIN - U0CB.URxDataPtr;
        
        // �������һ��DMAλ��
        last_dma_pos = current_dma_pos;
        
        // ���������ɱ�־
        DMA_ClearFlag(DMA1_FLAG_TC5);
    }
}

// DMA�жϷ�����������ʹ�ã�
//void DMA1_Channel5_IRQHandler(void)
//{
//    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
//    {
//        DMA_ClearITPendingBit(DMA1_IT_TC5);  // �޸����ʹ����ȷ�ĺ���
//        // ����������ж�
//    }
//}
