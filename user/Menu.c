#include "stm32f10x.h"                               // 第一行必须是官方库
#include <stdio.h>                                   // 要用 sprintf 把数字拼成字符串
#include "Delay.h"                                   // 系统节拍：取毫秒数
#include "OLED.h"                                    // 屏幕显示
#include "Key.h"                                     // 按键键值
#include "Buzzer.h"                                  // 蜂鸣器报警
#include "IR.h"                                       // 红外传感器圈数
#include "Menu.h"                                    // 自己的头文件

#define MENU_MAIN   0                                // 状态0：主菜单
#define MENU_SPEED  1                                // 状态1：测速子菜单
#define MENU_HIST   2                                // 状态2：历史数据子菜单
#define MENU_CLOCK  3                                // 状态3：时钟设置子菜单
#define MENU_TEMP   4                                // 状态4：温度监控子菜单
#define MENU_SAVED  5                                // 状态5：显示“保存成功”，过一会儿自动回主菜单

static uint8_t state  = MENU_MAIN;                   // 当前在哪个菜单；static=只在本文件用，外面改不到
static uint8_t cursor = 0;                           // 主菜单光标：0~3，对应4个选项

static uint8_t  run = 0;                             // 1=正在测速，0=停着
static uint16_t speed_x10 = 0;                       // 当前圈速，放大10倍存：20.5存成205（整数好算好显示）
static uint16_t hist[3] = {0, 0, 0};                 // 最近3条历史圈速，同样放大10倍
static uint8_t  hist_cnt = 0;                        // 历史里实际存了几条（最多3条）
static uint8_t  hist_idx = 0;                        // 现在屏幕上显示第几条

static uint8_t  clk_h = 18, clk_m = 30, clk_s = 0;   // 时钟时间：18:30:00
static uint8_t  clk_sel  = 0;                        // 正在选哪个单位：0=时 1=分 2=秒
static uint8_t  clk_edit = 0;                        // 0=只是在选单位，1=已经进入改数值状态
static uint8_t  bak_h = 18, bak_m = 30, bak_s = 0;   // 进菜单时的备份，按取消键就恢复
static uint32_t clk_ms = 0;                          // 上次“加1秒”的时刻

static uint16_t temp_x10 = 255;                      // 当前温度，放大10倍：255就是25.5℃（软件模拟，不接真实传感器）
static uint16_t thr_x10  = 300;                      // 报警阈值，放大10倍：300就是30.0℃
static uint16_t thr_bak  = 300;                      // 阈值备份，按取消键就恢复

static uint32_t last_key_ms  = 0;                    // 上次扫按键的时刻
static uint32_t last_draw_ms = 0;                    // 上次刷屏的时刻
static uint32_t saved_ms     = 0;                    // “保存成功”画面出现的时刻
static uint8_t  buz_on  = 0;                         // 蜂鸣器现在是不是在响：0没响，1在响
static uint8_t  want_buz = 0;                        // 按规则“应该”响不响，用来和 buz_on 比较

static void Show_Line(uint8_t line, char *txt)       // 在第line行(1~4)显示一行文字，不足16个字符自动补空格
{
    char buf[17];                                    // 16个字符 + 1个结束符
    uint8_t i;                                       // 循环变量
    for (i = 0; i < 16; i++)                         // 先把整行填成空格
        buf[i] = ' ';
    buf[16] = '\0';                                  // 字符串结束符
    for (i = 0; i < 16; i++)                         // 再把要显示的文字拷进去
    {
        if (txt[i] == '\0')                          // 文字到头了
            break;                                   // 剩下的保持空格
        buf[i] = txt[i];                             // 一个字符一个字符拷
    }
    OLED_ShowString(line, 1, buf);                   // 整行发到屏幕上；补空格是为了盖掉上一帧的旧字，防止残影
}

static void Draw_Main(void)                          // 画主菜单
{
    Show_Line(1, (cursor == 0) ? ">1.Speed"   : " 1.Speed");     // 光标在第1项就在前面画“>”，否则空一格
    Show_Line(2, (cursor == 1) ? ">2.History" : " 2.History");   // 第2项：历史数据
    Show_Line(3, (cursor == 2) ? ">3.Clock"   : " 3.Clock");     // 第3项：时钟设置
    Show_Line(4, (cursor == 3) ? ">4.Temp"    : " 4.Temp");      // 第4项：温度监控
}

static void Draw_Speed(void)                         // 画测速子菜单
{
    char buf[17];                                    // 拼字符串用的缓冲
    sprintf(buf, "Lap:%d", IR_GetLap());             // 圈数来自红外中断，真实计数
    Show_Line(1, buf);                               // 第1行显示圈数
    sprintf(buf, "Spd:%d.%d m/s", speed_x10 / 10, speed_x10 % 10);  // 整数部分.小数部分
    Show_Line(2, buf);                               // 第2行显示当前圈速
    Show_Line(3, (run == 1) ? "OK:Stop" : "OK:Start");  // 第3行提示确认键现在的作用
    Show_Line(4, "CAN:Back");                        // 第4行提示取消键返回
}

static void Draw_Hist(void)                          // 画历史数据子菜单
{
    char buf[17];                                    // 拼字符串用的缓冲
    if (hist_cnt == 0)                               // 一条记录都没有
    {
        Show_Line(1, "No record");                   // 提示没记录
        Show_Line(2, "");                            // 空行
        Show_Line(3, "");                            // 空行
    }
    else                                             // 有记录
    {
        sprintf(buf, "Rec:%d/%d", hist_idx + 1, hist_cnt);  // 第几条/共几条
        Show_Line(1, buf);                           // 第1行显示序号
        sprintf(buf, "Spd:%d.%d m/s", hist[hist_idx] / 10, hist[hist_idx] % 10);  // 这条记录的圈速
        Show_Line(2, buf);                           // 第2行显示圈速
        Show_Line(3, "OK:Delete");                   // 第3行提示确认键=删除
    }
    Show_Line(4, "UP/DN:Scroll");                    // 第4行提示上下键翻记录
}

static void Draw_Clock(void)                         // 画时钟设置子菜单
{
    char buf[17];                                    // 拼字符串用的缓冲
    uint8_t pos;                                     // 选中的两位数字在字符串里的位置
    uint8_t blink = (Get_Tick() / 300) % 2;          // 每300ms在0和1之间翻一次，用来闪烁
    sprintf(buf, "%02d:%02d:%02d", clk_h, clk_m, clk_s);  // 拼成 18:30:00，%02d表示不足两位前面补0
    if (blink == 0)                                  // 闪烁的“灭”的那半周期
    {
        pos = clk_sel * 3;                           // 时在0、分在3、秒在6（中间隔着冒号）
        buf[pos] = ' ';                              // 把选中的这两位用空格盖掉，看起来就在闪
        buf[pos + 1] = ' ';
    }
    Show_Line(1, (clk_edit == 1) ? "Clock  EDIT" : "Clock");  // 第1行：正在改数值时显示EDIT
    Show_Line(2, buf);                               // 第2行：时间
    Show_Line(3, (clk_edit == 1) ? "OK:Save" : "OK:Edit");    // 第3行：确认键的作用
    Show_Line(4, (clk_edit == 1) ? "UP/DN:Set" : "UP/DN:Sel");// 第4行：上下键的作用
}

static void Draw_Temp(void)                          // 画温度监控子菜单
{
    char buf[17];                                    // 拼字符串用的缓冲
    uint8_t blink = (Get_Tick() / 300) % 2;          // 每300ms翻一次，报警时让温度闪烁
    if ((temp_x10 >= thr_x10) && (blink == 0))       // 已经超温，并且赶上闪烁“灭”的那半周期
        Show_Line(1, "");                            // 第1行空着，看起来温度数字在闪
    else
    {
        sprintf(buf, "Temp:%d.%dC", temp_x10 / 10, temp_x10 % 10);  // 当前温度
        Show_Line(1, buf);                           // 第1行显示当前温度
    }
    sprintf(buf, "ALM :%d.%dC", thr_x10 / 10, thr_x10 % 10);        // 报警阈值
    Show_Line(2, buf);                               // 第2行显示阈值
    Show_Line(3, (temp_x10 >= thr_x10) ? "!!! HIGH !!!" : "Normal");// 第3行：超温就显示感叹号
    Show_Line(4, "OK:Save UP/DN");                   // 第4行提示：确认键保存，上下键调阈值
}

static void Draw_Saved(void)                         // 画“保存成功”提示画面
{
    Show_Line(1, "");                                // 空行
    Show_Line(2, "Saved!");                          // 第2行提示保存成功
    Show_Line(3, "");                                // 空行
    Show_Line(4, "");                                // 空行
}

static void Save_Speed(void)                         // 把当前圈速存进历史，最多留最近3条
{
    if (hist_cnt < 3)                                // 还没存满3条
        hist[hist_cnt++] = speed_x10;                // 直接放在末尾，条数加1
    else                                             // 已经满了
    {
        hist[0] = hist[1];                           // 最老的那条被挤掉，整体往前挪
        hist[1] = hist[2];                           // 第2条变第1条
        hist[2] = speed_x10;                         // 新记录放最后
    }
}

static void Del_Hist(void)                           // 删掉当前正在看的那条记录
{
    uint8_t i;                                       // 循环变量
    if (hist_cnt == 0)                               // 本来就没记录
        return;                                      // 什么都不用做
    for (i = hist_idx; i + 1 < hist_cnt; i++)        // 后面的记录往前挪一格
        hist[i] = hist[i + 1];                       // 一条一条覆盖
    hist_cnt--;                                      // 总条数减1
    if (hist_idx >= hist_cnt && hist_idx > 0)        // 删的是最后一条，序号要往回退
        hist_idx = hist_cnt - 1;
}

static void Clk_Add(uint8_t up)                      // 时钟菜单里按上下键的处理；up=1按下键，up=0按上键
{
    if (clk_edit == 0)                               // 当前只是在选单位
    {
        if (up)                                      // 按下键
            clk_sel = (clk_sel + 1) % 3;             // 时->分->秒->时，循环
        else                                         // 按上键
            clk_sel = (clk_sel == 0) ? 2 : clk_sel - 1;  // 反方向循环
    }
    else                                             // 当前在改具体数值
    {
        if (clk_sel == 0)                            // 改“时”
        {
            if (up) clk_h = (clk_h + 1) % 24;        // 23再加就回0
            else    clk_h = (clk_h == 0) ? 23 : clk_h - 1;
        }
        else if (clk_sel == 1)                       // 改“分”
        {
            if (up) clk_m = (clk_m + 1) % 60;        // 59再加就回0
            else    clk_m = (clk_m == 0) ? 59 : clk_m - 1;
        }
        else                                         // 改“秒”
        {
            if (up) clk_s = (clk_s + 1) % 60;        // 同上
            else    clk_s = (clk_s == 0) ? 59 : clk_s - 1;
        }
    }
}

static void On_Key(uint8_t key)                      // 收到一个键值后，按当前菜单分别处理
{
    switch (state)                                   // 先看现在在哪个菜单
    {
        case MENU_MAIN:                              // ===== 主菜单 =====
            if (key == KEY_UP)                       // 按上键
                cursor = (cursor == 0) ? 3 : cursor - 1;  // 光标往上挪，到顶了跳到最后一项
            if (key == KEY_DOWN)                     // 按下键
                cursor = (cursor + 1) % 4;           // 光标往下挪，到底了回到第一项
            if (key == KEY_OK)                       // 按确认键
            {
                if (cursor == 0) state = MENU_SPEED; // 进测速
                else if (cursor == 1) state = MENU_HIST;  // 进历史
                else if (cursor == 2) state = MENU_CLOCK; // 进时钟
                else state = MENU_TEMP;              // 进温度
                if (state == MENU_SPEED)             // 进测速前先清一下
                {
                    run = 0;                         // 进去是停着的
                    speed_x10 = 0;                   // 速度显示0.0
                    IR_ResetLap();                   // 圈数从0开始
                }
                if (state == MENU_HIST)              // 进历史
                    hist_idx = 0;                    // 从最新那条看起
                if (state == MENU_CLOCK)             // 进时钟
                {
                    clk_sel = 0;                     // 默认选“时”
                    clk_edit = 0;                    // 还没进入修改
                    bak_h = clk_h;                   // 备份时间，取消时恢复
                    bak_m = clk_m;
                    bak_s = clk_s;
                }
                if (state == MENU_TEMP)              // 进温度
                    thr_bak = thr_x10;               // 备份阈值，取消时恢复
                OLED_Clear();                        // 换菜单先清屏，免得旧画面残留在下面
            }
            break;

        case MENU_SPEED:                             // ===== 测速子菜单 =====
            if (key == KEY_OK)                       // 确认键
            {
                if (run == 1)                        // 正在跑
                {
                    run = 0;                         // 停下来
                    Save_Speed();                    // 把这次圈速存进历史
                }
                else
                    run = 1;                         // 没跑就开始跑
            }
            if (key == KEY_CAN)                      // 取消键
            {
                run = 0;                             // 退出前先停下
                state = MENU_MAIN;                   // 回主菜单
                OLED_Clear();                        // 清屏
            }
            break;

        case MENU_HIST:                              // ===== 历史数据子菜单 =====
            if ((key == KEY_UP) && (hist_cnt > 0))   // 按上键并且有记录
                hist_idx = (hist_idx + 1) % hist_cnt;// 往后翻一条，到最后回到第一条
            if ((key == KEY_DOWN) && (hist_cnt > 0)) // 按下键并且有记录
                hist_idx = (hist_idx == 0) ? (hist_cnt - 1) : (hist_idx - 1);  // 往前翻一条
            if (key == KEY_OK)                       // 确认键=删除当前这条
                Del_Hist();                          // 删记录
            if (key == KEY_CAN)                      // 取消键
            {
                state = MENU_MAIN;                   // 回主菜单
                OLED_Clear();                        // 清屏
            }
            break;

        case MENU_CLOCK:                             // ===== 时钟设置子菜单 =====
            if (key == KEY_UP)                       // 上键
                Clk_Add(0);                          // 选单位或者把数值减小
            if (key == KEY_DOWN)                     // 下键
                Clk_Add(1);                          // 选单位或者把数值增大
            if (key == KEY_OK)                       // 确认键
            {
                if (clk_edit == 0)                   // 现在在选单位
                    clk_edit = 1;                    // 进入修改数值状态
                else                                 // 现在在改数值
                {
                    clk_edit = 0;                    // 退出修改
                    state = MENU_SAVED;              // 显示“保存成功”
                    saved_ms = Get_Tick();           // 记下这一刻，700ms后自动回主菜单
                    OLED_Clear();                    // 清屏
                }
            }
            if (key == KEY_CAN)                      // 取消键：放弃这次修改
            {
                clk_h = bak_h;                       // 时恢复成进菜单前的值
                clk_m = bak_m;                       // 分恢复
                clk_s = bak_s;                       // 秒恢复
                clk_edit = 0;                        // 退出修改状态
                state = MENU_MAIN;                   // 回主菜单
                OLED_Clear();                        // 清屏
            }
            break;

        case MENU_TEMP:                              // ===== 温度监控子菜单 =====
            if (key == KEY_UP)                       // 上键：把阈值调大
            {
                if (thr_x10 < 1000)                  // 最大100.0℃
                    thr_x10 += 5;                    // 一次加0.5℃（放大10倍所以加5）
            }
            if (key == KEY_DOWN)                     // 下键：把阈值调小
            {
                if (thr_x10 > 0)                     // 最小0.0℃
                    thr_x10 -= 5;                    // 一次减0.5℃
            }
            if (key == KEY_OK)                       // 确认键：保存阈值
            {
                thr_bak = thr_x10;                   // 把新阈值记成“已保存”的值
                state = MENU_SAVED;                  // 显示“保存成功”
                saved_ms = Get_Tick();               // 记下时刻
                OLED_Clear();                        // 清屏
            }
            if (key == KEY_CAN)                      // 取消键：放弃修改
            {
                thr_x10 = thr_bak;                   // 阈值恢复成进菜单前的值
                state = MENU_MAIN;                   // 回主菜单
                OLED_Clear();                        // 清屏
            }
            break;

        case MENU_SAVED:                             // ===== 保存成功提示画面 =====
            break;                                   // 这个画面不响应按键，等时间到自己回去

        default:                                     // 理论上不会走到这里
            state = MENU_MAIN;                       // 保险起见回主菜单
            break;
    }
}

void Menu_Init(void)                                 // 菜单初始化
{
    state = MENU_MAIN;                               // 上电先进主菜单
    cursor = 0;                                      // 光标停在第1项
    OLED_Clear();                                    // 清一次屏
    Draw_Main();                                     // 立刻把主菜单画出来
    last_key_ms  = Get_Tick();                       // 按键扫描基准时刻
    last_draw_ms = Get_Tick();                       // 刷屏基准时刻
    clk_ms       = Get_Tick();                       // 时钟走秒基准时刻
    saved_ms     = Get_Tick();                       // “保存成功”基准时刻
}

void Menu_Task(void)                                 // 菜单主任务：主循环里一直调用，全部靠时间判断，不卡死
{
    uint32_t now = Get_Tick();                       // 取一次当前毫秒数，后面都用它

    if ((now - last_key_ms) >= 10)                   // 每10ms扫一次按键
    {
        uint8_t key;                                 // 这次扫到的键值
        last_key_ms = now;                           // 更新扫描时刻
        key = Key_Scan();                            // 扫描（函数内部已经消抖、保证按一次只报一次）
        if (key != KEY_NONE)                         // 真有键按下才处理
            On_Key(key);                             // 交给状态机
    }

    if ((state == MENU_CLOCK) && (clk_edit == 1))    // 正在改时间
        clk_ms = now;                                // 把走秒基准一直顶着，等于暂停走秒，免得改数字时它自己乱跳
    else
    {
        while ((now - clk_ms) >= 1000)               // 时钟走秒：每满1000ms加1秒
        {
            clk_ms += 1000;                          // 基准往前推1秒
            clk_s++;                                 // 秒加1
            if (clk_s >= 60)                         // 满60秒
            {
                clk_s = 0;                           // 秒归零
                clk_m++;                             // 分加1
                if (clk_m >= 60)                     // 满60分
                {
                    clk_m = 0;                       // 分归零
                    clk_h++;                         // 时加1
                    if (clk_h >= 24)                 // 满24小时
                        clk_h = 0;                   // 时归零
                }
            }
        }
    }

    if ((state == MENU_SPEED) && (run == 1))         // 正在测速
        speed_x10 = 200 + (uint16_t)((now / 200) % 11);  // 演示用：20.0~21.0之间每200ms跳一下，让人看到“在测”

    if ((state == MENU_TEMP) && (temp_x10 >= thr_x10))  // 在温度菜单并且温度≥阈值
        want_buz = 1;                                // 应该响
    else
        want_buz = 0;                                // 其他情况都不响（离开温度菜单也会停）

    if (want_buz != buz_on)                          // 只在“该响/该停”发生变化时才动引脚，不用每圈都重复设置
    {
        buz_on = want_buz;                           // 记住当前状态
        Buzzer_Set(want_buz);                        // 真正去控制蜂鸣器
    }

    if ((state == MENU_SAVED) && ((now - saved_ms) >= 700))  // 保存提示显示够700ms
    {
        state = MENU_MAIN;                           // 自动回主菜单
        OLED_Clear();                                // 清屏
    }

    if ((now - last_draw_ms) >= 100)                 // 每100ms刷一次屏（不是死等，只是到点才画）
    {
        last_draw_ms = now;                          // 更新刷屏时刻
        switch (state)                               // 按当前菜单画对应的画面
        {
            case MENU_MAIN:  Draw_Main();  break;    // 主菜单
            case MENU_SPEED: Draw_Speed(); break;    // 测速
            case MENU_HIST:  Draw_Hist();  break;    // 历史
            case MENU_CLOCK: Draw_Clock(); break;    // 时钟
            case MENU_TEMP:  Draw_Temp();  break;    // 温度
            case MENU_SAVED: Draw_Saved(); break;    // 保存提示
            default:         break;                  // 其它情况不画
        }
    }
}
