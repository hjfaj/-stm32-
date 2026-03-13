#include "PID.h"

/**
 * @brief  PID初始化
 * @param  pid: PID结构体指针
 * @param  kp/ki/kd: PID系数
 * @param  max/min: 输出限幅范围
 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max,
              float min) {
  pid->Kp = kp;
  pid->Ki = ki;
  pid->Kd = kd;

  pid->Target = 0;
  pid->Current = 0;

  pid->Error = 0;
  pid->LastError = 0;
  pid->PrevError = 0;

  pid->Output = 0;
  pid->MaxOutput = max;
  pid->MinOutput = min;
}

/**
 * @brief  增量式PID计算
 * @param  pid: PID结构体指针
 * @param  target: 目标值
 * @param  current: 当前反馈值
 * @retval PID输出值 (增量累加后的结果)
 * @note   公式: Δu = Kp(e[n]-e[n-1]) + Ki*e[n] + Kd(e[n]-2*e[n-1]+e[n-2])
 */
float PID_Incremental_Compute(PID_TypeDef *pid, float target, float current) {
  pid->Target = target;
  pid->Current = current;

  // 计算当前误差
  pid->Error = pid->Target - pid->Current;

  // 计算增量
  float delta_output =
      pid->Kp * (pid->Error - pid->LastError) + pid->Ki * pid->Error +
      pid->Kd * (pid->Error - 2 * pid->LastError + pid->PrevError);

  // 累加到输出
  pid->Output += delta_output;

  // 保存误差用于下次计算
  pid->PrevError = pid->LastError;
  pid->LastError = pid->Error;

  // 输出限幅
  if (pid->Output > pid->MaxOutput)
    pid->Output = pid->MaxOutput;
  if (pid->Output < pid->MinOutput)
    pid->Output = pid->MinOutput;

  return pid->Output;
}

/**
 * @brief  设置目标值
 */
void PID_SetTarget(PID_TypeDef *pid, float target) { pid->Target = target; }
