# STM32 水稻收割机器人系统设计 🌾

> 2026年本科毕业设计项目。本项目主要用于保存代码进度，并进行版本防错管理。目前正在进行局域网通信版本的开发。

## 🛠️ 技术栈
* **主控芯片:** STM32 系列
* **通信模块:** ESP8266 (当前基于局域网通信)
* **上位机/UI:** Qt C++ 
* **硬件建模:** SolidWorks

## ⚙️ 系统功能 (持续更新中...)
- [x] 基础电机驱动与运动控制
- [ ] ESP8266 局域网通信链路打通
- [ ] Qt 上位机界面开发与数据下发
- [ ] 远端服务器连接 (优化中)

## 📂 目录结构说明
* `/stm32_esp8266_communication` : 下位机 Keil 工程代码
  * `/User` - 主程序与中断处理
  * `/Hardware` - 外设驱动(ESP8266/MPU6050/HX711/L298N等)
  * `/System` - 系统功能(PID控制/延时)
  * `/Library` - STM32标准库文件
* `/QT_an` : 上位机控制端源码
  * `mainwindow.h/cpp` - 主界面与TCP通信实现
* `/paddy-server` : Ubuntu TCP服务器
  * `rice_robot_server.cpp` - 服务器主程序
  * `protocol.h` - 通信协议定义
* `/Android` : Android移植相关文件
* `/arduino` : Arduino辅助测试代码

## 🚀 快速开始
1. 克隆本项目到本地
2. 使用 Keil uVision5 打开 `stm32_esp8266_communication` 工程进行编译烧录
3. 编译并运行 `paddy-server/rice_robot_server.cpp` 作为通信服务器
4. 使用 Qt Creator 打开 `QT_an` 工程编译运行上位机

## 📊 系统架构
```
STM32 (下位机)
  ├─ ESP8266 (WiFi模块)
  │   └─ TCP连接 → Ubuntu服务器
  ├─ 传感器组
  │   ├─ MPU6050 (姿态)
  │   └─ HX711 (称重)
  └─ L298N (电机驱动)

Qt上位机
  └─ TCP连接 → Ubuntu服务器

Ubuntu服务器
  ├─ 监听ESP8266上报端口
  └─ 监听Qt客户端订阅端口
```

## 🔧 关键配置
* **ESP8266通信端口**: `SERVER_PORT` (见 `paddy-server/rice_robot_server.cpp`)
* **Qt客户端端口**: `QT_CLIENT_PORT` (见 `paddy-server/rice_robot_server.cpp`)
* **STM32串口波特率**: 见 `Hardware/Serial.c` 配置

## 📝 开发日志
- 2026.01: 初始化项目结构
- 2026.01: 完成基础电机驱动
- 2026.02: 添加PID控制模块
- 2026.02: 集成MPU6050/HX711传感器
- 2026.03: 开发ESP8266通信模块
- 2026.03: 开发Qt上位机界面

## ⚠️ 注意事项
1. ESP8266初始化前请确保WiFi热点已配置
2. 服务器与STM32需在同一局域网内
3. Qt上位机需正确配置服务器IP和端口
4. PID参数需根据实际电机特性调整

## 📄 许可证
本项目仅供学习交流使用
            本文档及内容由 xjp因为毕设进行创建与优化，保留所有权利。

#基于stm32水稻收割机器人系统设计
2026年毕设，用于保存和防止代码改错进行版本管理
>>>>>>> f794ee7b1f9787a7a3513133defa95ec0150edba
