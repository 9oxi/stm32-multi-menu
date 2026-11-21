#ifndef __DELAY_H                                     // 防止重复包含；宏名固定用文件名大写：Delay.h 对应 __DELAY_H
#define __DELAY_H                                     // 定义这个宏，表示本文件已经包含过一次了

void Delay_Init(void);                                // 初始化TIM4，让它每1ms中断一次，给全系统当“心跳”
uint32_t Get_Tick(void);                              // 返回开机到现在过了多少毫秒（uint32_t就是无符号32位整数）
void Delay_ms(uint32_t ms);                           // 阻塞等待ms毫秒（ms=要等的毫秒数），只给上电初始化时用

#endif                                                // 条件编译到此结束
