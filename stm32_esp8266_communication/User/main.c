#include "Delay.h"
#include "Key.h"
#include "L298N.h"
#include "LED.h"
#include "Serial.h"
#include "esp8266.h"
#include "MPU6050.h"
#include "HX711.h"
#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>  // 必须包含，用于 sprintf
#include <string.h> // 用于字符串操作

// 基于stm32的水稻收割机器人系统设计

#include "PID.h"

// 全局变量用于模拟机器人状态
float RiceWeight = 0.0;
uint8_t Battery = 100;
uint8_t RobotRunning = 1; // 0: 停止, 1: 运行

// PID 控制器实例
PID_TypeDef MotorL_PID, MotorR_PID;

int main(void) {
  char DisplayBuf[256]; // 增加容量以容纳更多传感器数据
  
  // 传感器原始数据变量
  int16_t ax, ay, az, gx, gy, gz;
  uint32_t hx711_val;
  static uint32_t hx711_offset = 0;

  // 中断优先级分组配置，建议在整个系统初始化最前面调用一次
  // 设置为NVIC_PriorityGroup_2，可以配置2位抢占优先级和2位子优先级
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  // 初始化硬件外设
  LED_Init();     // 初始化LED相关硬件
  Key_Init();     // 初始化按键硬件 (PB1, PB11)
  Serial_Init();  // 初始化串口通信（USART1）
  L298N_Init();   // 初始化L298N电机驱动
  ESP8266_Init(); // 初始化ESP8266模块
  
  MPU6050_Init(); // 初始化姿态传感器
  HX711_Init();   // 初始化称重模块
  
  // 获取初始重量偏移（去皮）
  Delay_ms(100);
  hx711_offset = HX711_Read();

  // 初始化 PID (参数: Kp, Ki, Kd, MaxOutput, MinOutput)
  // 注意：这里的 PID 分别对应输出 PWM 的占空比 (0-100)
  PID_Init(&MotorL_PID, 1.5f, 0.1f, 0.05f, 100.0f, 0.0f);
  PID_Init(&MotorR_PID, 1.5f, 0.1f, 0.05f, 100.0f, 0.0f);

  if (RobotRunning) {
    LED1_ON();
  }

  // ... 前面部分逻辑略 ...

  /* ======= 硬件接口 PB10/PB11 测试模式（测试完请删除或注释） ======= */
  // 使用方法：用万用表直流电压档测量 PB10 引脚（TX3）
  // 正常表现：电压应在 0V 到 3.3V 之间快速跳变，或稳定在约 1.6V 左右（高频发送
  // U 时）
  while (0) {                  // 修改为 1 即可进入测试模式
    ESP8266_SendData("UUUUU"); // 发送 0101 序列，方便观察电平变化
    LED1_Turn();               // 伴随 LED 闪烁
    Delay_ms(100);
  }
  /* ========================================================== */

  while (1) // 主循环，持续执行
  {
    // ... 按键逻辑略 ...

    // 根据机器人状态执行动作
    if (RobotRunning) {
      // --- PID 模拟调用示例 ---
      // 假设目标转速是 70，从编码器读取的实际转速是 current_speed
      float target_speed = 70.0;
      float current_speed_L =
          65.0; // 这是一个模拟值，实际应从编码器相关函数读取
      float current_speed_R = 66.0;

      // 计算 PID 输出 (修正后的 PWM 占空比)
      float speedL =
          PID_Incremental_Compute(&MotorL_PID, target_speed, current_speed_L);
      float speedR =
          PID_Incremental_Compute(&MotorR_PID, target_speed, current_speed_R);

      // 将 PID 结果应用到电机
      MotorA_Control(FORWARD, (uint8_t)speedL);
      MotorB_Control(FORWARD, (uint8_t)speedR);

      RiceWeight += 0.05;
      if (Battery > 10 && (RiceWeight > 5.0 || Battery > 20)) {
        static uint16_t battery_cnt = 0;
        if (++battery_cnt >= 10) { // 每 10 秒减 1% 电量
          Battery--;
          battery_cnt = 0;
        }
      }
      
      // 读取物理传感器数据
      MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
      hx711_val = HX711_Read();
      
      // 计算实际重量（根据你的传感器量程调整系数，这里假设转换）
      // RiceWeight = (float)(hx711_val - hx711_offset) / 400.0f; // 示例系数
      
    } else {
      Robot_Move(ROBOT_STOP);
    }

    // ============ 新增通信拦截锁 ============
    // 只有在ESP8266成功连接上服务器时，才允许发送数据包
    if (ESP8266_Connected == 1) {
      // 数据打包并上报（JSON格式，包含新增传感器信息）
      sprintf(DisplayBuf,
              "{\"weight\":%.2f,\"motor_status\":%d,\"battery\":%d,\"storage_full\":false,"
              "\"accel\":{\"x\":%d,\"y\":%d,\"z\":%d},"
              "\"gyro\":{\"x\":%d,\"y\":%d,\"z\":%d},"
              "\"device_id\":\"STM32_001\"}",
              RiceWeight, RobotRunning ? 1 : 0, Battery,
              ax, ay, az, gx, gy, gz);

      // 串口1打印调试信息以便监控
      // UART1_SendString("Send JSON: ");
      // UART1_SendString(DisplayBuf);
      // UART1_SendString("\r\n");

      // 通过ESP8266封装成协议帧发送
      ESP8266_SendFrame(DisplayBuf);
    }
    // ==========================================

    Delay_ms(1000); // 降低上报频率至 1Hz，适合远程通信
  }
}
