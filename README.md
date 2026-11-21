# STM32 多级菜单交互（STM32F103C8T6）

基于 **STM32F103C8T6 + Keil MDK（标准外设库）** 的多级菜单交互示例：OLED 屏幕显示四级菜单，4 个按键操作，蜂鸣器按键提示，红外传感器计圈。

## 功能菜单

| 菜单项 | 功能 |
| --- | --- |
| `1.Speed` | 显示红外圈数 / 速度，OK 键启停 |
| `2.History` | 历史记录 |
| `3.Clock` | 时钟显示 |
| `4.Temp` | 温度显示 |

按键：`UP`（上）、`DOWN`（下）、`OK`（确认）、`BACK`（返回/取消）。

## 硬件连接

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| OLED | SCL=PB8，SDA=PB9 | 4 针 I2C 屏 |
| 按键 | PA0 ~ PA3 | 上拉输入，分别为上/下/确认/取消 |
| 蜂鸣器 | PA4 | 有源蜂鸣器，高电平响 |
| 红外传感器 | PB0 | 外部中断，用于计圈 |
| 系统节拍 | TIM4 | 每 1 ms 中断一次，供延时/消抖/刷屏使用 |

## 目录结构

```
.
├── project.uvprojx        # Keil 工程文件（用 Keil5 打开）
├── user/                  # 应用层：main.c、Menu.c/h（菜单状态机）
├── hardware/              # 硬件驱动：OLED、Key、Buzzer、IR、Track、Usart1
├── system/                # Delay 延时（TIM4 节拍）
├── library/               # STM32F10x 标准外设库
├── start/                 # 启动文件与 CMSIS 内核文件
└── 多级菜单/               # 菜单功能的单文件版本（学习/对照用）
```

## 编译与下载

1. 用 **Keil MDK 5** 打开 `project.uvprojx`
2. `F7` 编译（Build）
3. `F8` 下载到开发板（需先装好 ST-Link / DAP 驱动）

## 说明

- 仓库只提交源码与工程文件，`Objects/`、`Listings/` 等编译产物已通过 `.gitignore` 排除。
- 根目录下 `prject`、`project` 两个无扩展名文件是 `Sensor.c` / `Sensor.h` 丢失扩展名后的残留，未参与 Keil 工程编译，仅作备份保留。
