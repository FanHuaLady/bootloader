#ifndef __ENCODER_H
#define __ENCODER_H
#include "stm32f10x.h"                  // Device header

void Encoder_Init(void);
int16_t Encoder_Get(void);
int8_t Encoder_Get_Direction(void);

#endif
