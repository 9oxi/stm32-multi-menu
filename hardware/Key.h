#ifndef __KEY_H                                       // 防止重复包含；宏名与文件名 Key.h 对应
#define __KEY_H                                       // 定义宏，表示已经包含过本文件

#define KEY_NONE   0                                  // 没有按键按下（键值0）
#define KEY_UP     1                                  // 上键（PA0）按下
#define KEY_DOWN   2                                  // 下键（PA2）按下
#define KEY_OK     3                                  // 确认键（PA4）按下
#define KEY_CAN    4                                  // 取消键（PA6）按下

void Key_Init(void);                                  // 初始化4个按键：全配成上拉输入（不按是高电平，按下是低电平0）
uint8_t Key_Scan(void);                               // 扫一次按键：返回上面5个键值之一，已经做过消抖

#endif                                                // 条件编译结束
