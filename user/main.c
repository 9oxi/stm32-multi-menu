#include "stm32f10x.h"                               // 第一行必须是官方库
#include "Delay.h"                                   // 系统节拍：TIM4每1ms中断一次
#include "OLED.h"                                    // 四针I2C屏幕（SCL=PB8，SDA=PB9）
#include "Key.h"                                     // 4个按键（PA0~PA3，上拉输入）
#include "Buzzer.h"                                  // 有源蜂鸣器（PA4，高电平响）
#include "IR.h"                                      // 红外传感器（PB0，外部中断，数圈数）
#include "Menu.h"                                    // 多级菜单的全部逻辑都在这里

int main(void)                                       // 程序入口
{
    Delay_Init();                                    // 第一步先开系统节拍；后面按键消抖、时钟、刷屏全靠它

    Key_Init();                                      // 初始化4个按键（上、下、确认、取消）
    Buzzer_Init();                                   // 初始化蜂鸣器，先关掉，上电不叫
    OLED_Init();                                     // 初始化屏幕，屏幕亮起来

    IR_Init();                                       // 初始化红外传感器，开外部中断准备数圈数

    Menu_Init();                                     // 菜单初始化：清屏并显示主菜单，光标停在第1项

    while (1)                                        // 主循环：死循环，永远不退出
    {
        Menu_Task();                                 // 菜单任务：扫按键、走时钟、刷屏幕；函数内部靠时间判断，不会卡住
    }
}
