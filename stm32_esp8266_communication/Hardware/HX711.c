#include "HX711.h"
#include "Delay.h"

/* 引脚定义：SCK->PB12, DT->PB13 */

/**
  * 函    数：HX711初始化
  * 参    数：无
  * 返 回 值：无
  */
void HX711_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// SCK 为输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// DT 为输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_12); // SCK置低
}

/**
  * 函    数：HX711读取原始值
  * 参    数：无
  * 返 回 值：24位称重原始数据
  */
uint32_t HX711_Read(void)
{
	uint32_t count;
	uint8_t i;
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 确保时钟为低
	count = 0;
	
	// 等待数据就绪
	while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 1);
	
	for (i = 0; i < 24; i ++)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_12);
		count = count << 1;
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0)
		{
			count ++;
		}
	}
	
	// 第25个脉冲，设定增益为128
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	count = count ^ 0x800000; // 转换成无符号数
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	
	return count;
}
