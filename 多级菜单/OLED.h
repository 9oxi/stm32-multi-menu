#ifndef __OLED_H                                      // 防止重复包含；宏名与文件名 OLED.h 对应
#define __OLED_H                                      // 定义宏，表示已经包含过本文件

void OLED_Init(void);                                 // 初始化屏幕：配好PB8/PB9两个引脚，再发一串开机命令
void OLED_Clear(void);                                // 清屏：全屏点阵写0，变成全黑
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);          // 在第Line行(1~4)、第Column列(1~16)显示一个字符Char
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);     // 从第Line行第Column列开始，显示一整串字符String

#endif                                                // 条件编译结束
