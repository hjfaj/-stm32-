#ifndef __HX711_H
#define __HX711_H

#include "stm32f10x.h"

void HX711_Init(void);
uint32_t HX711_Read(void);

#endif
