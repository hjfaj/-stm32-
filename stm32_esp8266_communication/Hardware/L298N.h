
#ifndef __L298N_H
#define __L298N_H

#include "stm32f10x.h"

// 电机方向定义
#define FORWARD 1  // 前进
#define BACKWARD 2 // 后退
#define STOP 0     // 停止

// JGB37-520减速电机速度参数 (0-100, 对应PWM占空比)
#define MOTOR_DEFAULT_SPEED 70 // 默认行驶速度 (减速电机扭矩大，70%即可)
#define MOTOR_TURN_SPEED 50    // 转弯时内侧轮速度 (差速转弯更平稳)

// 机器人运动方向定义
#define GO_FORWARD 1  // 前进
#define GO_BACKWARD 2 // 后退
#define TURN_LEFT 3   // 左转
#define TURN_RIGHT 4  // 右转
#define ROBOT_STOP 0  // 停止

// 函数声明
void L298N_Init(void);
void MotorA_Control(uint8_t direction, uint8_t speed);
void MotorB_Control(uint8_t direction, uint8_t speed);
void Robot_Move(uint8_t direction);

#endif
