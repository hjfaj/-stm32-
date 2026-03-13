#include "Serial.h"
#include "LED.h"
#include "stm32f10x.h"

/**
 * @brief  串口初始化函数（USART1）
 * @note   配置USART1串口，包括GPIO引脚、波特率、数据位、停止位等参数，
 *         并配置串口接收中断，使串口能够接收数据并触发中断。
 *         TX-PA9（复用推挽输出），RX-PA10（浮空输入）
 *         波特率115200，8位数据位，1位停止位，无校验
 */
void Serial_Init(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  // TX (PA9) - 复用推挽输出
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  // RX (PA10) - 浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART1, &USART_InitStructure);

  // 开启串口接收中断
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  // 配置NVIC中断优先级
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_Cmd(USART1, ENABLE);
}

/**
 * @brief  通过USART1发送字符串
 * @param  str: 要发送的字符串指针
 * @note   逐字符发送，等待每个字符发送完毕后再发送下一个
 */
void UART1_SendString(char *str)
{
  while (*str)
  {                                 // 遍历字符串直到遇到空字符'\0'
    USART_SendData(USART1, *str++); // 发送当前字符并移动指针到下一个字符
    
    uint16_t timeout = 10000;       // 设置一个超时计数值
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
      timeout--;
      if (timeout == 0)
      {
        break; // 如果等了很久还是发不出去，就强行打破循环，防止死机
      }
    }
  }
}

/**
 * @brief  USART1中断服务函数
 * @note   接收到数据后，根据指令控制LED：
 *         '1' - 点亮LED1
 *         '0' - 熄灭LED1
 */
void USART1_IRQHandler(void)
{
  if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
  {
    uint8_t RxData = USART_ReceiveData(USART1);

    // 远程控制逻辑：接收字符 '1' 亮灯，'0' 灭灯
    if (RxData == '1')
      LED1_ON();
    if (RxData == '0')
      LED1_OFF();

    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}
