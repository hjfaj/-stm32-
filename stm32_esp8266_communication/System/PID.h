#ifndef __PID_H
#define __PID_H

#include "stm32f10x.h"

/**
 * @brief PID控制结构体
 */
typedef struct {
  float Kp; // 比例系数
  float Ki; // 积分系数
  float Kd; // 微分系数

  float Target;  // 目标值
  float Current; // 当前值 (反馈值)

  float Error;     // 当前误差
  float LastError; // 上一次误差
  float PrevError; // 上上次误差 (用于增量式PID)

  float Output;    // PID计算输出值
  float MaxOutput; // 输出限幅最大值
  float MinOutput; // 输出限幅最小值
} PID_TypeDef;

// 函数声明
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max,
              float min);
float PID_Incremental_Compute(PID_TypeDef *pid, float target, float current);
void PID_SetTarget(PID_TypeDef *pid, float target);

#endif
