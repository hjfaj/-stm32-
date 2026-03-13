#ifndef __ESP8266
#define __ESP8266

#include "stm32f10x.h"

// ============ 网络连接配置 (支持公网) ============
#define WIFI_SSID "Test"         // ← 替换为你的WiFi名称
#define WIFI_PASSWORD "88888888" // ← 替换为你的WiFi密码
#define SERVER_IP                                                              \
  "113.45.231.111"       // ← 请将此处修改为 Ubuntu 服务器的【公网IP】或【域名】
#define SERVER_PORT 8888 // ESP8266 连接的服务器端口
// =================================================

// ESP8266通信相关函数声明
void USART2_Init(unsigned int baud);
void ESP8266_Init(void);
void ESP8266_SendData(char *data);
void ESP8266_SendByte(uint8_t byte);
void ESP8266_SendFrame(const char *json);
uint8_t ESP8266_CheckResponse(char *response, uint32_t timeout);

// 全局网络状态标志位
extern uint8_t ESP8266_Connected;
#endif
