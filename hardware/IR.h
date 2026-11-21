#ifndef __IR_H                                        // 防止重复包含；宏名与文件名 IR.h 对应
#define __IR_H                                        // 定义宏，表示已经包含过本文件

void IR_Init(void);                                   // 初始化反射式红外传感器：PB0配成外部中断，下降沿触发
uint16_t IR_GetLap(void);                             // 读当前圈数
void IR_ResetLap(void);                               // 圈数清零（每次进测速菜单时用）

#endif                                                // 条件编译结束
