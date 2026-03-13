#include "Delay.h"
#include "stm32f10x.h" // Device header

/* 按键引脚定义 */
#define KEY1_PIN GPIO_Pin_1
#define KEY2_PIN GPIO_Pin_0

/* 按键编号定义 */
#define KEY_NONE 0
#define KEY1_NUM 1
#define KEY2_NUM 2

/* 按键状态掩码 */
#define KEY1_MASK 0x0002 // GPIO_Pin_1
#define KEY2_MASK 0x0001 // GPIO_Pin_0

/* 按键消抖时间定义 */
#define KEY_DEBOUNCE_TIME 20 // 消抖延时时间(ms)

void Key_Init(void) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  获取按键编号（检测按键按下）
 * @param  无
 * @retval 按键编号：KEY_NONE(0)、KEY1_NUM(1)、KEY2_NUM(2)
 * @note   修复了同时按下或重叠按下时按键信号丢失的问题
 */
uint8_t Key_GetNum(void) {
  static uint8_t key1_prev_state = 1;
  static uint8_t key2_prev_state = 1;

  uint16_t port_data = GPIO_ReadInputData(GPIOB);
  uint8_t key1_curr_state = (port_data & KEY1_MASK) ? 1 : 0;
  uint8_t key2_curr_state = (port_data & KEY2_MASK) ? 1 : 0;

  /* ---------------- 检测按键1 ---------------- */
  if (key1_prev_state == 1 && key1_curr_state == 0) {
    Delay_ms(KEY_DEBOUNCE_TIME);
    if ((GPIO_ReadInputData(GPIOB) & KEY1_MASK) == 0) {
      key1_prev_state = 0; // 手动更新状态，防止下次重复触发
      return KEY1_NUM;     // 立即返回，不干扰按键2的后续检测
    }
  } else {
    // 只有在没有触发下降沿时，才正常更新上一次的状态
    key1_prev_state = key1_curr_state;
  }

  /* ---------------- 检测按键2 ---------------- */
  if (key2_prev_state == 1 && key2_curr_state == 0) {
    Delay_ms(KEY_DEBOUNCE_TIME);
    if ((GPIO_ReadInputData(GPIOB) & KEY2_MASK) == 0) {
      key2_prev_state = 0;
      return KEY2_NUM;
    }
  } else {
    key2_prev_state = key2_curr_state;
  }

  return KEY_NONE;
}
