#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"                  // Device header

#define U0_RX_SIZE 2048                 // 缓冲区最大容量
#define U0_RX_MAX 256                   // 一次数据最大容量
#define NUM 10                          // 管理数据变量的数量

typedef struct{                         // 管理单个数据的结构体
    uint8_t *start;
    uint8_t *end;
}UCB_URxBuffptr;

typedef struct{                         // 用于知道数据处理到哪个位置
    uint16_t URxCounter;                // 计数
    UCB_URxBuffptr URxDataPtr[NUM];     // 管理多个数据的结构体数组
    UCB_URxBuffptr *URxDataIN;          // 数据存在记录
    UCB_URxBuffptr *URxDataOUT;         // 数据处理记录
    UCB_URxBuffptr *URxDataEND;         // 数据管理达到存储极限记录指针
}UCB_CB;

void Serial_Init(void);

#endif
