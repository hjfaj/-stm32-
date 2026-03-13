
#include "L298N.h"
#include "stm32f10x.h"

// ========== JGB37-520 减速电机参数 ==========
// 额定电压: 12V
// 额定转速: 110RPM (空载)
// 减速比: 约1:90
// 特点: 高扭矩、低转速，适合机器人底盘驱动
// PWM频率建议: 500Hz (减速电机适合较低频率，运行更平稳)
// 默认行驶速度: 70% (减速电机扭矩足够，无需满速)

// PWM相关变量
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;

/**
 * @brief  L298N电机驱动初始化函数
 * @note   初始化电机控制引脚
 *         Motor A: IN1-PA7, IN2-PA8, ENA-PA0 (TIM2_CH1)
 *         Motor B: IN3-PA11, IN4-PA4, ENB-PA6 (TIM3_CH1)
 */
void L298N_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  // 开启GPIOA时钟，以及TIM2和TIM3时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3, ENABLE);

  // 配置电机方向控制引脚为推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

  // 配置电机A方向控制引脚 (IN1-PA7, IN2-PA8)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 配置电机B方向控制引脚 (IN3-PA11, IN4-PA4)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_4;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 配置ENA信号输出引脚 (PA0) 为复用推挽输出 (TIM2_CH1 PWM)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 配置ENB信号输出引脚 (PA6) 为复用推挽输出 (TIM3_CH1 PWM)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  // 初始状态下所有方向引脚置低，电机停止
  GPIO_ResetBits(GPIOA, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_4);

  // ========== TIM2 配置（电机A PWM，PA0-CH1，500Hz）==========
  TIM_TimeBaseStructure.TIM_Period = 999; // 1000个计数值
  TIM_TimeBaseStructure.TIM_Prescaler =
      143; // 72MHz/(143+1)=500kHz, 500kHz/1000=500Hz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为0
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
  TIM_Cmd(TIM2, ENABLE);

  // ========== TIM3 配置（电机B PWM，PA6-CH1，500Hz）==========
  TIM_TimeBaseStructure.TIM_Period = 999;
  TIM_TimeBaseStructure.TIM_Prescaler = 143;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为0
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  电机A控制函数
 * @param  direction: 电机方向 (FORWARD-前进, BACKWARD-后退, STOP-停止)
 * @param  speed: 电机速度 (0-100, 对应PWM占空比)
 * @note   使用TIM2_CH1 (PA0) 输出PWM控制电机A速度
 */
void MotorA_Control(uint8_t direction, uint8_t speed) {
  // 限制速度范围并设置PWM占空比
  if (speed > 100)
    speed = 100;
  TIM_SetCompare1(TIM2, speed * 10); // 100% -> CCR=1000

  switch (direction) {
  case FORWARD:
    GPIO_SetBits(GPIOA, GPIO_Pin_7);   // IN1高
    GPIO_ResetBits(GPIOA, GPIO_Pin_8); // IN2低
    break;

  case BACKWARD:
    GPIO_ResetBits(GPIOA, GPIO_Pin_7); // IN1低
    GPIO_SetBits(GPIOA, GPIO_Pin_8);   // IN2高
    break;

  case STOP:
  default:
    TIM_SetCompare1(TIM2, 0); // 停止时速度为0
    GPIO_ResetBits(GPIOA, GPIO_Pin_7);
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    break;
  }
}

/**
 * @brief  电机B控制函数
 * @param  direction: 电机方向 (FORWARD-前进, BACKWARD-后退, STOP-停止)
 * @param  speed: 电机速度 (0-100, 对应PWM占空比)
 * @note   使用TIM3_CH1 (PA6) 输出PWM控制电机B速度
 */
void MotorB_Control(uint8_t direction, uint8_t speed) {
  // 限制速度范围并设置PWM占空比
  if (speed > 100)
    speed = 100;
  TIM_SetCompare1(TIM3, speed * 10);

  switch (direction) {
  case FORWARD:
    GPIO_SetBits(GPIOA, GPIO_Pin_11);  // IN3高
    GPIO_ResetBits(GPIOA, GPIO_Pin_4); // IN4低
    break;

  case BACKWARD:
    GPIO_ResetBits(GPIOA, GPIO_Pin_11); // IN3低
    GPIO_SetBits(GPIOA, GPIO_Pin_4);    // IN4高
    break;

  case STOP:
  default:
    TIM_SetCompare1(TIM3, 0); // 停止时速度为0
    GPIO_ResetBits(GPIOA, GPIO_Pin_11);
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
    break;
  }
}

/**
 * @brief  机器人运动控制函数
 * @param  direction: 运动方向 (GO_FORWARD-前进, GO_BACKWARD-后退,
 * TURN_LEFT-左转, TURN_RIGHT-右转, ROBOT_STOP-停止)
 * @note   通过控制两个电机的方向实现机器人的整体运动
 */
void Robot_Move(uint8_t direction) {
  switch (direction) {
  case GO_FORWARD:
    // JGB37-520减速电机扭矩大，70%速度即可稳定前进
    MotorA_Control(FORWARD, MOTOR_DEFAULT_SPEED);
    MotorB_Control(FORWARD, MOTOR_DEFAULT_SPEED);
    break;

  case GO_BACKWARD:
    MotorA_Control(BACKWARD, MOTOR_DEFAULT_SPEED);
    MotorB_Control(BACKWARD, MOTOR_DEFAULT_SPEED);
    break;

  case TURN_LEFT:
    // 差速转弯：一侧低速反转，另一侧正常速度，转弯更平稳
    MotorA_Control(BACKWARD, MOTOR_TURN_SPEED);
    MotorB_Control(FORWARD, MOTOR_DEFAULT_SPEED);
    break;

  case TURN_RIGHT:
    // 差速转弯：一侧正常速度，另一侧低速反转
    MotorA_Control(FORWARD, MOTOR_DEFAULT_SPEED);
    MotorB_Control(BACKWARD, MOTOR_TURN_SPEED);
    break;

  case ROBOT_STOP:
  default:
    MotorA_Control(STOP, 0);
    MotorB_Control(STOP, 0);
    break;
  }
}
