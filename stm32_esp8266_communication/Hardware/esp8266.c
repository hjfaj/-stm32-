#include "esp8266.h"
#include "Delay.h"
#include "LED.h"
#include "Serial.h"
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

uint8_t ESP8266_Connected = 0; // 0=未连接或断开, 1=已连接服务器

/**
 * @brief 串口3初始化 (PB10->TX, PB11->RX)
 */
void USART3_Init(unsigned int baud) {
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = baud;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl =
      USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART3, &USART_InitStructure);
  USART_Cmd(USART3, ENABLE);
}

/**
 * @brief 检查响应并透传日志到调试串口 (UART1)
 */
uint8_t ESP8266_CheckResponse(char *response, uint32_t timeout) {
  char buffer[512] = {0};
  uint16_t index = 0;

  while (timeout > 0) {
    while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
      char c = USART_ReceiveData(USART3);

      if (index < 500) {
        buffer[index++] = c;
        buffer[index] = '\0';
      }

      // 【关键容错】: 只要缓冲区里新进了字符，就立刻搜索！不管有没有换行！
      // 这能完美过滤掉极其恶心的模块回显（比如 "AT\r\n\r\nOK\r\n"
      // 这种碎片化返回）
      if (strstr(buffer, response) != NULL) {
        UART1_SendString("[MATCHED]: ");
        UART1_SendString(response);
        UART1_SendString("\r\n");
        return 1;
      }

      // 如果缓冲区快满了（防止越界），或者遇到长换行群，才清空重来
      if (index >= 480) {
        UART1_SendString("[ESP8266_RAW_CHUNK]: ");
        UART1_SendString(buffer);
        memset(buffer, 0, sizeof(buffer));
        index = 0;
      }
    }

    Delay_ms(1);
    timeout--;
  }

  // 超时前把残留在缓冲区的尸体打印出来，方便死后排错
  if (index > 0) {
    UART1_SendString("[ESP8266_RAW_TIMEOUT]: ");
    UART1_SendString(buffer);
    UART1_SendString("\r\n");
  }

  return 0;
}

/**
 * @brief 向模块发送原始字符串
 */
void ESP8266_SendData(char *data) {
  while (*data) {
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
      ;
    USART_SendData(USART3, *data++);
  }
}

/**
 * @brief 向模块发送单字节
 */
void ESP8266_SendByte(uint8_t byte) {
  while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
    ;
  USART_SendData(USART3, byte);
}

/**
 * @brief 清空串口接收缓冲，防止乱码干扰
 */
void Clear_USART3_Buffer(void) {
  while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
    USART_ReceiveData(USART3);
  }
}

/**
 * @brief 初始化整体流程
 */
void ESP8266_Init(void) {
  char cmd[100];
  ESP8266_Connected = 0;

  UART1_SendString("\r\n--- STM32 TO REMOTE SERVER DEBUG ---\r\n");

  // 尝试默认 115200 波特率
  USART3_Init(115200);
  UART1_SendString("Trying 115200 baudrate...\r\n");

  UART1_SendString("Sending '+++' to exit transparent transmission...\r\n");
  ESP8266_SendData("+++");
  Delay_ms(1500);
  ESP8266_SendData("\r\n");
  Clear_USART3_Buffer();
  Delay_ms(500);

  UART1_SendString("Waiting for ESP8266 boot...\r\n");
  Delay_ms(2000);

  // 【强力镇压回显】：在任何探测前先盲发一次 ATE0
  // 强关模块自身的回显，并清空历史垃圾
  ESP8266_SendData("ATE0\r\n");
  Delay_ms(200);
  Clear_USART3_Buffer();

  uint8_t hw_ok = 0;
  // 阶段 1: 115200 探测
  for (int i = 0; i < 3; i++) {
    ESP8266_SendData("AT\r\n");
    if (ESP8266_CheckResponse("OK", 500)) {
      hw_ok = 1;
      break;
    }
    UART1_SendString("[1] 115200 Retrying...\r\n");
    Delay_ms(500);
  }

  // 阶段 2: 9600 补救探测 (某些出厂默认或意外切至 9600)
  if (!hw_ok) {
    UART1_SendString("Switching to 9600 baudrate...\r\n");
    USART3_Init(9600);
    Clear_USART3_Buffer();
    Delay_ms(500);
    ESP8266_SendData("+++");
    Delay_ms(1500);
    ESP8266_SendData("\r\n");

    for (int i = 0; i < 3; i++) {
      ESP8266_SendData("AT\r\n");
      if (ESP8266_CheckResponse("OK", 500)) {
        hw_ok = 1;
        UART1_SendString("Found device at 9600! Forcing to 115200...\r\n");
        ESP8266_SendData("AT+UART_DEF=115200,8,1,0,0\r\n");
        Delay_ms(500);
        USART3_Init(115200); // 切回高速
        break;
      }
      UART1_SendString("[1] 9600 Retrying...\r\n");
      Delay_ms(500);
    }
  }

  if (hw_ok) {
    UART1_SendString("[1] Hardware Ready: OK\r\n");
  } else {
    // 最后大招：物理软重启
    UART1_SendString("Issuing AT+RST soft reset...\r\n");
    USART3_Init(115200);
    ESP8266_SendData("AT+RST\r\n");
    Delay_ms(3000);

    ESP8266_SendData("AT\r\n");
    if (ESP8266_CheckResponse("OK", 1000)) {
      UART1_SendString("[1] Hardware Ready (After Reset): OK\r\n");
    } else {
      UART1_SendString("[1] Hardware Ready: FATAL ERROR (Hardware Dead / Need "
                       "Power Cycle)\r\n");
      return;
    }
  }

  // 关闭回显以减少垃圾日志
  ESP8266_SendData("ATE0\r\n");
  Delay_ms(100);

  ESP8266_SendData("AT+CWMODE=1\r\n");
  ESP8266_CheckResponse("OK", 500);

  // 【关键：强制设为单连接模式以匹配 CIPSTART 语法】
  ESP8266_SendData("AT+CIPMUX=0\r\n");
  ESP8266_CheckResponse("OK", 500);

  // 断开可能遗留的连接
  ESP8266_SendData("AT+CIPCLOSE\r\n");
  ESP8266_CheckResponse("OK", 500);

  sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
  ESP8266_SendData(cmd);
  UART1_SendString("[2] WiFi: Connecting to ");
  UART1_SendString(WIFI_SSID);
  UART1_SendString("...\r\n");

  uint8_t wifi_ok = 0;
  for (int i = 0; i < 80; i++) { // 给热点一点时间，约16秒
    LED1_Turn();
    if (ESP8266_CheckResponse("WIFI GOT IP", 200)) {
      wifi_ok = 1;
      break;
    }
    if (i % 5 == 0)
      UART1_SendString("Waiting WiFi...\r\n");
  }

  if (wifi_ok) {
    UART1_SendString("2. WiFi Status: CONNECTED\r\n");
    LED1_OFF();

    // 【关键】：分配 IP 后等待至少 2 秒，让热点的 NAT 路由生效
    UART1_SendString("   Waiting for NAT routing to settle...\r\n");
    Delay_ms(2500);

    uint8_t server_ok = 0;
    for (int retry = 1; retry <= 2; retry++) {
      sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", SERVER_IP, SERVER_PORT);
      ESP8266_SendData(cmd);
      UART1_SendString("3. Connecting Server (Try ");
      sprintf(cmd, "%d", retry);
      UART1_SendString(cmd);
      UART1_SendString(")...\r\n");

      // 公网环境将超时增加到 10 秒
      if (ESP8266_CheckResponse("CONNECT", 10000) ||
          ESP8266_CheckResponse("ALREADY CONNECTED", 100)) {
        server_ok = 1;
        break;
      }
      UART1_SendString("[3] Server Connect Failed, retrying...\r\n");
      Delay_ms(2000);
    }

    if (server_ok) {
      UART1_SendString("3. Server Status: ONLINE\r\n");
      LED1_ON();
      ESP8266_Connected = 1; // 成功置位
    } else {
      UART1_SendString("3. Server Status: FAILED (Check Firewall/IP)\r\n");
      LED1_OFF();
    }
  } else {
    UART1_SendString("2. WiFi Status: TIMEOUT\r\n");
    LED1_OFF();
  }
}

/**
 * @brief 封包发送 JSON 数据（含帧头帧尾）
 */
void ESP8266_SendFrame(const char *json) {
  uint16_t len = strlen(json);
  char cmd[32];
  sprintf(cmd, "AT+CIPSEND=%d\r\n", len + 6);
  ESP8266_SendData(cmd);

  // 【关键修复】：必须等待 ESP8266 准备好并返回
  // '>'，否则盲发包会被当做非法指令导致断连
  if (ESP8266_CheckResponse(">", 2000)) {
    ESP8266_SendByte(0xAA);
    ESP8266_SendByte(0xBB);
    ESP8266_SendByte((uint8_t)(len >> 8));
    ESP8266_SendByte((uint8_t)(len & 0xFF));

    while (*json) {
      ESP8266_SendByte((uint8_t)*json++);
    }

    ESP8266_SendByte(0xCC);
    ESP8266_SendByte(0xDD);
  } else {
    UART1_SendString("[ERROR] CIPSEND Timeout! No '>' prompt.\r\n");
  }
}
