#ifndef __MENU_H                                      // 防止重复包含；宏名与文件名 Menu.h 对应
#define __MENU_H                                      // 定义宏，表示已经包含过本文件

void Menu_Init(void);                                 // 菜单初始化：上电显示主菜单，光标停在第1项
void Menu_Task(void);                                 // 菜单主任务：主循环里反复调用，负责扫按键、走时钟、刷屏幕

#endif                                                // 条件编译结束
