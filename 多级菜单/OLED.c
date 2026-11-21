#include "stm32f10x.h"                                           // 第一行必须是官方库
#include "OLED.h"                                                 // 自己的头文件，里面声明了对外提供的4个函数
#include "OLED_Font.h"                                            // 8x16点阵字库：告诉屏幕每个字符由哪些亮点组成

#define OLED_SCL(x)  GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))   // 时钟线SCL接PB8；写1=拉高，写0=拉低
#define OLED_SDA(x)  GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))   // 数据线SDA接PB9；屏幕只认这一根线上的0和1

static void OLED_I2C_Start(void)                                  // 起始信号：告诉屏幕“我要开始发数据了”
{
    OLED_SDA(1);                                                  // 先把数据线放开（拉高）
    OLED_SCL(1);                                                  // 时钟线拉高
    OLED_SDA(0);                                                  // 时钟还是高的时候把数据线拉低——这一下就叫起始
    OLED_SCL(0);                                                  // 时钟拉低，准备开始发第一位
}

static void OLED_I2C_Stop(void)                                   // 停止信号：告诉屏幕“我发完了”
{
    OLED_SDA(0);                                                  // 先把数据线拉低
    OLED_SCL(1);                                                  // 时钟拉高
    OLED_SDA(1);                                                  // 时钟还是高的时候把数据线拉高——这一下就叫停止
}

static void OLED_I2C_SendByte(uint8_t Byte)                       // 发一个字节（8位）；参数Byte=要发的那个字节
{
    uint8_t i;                                                    // 循环变量，用来数8个二进制位
    for (i = 0; i < 8; i++)                                       // 一位一位往外发
    {
        if (Byte & (0x80 >> i))                                   // 从最高位开始看，这一位是1还是0
            OLED_SDA(1);                                          // 是1就把数据线拉高
        else
            OLED_SDA(0);                                          // 是0就把数据线拉低
        OLED_SCL(1);                                              // 时钟拉高，屏幕就在这一刻把数据读走
        OLED_SCL(0);                                              // 时钟拉低，准备发下一位
    }
    OLED_SCL(1);                                                  // 第9个时钟是“应答位”，这里偷懒不检查屏幕回没回
    OLED_SCL(0);                                                  // 应答时钟拉低，一个字节发送结束
}

static void OLED_WriteCmd(uint8_t Cmd)                            // 发命令：坐标、亮灭、对比度这些都属于命令
{
    OLED_I2C_Start();                                             // 起始
    OLED_I2C_SendByte(0x78);                                      // 0x78是这块屏幕的I2C地址（相当于门牌号）
    OLED_I2C_SendByte(0x00);                                      // 0x00表示“接下来发的是命令”
    OLED_I2C_SendByte(Cmd);                                       // 真正的命令内容
    OLED_I2C_Stop();                                              // 停止
}

static void OLED_WriteData(uint8_t Data)                          // 发数据：要显示的像素点阵都属于数据
{
    OLED_I2C_Start();                                             // 起始
    OLED_I2C_SendByte(0x78);                                      // 屏幕地址
    OLED_I2C_SendByte(0x40);                                      // 0x40表示“接下来发的是显示数据”
    OLED_I2C_SendByte(Data);                                      // 真正的数据
    OLED_I2C_Stop();                                              // 停止
}

static void OLED_SetPos(uint8_t Y, uint8_t X)                     // 把“写字的位置”挪到第Y页(0~7)、第X列(0~127)
{
    OLED_WriteCmd(0xB0 | Y);                                      // 设置页地址（一页 = 8个像素高，4行文字用掉8页）
    OLED_WriteCmd(0x10 | ((X & 0xF0) >> 4));                      // 设置列地址的高4位
    OLED_WriteCmd(0x00 | (X & 0x0F));                             // 设置列地址的低4位
}

void OLED_Clear(void)                                             // 清屏：把128x64个像素全部写成0（黑）
{
    uint8_t page, col;                                            // page=第几页(0~7)，col=第几列(0~127)
    for (page = 0; page < 8; page++)                              // 一页一页来
    {
        OLED_SetPos(page, 0);                                     // 光标挪到这一页开头
        for (col = 0; col < 128; col++)                           // 一列一列写
            OLED_WriteData(0x00);                                 // 写0x00，这一列的8个点全灭
    }
}

void OLED_Init(void)                                              // 上电初始化屏幕：配引脚 + 发厂家规定的一串命令
{
    GPIO_InitTypeDef GPIO_InitStruct;                             // GPIO配置结构体

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);         // 打开GPIOB时钟，不开引脚没反应

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;           // 选中PB8(SCL)和PB9(SDA)
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;                 // 开漏输出：I2C总线必须这么配，才能和其他设备“线与”
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                // 输出速度50MHz，够快
    GPIO_Init(GPIOB, &GPIO_InitStruct);                           // 把配置写进去

    OLED_SCL(1);                                                  // 空闲时两根线都保持高电平
    OLED_SDA(1);                                                  // 这是I2C总线的空闲状态

    OLED_WriteCmd(0xAE);                                          // 关闭显示（初始化期间先别亮，防止花屏）
    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);                     // 设置显示时钟分频比
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);                     // 设置驱动路数（1/64）
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);                     // 显示偏移设为0
    OLED_WriteCmd(0x40);                                          // 起始行从第0行开始
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);                     // 打开电荷泵（屏幕能亮的关键开关）
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x00);                     // 内存寻址模式：水平寻址
    OLED_WriteCmd(0xA1);                                          // 左右方向：不镜像
    OLED_WriteCmd(0xC8);                                          // 上下方向：不翻转
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);                     // COM引脚硬件配置
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);                     // 对比度（亮度），0xCF比较亮
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);                     // 预充电周期
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x40);                     // VCOMH电压
    OLED_WriteCmd(0xA4);                                          // 显示跟随RAM内容（不是全亮）
    OLED_WriteCmd(0xA6);                                          // 正常显示（白字黑底，不反色）
    OLED_WriteCmd(0xAF);                                          // 打开显示，屏幕亮起来

    OLED_Clear();                                                 // 清一次屏，保证上电是干净的
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)       // 显示一个字符：Line行(1~4)、Column列(1~16)
{
    uint8_t i;                                                    // 循环变量，一个字符点阵是16个字节
    OLED_SetPos((Line - 1) * 2, (Column - 1) * 8);                // 每个字符占2页高、8列宽，先定位到上半部分
    for (i = 0; i < 8; i++)                                       // 上半部分8个字节
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);                // 字库里从空格开始排，所以减掉空格的ASCII码就是下标
    OLED_SetPos((Line - 1) * 2 + 1, (Column - 1) * 8);            // 再定位到下半部分
    for (i = 0; i < 8; i++)                                       // 下半部分8个字节
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);            // 下标从8开始
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)  // 显示一串字符：从Line行Column列开始往右排
{
    uint8_t i = 0;                                                // 数到第几个字符
    while (String[i] != '\0')                                     // 一直显示到字符串结束符为止
    {
        OLED_ShowChar(Line, Column + i, String[i]);               // 一个字符一个字符画上去
        i++;                                                      // 换下一个字符
    }
}
