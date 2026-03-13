#include "stm32f10x.h" // Device header

void LED_Init(void) {
  // 开启GPIOB和GPIOC的时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  // LED1 使用 PC13
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // LED2 使用 PB14
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // 默认置高电平（灭灯，标准核心板低电平点亮）
  GPIO_SetBits(GPIOC, GPIO_Pin_13);
  GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

// 恢复：ON 为置低，OFF 为置高（适配标准低电平点亮的核心板 PC13）
void LED1_ON(void) { GPIO_ResetBits(GPIOC, GPIO_Pin_13); }
void LED1_OFF(void) { GPIO_SetBits(GPIOC, GPIO_Pin_13); }

void LED1_Turn(void) {
  if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13) == 0) {
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
  } else {
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
  }
}

void LED2_ON(void) { GPIO_SetBits(GPIOB, GPIO_Pin_14); }
void LED2_OFF(void) { GPIO_ResetBits(GPIOB, GPIO_Pin_14); }

void LED2_Turn(void) {
  if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_14) == 0) {
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
  } else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
  }
}
