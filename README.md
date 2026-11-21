# STM32 多级菜单交互（STM32F103C8T6）

基于 **STM32F103C8T6 + Keil MDK（标准外设库）** 的多级菜单交互程序：OLED 显示四级菜单，4 个按键操作，蜂鸣器按键/报警提示，红外传感器计圈。

## 功能菜单

| 菜单项 | 功能 |
| --- | --- |
| `1.Speed` | 测速：显示圈数与当前圈速，确认键启停，停止时把成绩存入历史 |
| `2.History` | 历史数据：上下键在最近 3 条记录间循环查看，确认键删除当前这条 |
| `3.Clock` | 时钟：上下键选时/分/秒，确认键进入修改，修改后保存并返回 |
| `4.Temp` | 温度监控：上下键调报警阈值，温度 ≥ 阈值时蜂鸣器报警，确认键保存阈值 |

按键：`UP`（上）、`DOWN`（下）、`OK`（确认）、`CANC`（取消）。

## 硬件连接

| 外设 | 引脚 | 说明 |
| --- | --- | --- |
| OLED | SCL=PB8，SDA=PB9 | 4 针 I2C 屏 |
| 上键 | PA0 | 上拉输入，按下为低电平 |
| 下键 | PA2 | 上拉输入，按下为低电平 |
| 确认键 | PA4 | 上拉输入，按下为低电平 |
| 取消键 | PA6 | 上拉输入，按下为低电平 |
| 蜂鸣器 | PA8 | 推挽输出，**低电平触发**（输出 0 响、输出 1 停） |
| 红外传感器 | PB0 | 外部中断 EXTI0，下降沿触发，用于计圈 |
| 系统节拍 | TIM4 | 预分频 72-1、重装载 1000-1，每 1 ms 中断一次 |

## 目录结构

```
.
├── project.uvprojx        # Keil 工程文件（用 Keil5 打开）
├── quest.md               # 课程题目要求（文档，不参与编译）
├── user/                  # 应用层：main.c、Menu.c/h（菜单状态机）
├── hardware/              # 硬件驱动：OLED、Key、Buzzer、IR
├── system/                # Delay 延时（TIM4 节拍）
├── library/               # STM32F10x 标准外设库
└── start/                 # 启动文件与 CMSIS 内核文件
```

## 编译与下载

1. 用 **Keil MDK 5** 打开 `project.uvprojx`
2. `F7` 编译（Build）
3. `F8` 下载到开发板（需先装好 ST-Link / DAP 驱动）

## 仓库说明

- 只提交源码与工程文件；`Objects/`、`Listings/`、`*.axf`、`*.uvguix.*` 等编译产物由 `.gitignore` 排除。
- `quest.md` 是题目要求文档（内容是中文说明，不是 C 代码），放在这里方便对照需求。
- 以下杂项**未纳入版本管理**：`prject`、`project`（`Sensor.c`/`Sensor.h` 丢失扩展名后的残留）、`project_fixed_backup.*`（工程备份）、`hardware/Menu.c`、`hardware/Menu.h`（与 `user/Menu.c` 内容完全重复，且未被 Keil 工程引用）。
