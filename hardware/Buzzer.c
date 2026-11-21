#include "stm32f10x.h"                               // 第一行必须是官方库
#include "Buzzer.h"                                  // 自己的头文件

#define BUZ_PORT GPIOA                               // 蜂鸣器接在GPIOA口
#define BUZ_PIN  GPIO_Pin_8                          // 信号线接PA4；这是有源蜂鸣器，给高电平就响

void Buzzer_Init(void)                               // 初始化蜂鸣器引脚
{
    GPIO_InitTypeDef GPIO_InitStruct;                // GPIO配置结构体

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 打开GPIOA时钟

    GPIO_InitStruct.GPIO_Pin = BUZ_PIN;              // 选中PA4
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出：能主动输出高电平和低电平，驱动蜂鸣器够用
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速度50MHz
    GPIO_Init(BUZ_PORT, &GPIO_InitStruct);           // 写入配置

    Buzzer_Set(0);                                   // 上电先让它不响，防止一开机就尖叫
}

void Buzzer_Set(uint8_t on)                          // 控制响不响；参数on=1响，on=0不响
{
    if (on)                                          // 要求响
        GPIO_ResetBits(BUZ_PORT, BUZ_PIN);             // PA4输出高电平，有源蜂鸣器就叫
    else                                             // 要求不响
        GPIO_SetBits(BUZ_PORT, BUZ_PIN);           // PA4输出低电平，蜂鸣器停
}
