#include "stm32f10x.h"                               // 第一行必须是官方库
#include "Delay.h"                                   // 中断里要用 Get_Tick() 做防抖
#include "IR.h"                                      // 自己的头文件

#define IR_PORT     GPIOB                            // 传感器信号线接GPIOB口
#define IR_PIN      GPIO_Pin_0                       // 信号线接PB0
#define IR_LINE     EXTI_Line0                       // 用EXTI0这根外部中断线（0号线和各端口第0脚是连通的）
#define IR_PIN_SRC  GPIO_PinSource0                  // 告诉AFIO：中断线0要接到“第0脚”上
#define IR_PORT_SRC GPIO_PortSourceGPIOB             // 再告诉AFIO：第0脚来自GPIOB口，合起来就是PB0

static volatile uint16_t ir_lap  = 0;                // 圈数；volatile：中断里加1，主循环要读到最新值
static volatile uint32_t ir_last = 0;                // 上次触发时刻（毫秒），用来丢掉传感器抖动

void IR_Init(void)                                   // 初始化：PB0上拉输入 + 下降沿中断
{
    GPIO_InitTypeDef GPIO_InitStruct;                // GPIO配置结构体
    EXTI_InitTypeDef EXTI_InitStruct;                // 外部中断配置结构体
    NVIC_InitTypeDef NVIC_InitStruct;                // 中断控制器配置结构体

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);  // 开GPIOB和AFIO时钟；AFIO是“中断线接哪个引脚”的开关

    GPIO_InitStruct.GPIO_Pin = IR_PIN;               // 选中PB0
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;       // 上拉输入：传感器没信号时线保持高电平，输出低电平时才算“遮挡”
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输入模式速度随便填
    GPIO_Init(IR_PORT, &GPIO_InitStruct);            // 写入配置

    GPIO_EXTILineConfig(IR_PORT_SRC, IR_PIN_SRC);    // 把EXTI0这根线正式接到PB0上

    EXTI_InitStruct.EXTI_Line = IR_LINE;             // 配置0号线
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;// 用“中断”方式，不是只查标志
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  // 下降沿触发：信号由高变低（被遮挡）时才触发一次
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;           // 打开这根中断线
    EXTI_Init(&EXTI_InitStruct);                     // 写入配置

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;    // 0号外部中断的中断号
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;  // 抢占优先级2，比系统节拍(1)低一点
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  // 子优先级0
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;     // 允许CPU响应
    NVIC_Init(&NVIC_InitStruct);                     // 写进NVIC
}

uint16_t IR_GetLap(void)                             // 主循环读圈数用
{
    return ir_lap;                                   // 返回中断里累加的那个数
}

void IR_ResetLap(void)                               // 圈数清零
{
    ir_lap = 0;                                      // 直接写0；进测速菜单时调用，屏幕上就是“圈数:0”
}

void EXTI0_IRQHandler(void)                          // 函数名固定：PB0来一个下降沿，CPU就自动跳到这里
{
    if (EXTI_GetITStatus(IR_LINE) != RESET)          // 确认是0号线触发的
    {
        uint32_t now = Get_Tick();                   // 取当前毫秒数
        if ((now - ir_last) >= 30)                   // 距上次触发超过30ms才算真的过了一圈
        {
            ir_last = now;                           // 记下这次时刻
            ir_lap++;                                // 圈数加1
        }
        EXTI_ClearITPendingBit(IR_LINE);             // 清中断标志；不清会一直重复进中断
    }
}
