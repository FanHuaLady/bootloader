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

typedef struct{                         // ����֪�����ݴ����ĸ�λ��
    uint16_t URxCounter;                // ����
    UCB_URxBuffptr URxDataPtr[NUM];     // ���������ݵĽṹ������
    UCB_URxBuffptr *URxDataIN;          // ���ݴ��ڼ�¼
    UCB_URxBuffptr *URxDataOUT;         // ���ݴ����¼
    UCB_URxBuffptr *URxDataEND;         // ���ݹ���ﵽ�洢���޼�¼ָ��
}UCB_CB;

void Serial_Init(void);

#endif
