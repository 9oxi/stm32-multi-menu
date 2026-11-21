#include "stm32f10x.h"                                          // 第一行必须是官方库：uint32_t、TIM、NVIC 都在里面
#include "Delay.h"                                              // 自己的头文件，里面声明了下面这三个函数

volatile uint32_t sys_ms = 0;                                   // 开机到现在的毫秒数；volatile 告诉编译器“中断里会改它”，主循环每次都老实去内存读新值

void Delay_Init(void)                                           // 初始化系统节拍：把TIM4配成每1ms中断一次
{
    TIM_TimeBaseInitTypeDef TIM_InitStruct;                     // 定时器参数结构体，用来装预分频、重装值这些设置
    NVIC_InitTypeDef NVIC_InitStruct;                           // 中断参数结构体，用来装优先级、开不开中断

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);        // 打开TIM4的时钟；TIM4挂在APB1总线上，不开时钟它不数数

    TIM_InitStruct.TIM_Prescaler = 72 - 1;                      // 预分频：72MHz先除以72，得到1MHz，也就是1微秒数一次
    TIM_InitStruct.TIM_Period = 1000 - 1;                       // 重装载值：数满1000次正好1毫秒，然后产生一次更新中断
    TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;            // 时钟分频，这里不分频（数字滤波才用得到）
    TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;        // 向上计数：0、1、2……999，再回到0从头数
    TIM_InitStruct.TIM_RepetitionCounter = 0;                   // 重复计数是高级定时器(TIM1/TIM8)的功能，普通定时器填0
    TIM_TimeBaseInit(TIM4, &TIM_InitStruct);                    // 把上面这些参数写进TIM4的寄存器

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);                       // 先清一次更新标志，防止一开中断就误进一次
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);                  // 允许“数满1ms”这个事件产生中断
    TIM_Cmd(TIM4, ENABLE);                                      // 启动TIM4，开始数数

    NVIC_InitStruct.NVIC_IRQChannel = TIM4_IRQn;                // 选TIM4的中断号
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;      // 抢占优先级1，数字越小越优先
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;             // 子优先级1
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;                // 允许CPU响应这个中断
    NVIC_Init(&NVIC_InitStruct);                                // 把中断配置写进NVIC（中断控制器）
}

uint32_t Get_Tick(void)                                         // 读当前毫秒数：主循环、按键消抖、时钟计时都靠它
{
    return sys_ms;                                              // 直接返回全局变量，它由TIM4中断在后台自动加1
}

void Delay_ms(uint32_t ms)                                      // 阻塞式延时：站在这里等到毫秒数走够为止
{
    uint32_t start = sys_ms;                                    // 记下开始时刻
    while ((Get_Tick() - start) < ms);                          // 用无符号减法比较，sys_ms溢出（约49天）也不会算错
}

void TIM4_IRQHandler(void)                                      // 函数名固定：TIM4数满1ms，CPU就自动跳到这里
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)          // 确认真的是“更新”这个中断触发的
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);             // 清掉中断标志；不清的话会一直重复进中断
        sys_ms++;                                               // 毫秒数加1，这就是整个系统的心跳
    }
}
