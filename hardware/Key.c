#include "stm32f10x.h"                               // 第一行必须是官方库
#include "Delay.h"                                   // 要用 Get_Tick() 取毫秒数来算消抖时间
#include "Key.h"                                     // 自己的头文件

#define KEY_PORT     GPIOA                           // 4个按键都接在GPIOA口上
#define KEY_UP_PIN   GPIO_Pin_0                      // 上键接PA0
#define KEY_DOWN_PIN GPIO_Pin_2                      // 下键接PA2
#define KEY_OK_PIN   GPIO_Pin_4                      // 确认键接PA4
#define KEY_CAN_PIN  GPIO_Pin_6                      // 取消键接PA6

void Key_Init(void)                                  // 初始化按键：4条线全配成上拉输入
{
    GPIO_InitTypeDef GPIO_InitStruct;                // GPIO配置结构体

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 打开GPIOA时钟

    GPIO_InitStruct.GPIO_Pin = KEY_UP_PIN | KEY_DOWN_PIN | KEY_OK_PIN | KEY_CAN_PIN;  // 一次选中这4根引脚
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;       // 上拉输入：内部接个电阻到3.3V，不按时读到1，按下接地读到0
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输入模式下速度随便填，不影响功能
    GPIO_Init(KEY_PORT, &GPIO_InitStruct);           // 写入配置
}

uint8_t Key_Scan(void)                               // 扫描按键：返回键值；没按键就返回KEY_NONE
{
    static uint8_t  last_raw = 0;                    // 上次读到的原始电平状态；static让这个值下次调用还留着
    static uint32_t change_ms = 0;                   // 上次原始状态发生变化的时刻（毫秒）
    static uint8_t  locked = 0;                      // 1=这轮按键已经上报过，必须等松手才能再报
    uint8_t raw = KEY_NONE;                          // 本次读到的原始键值，先当成没按

    if (GPIO_ReadInputDataBit(KEY_PORT, KEY_UP_PIN)   == Bit_RESET) raw = KEY_UP;    // PA0是低电平说明上键按着
    if (GPIO_ReadInputDataBit(KEY_PORT, KEY_DOWN_PIN) == Bit_RESET) raw = KEY_DOWN;  // PA2低电平=下键按着
    if (GPIO_ReadInputDataBit(KEY_PORT, KEY_OK_PIN)   == Bit_RESET) raw = KEY_OK;    // PA4低电平=确认键按着
    if (GPIO_ReadInputDataBit(KEY_PORT, KEY_CAN_PIN)  == Bit_RESET) raw = KEY_CAN;   // PA6低电平=取消键按着

    if (raw != last_raw)                             // 电平跟上次不一样，说明手指刚按下或刚松开
    {
        last_raw  = raw;                             // 记住新状态
        change_ms = Get_Tick();                      // 记下变化时刻
        return KEY_NONE;                             // 刚变化的这一刻先不认，等它稳定
    }

    if ((Get_Tick() - change_ms) < 15)               // 同一个电平还没稳住15ms，可能是在抖，不认
        return KEY_NONE;                             // 这就是软件消抖：等15ms再看，抖动基本停了

    if (raw == KEY_NONE)                             // 已经稳定在“没按键”，说明手松开了
    {
        locked = 0;                                  // 解锁，下回按下可以再上报
        return KEY_NONE;                             // 松手不产生键值
    }

    if (locked)                                      // 还按着，但这一按已经报过了
        return KEY_NONE;                             // 不重复报，保证“按一次只触发一次”

    locked = 1;                                      // 记下“这一按报过了”
    return raw;                                      // 上报这一次按键
}
