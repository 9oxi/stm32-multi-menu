#ifndef __BUZZER_H                                    // 防止重复包含；宏名与文件名 Buzzer.h 对应
#define __BUZZER_H                                    // 定义宏，表示已经包含过本文件

void Buzzer_Init(void);                               // 初始化蜂鸣器：PA8配成推挽输出，先关掉（输出高电平=不响）
void Buzzer_Set(uint8_t on);                          // 控制蜂鸣器：on=1响，on=0不响

#endif                                                // 条件编译结束
