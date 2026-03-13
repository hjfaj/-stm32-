#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"

// 函数声明
void Serial_Init(void);
void UART1_SendString(char *str);

#endif
