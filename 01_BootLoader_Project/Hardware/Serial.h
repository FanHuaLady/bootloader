#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"                  // Device header

#define U0_RX_SIZE 2048                 // �������������
#define U0_RX_MAX 256                   // һ�������������
#define NUM 10                          // �������ݱ���������

typedef struct{                         // ���������ݵĽṹ��
    uint8_t *start;
    uint8_t *end;
}UCB_URxBuffptr;

typedef struct{
    uint16_t URxCounter;                // ����
    UCB_URxBuffptr URxDataPtr[NUM];     // ���������ݵĽṹ������
    UCB_URxBuffptr *URxDataIN;          // ���ݴ���λ��ָ��
    UCB_URxBuffptr *URxDataOUT;         // ���ݴ����¼ָ��
    UCB_URxBuffptr *URxDataEND;         // ���ݹ���ﵽ�洢���޼�¼ָ��
    uint8_t packetValid[NUM];           // ������ݰ��Ƿ���Ч
    uint8_t nextPacketIndex;            // ��һ�������������ݰ�����
}UCB_CB;

extern UCB_CB U0CB;
extern uint8_t U0_RxBuff[U0_RX_SIZE];

void Serial_Init(void);
void MarkPacketProcessed(uint8_t index);

#endif
