# CAN MOBA 项目接口接线说明

本文档只说明 `CAN_MOBA_PROJECT` 的硬件接口和接线。

## 1. 系统接线结构

完整系统包含：

```text
PC + USB-CAN
CH32V307 板子 A + CAN 收发器 A
CH32V307 板子 B + CAN 收发器 B
```

总线结构：

```text
                    CANH
USB-CAN ─────────────┬───────────── CAN 收发器 A
                     └───────────── CAN 收发器 B

                    CANL
USB-CAN ─────────────┬───────────── CAN 收发器 A
                     └───────────── CAN 收发器 B

USB-CAN GND ─────────┬───────────── 板子 A / 收发器 A GND
                     └───────────── 板子 B / 收发器 B GND
```

三组设备必须共地。

## 2. CH32V307 与 CAN 收发器

板子 A 和板子 B 接法相同。

### 2.1 默认 CAN1 引脚：PB8/PB9 重映射

| CH32V307 | 方向 | CAN 收发器 |
|---|---|---|
| PB9 / CAN1_TX | MCU 输出 | TXD |
| PB8 / CAN1_RX | MCU 输入 | RXD |
| GND | 电源地 | GND |
| 3.3 V 或模块要求电源 | 电源 | VCC |

注意：

- `PB9` 接收发器的 `TXD`，不要接 `RXD`。
- `PB8` 接收发器的 `RXD`，不要接 `TXD`。
- PB8/PB9 是 MCU 逻辑电平，不能直接接 CANH/CANL。
- CAN 收发器逻辑侧必须兼容 CH32V307 的 3.3 V GPIO。

固件默认配置：

```c
#define CAN1_USE_REMAP_PB8_PB9  1
```

### 2.2 可选 PA11/PA12

如果需要使用原始 CAN1 引脚，可修改：

```c
#define CAN1_USE_REMAP_PB8_PB9  0
```

此时接线改为：

| CH32V307 | 方向 | CAN 收发器 |
|---|---|---|
| PA12 / CAN1_TX | MCU 输出 | TXD |
| PA11 / CAN1_RX | MCU 输入 | RXD |
| GND | 电源地 | GND |

默认方案中的 PB8/PB9 不得同时连接其他外设。

### 2.3 收发器控制引脚

部分模块带有：

```text
STB
S
RS
EN
SHDN
```

这些引脚必须设置为正常收发模式。具体电平以模块芯片手册为准：

- `STB/S` 常见为低电平进入正常模式；
- `EN` 常见为高电平使能；
- 不要让模块停留在待机、静默或关断模式。

## 3. CAN 总线侧接线

### 3.1 CANH/CANL

| 设备 | 连接目标 |
|---|---|
| USB-CAN CANH | 收发器 A CANH、收发器 B CANH |
| USB-CAN CANL | 收发器 A CANL、收发器 B CANL |
| USB-CAN GND | 板 A GND、板 B GND、两个收发器 GND |

正确原则：

```text
CANH 对 CANH
CANL 对 CANL
GND 全部共地
```

不要接成星形长分支。建议使用一条主干线：

```text
120Ω
 │
USB-CAN ===== 板子 A ===== 板子 B
                            │
                           120Ω
```

节点到主干的支线应尽量短。

### 3.2 终端电阻

CAN 总线物理两端各接一个：

```text
120 Ω，连接在 CANH 与 CANL 之间
```

本项目三节点接法示例：

```text
USB-CAN 端：120 Ω
板子 B 端：120 Ω
板子 A 位于中间：不接终端电阻
```

如果 USB-CAN 内置可开关终端电阻，可直接启用其 120 Ω。

断电后测量 CANH 与 CANL：

```text
约 60 Ω：两个 120 Ω 正常并联
约 120 Ω：只接了一个终端
明显小于 60 Ω：终端电阻过多
无穷大或很大：没有终端或线路断开
```

## 4. 两块板子的节点设置

接线相同，但固件节点编号不同。

| 开发板 | NODE_ID | 初始职责 |
|---|---:|---|
| 板子 A | 1 | Master |
| 板子 B | 2 | Player |

板子 A 使用默认配置。

板子 B 编译时增加：

```text
NODE_ID=2
```

不能给两块板烧录相同的节点编号。

## 5. 技能按键接线

两块板都可以连接三个技能按键。

| 技能 | CH32V307 引脚 | 按键另一端 |
|---|---|---|
| 技能 1 | PB0 | GND |
| 技能 2 | PB1 | GND |
| 技能 3 | PB2 | GND |

单个按键接法：

```text
PB0/PB1/PB2 ───── 按键 ───── GND
```

程序设置为内部上拉输入：

```text
松开：高电平
按下：低电平
```

不需要外部上拉电阻。若线路较长或干扰较大，可增加约 10 kΩ 外部上拉。

## 6. LED 接线

代码默认使用两个低电平点亮的 LED。

| LED | CH32V307 引脚 | 功能 |
|---|---|---|
| LED_RUN | PC0 | 系统运行指示 |
| LED_ROLE | PC1 | Master/Player/Cooldown 角色指示 |

推荐低电平点亮接法：

```text
3.3 V ── 330Ω~1kΩ 电阻 ── LED 正极
LED 负极 ── PC0 或 PC1
```

对应配置：

```c
#define LED_ACTIVE_LOW  1
```

如果使用高电平点亮：

```text
PC0/PC1 ── 330Ω~1kΩ 电阻 ── LED 正极
LED 负极 ── GND
```

同时修改：

```c
#define LED_ACTIVE_LOW  0
```

如果核心板已有板载 LED，应先确认其实际 GPIO，不要直接假设为 PC0/PC1。

## 7. 可选摇杆接线

摇杆默认关闭：

```c
#define USE_JOYSTICK  0
```

启用时修改为：

```c
#define USE_JOYSTICK  1
```

接口：

| 摇杆接口 | CH32V307 |
|---|---|
| VRX | PA0 / ADC1_CH0 |
| VRY | PA1 / ADC1_CH1 |
| VCC | 3.3 V |
| GND | GND |
| SW | 当前版本未使用，可预留普通 GPIO |

重要：

- 摇杆必须使用 3.3 V 供电；
- PA0/PA1 输入电压不能超过 MCU 允许范围；
- 不要用 5 V 给摇杆供电后直接把 VRX/VRY 接入 MCU。

## 8. 串口调试接口

当前 Library 默认 `printf` 使用 UART7：

| CH32V307 | USB 转串口 |
|---|---|
| PE12 / UART7_TX | RXD |
| GND | GND |

串口参数：

```text
115200 baud
8 data bits
1 stop bit
No parity
No flow control
```

当前程序只需要输出调试信息，因此不强制连接 UART RX。

如果使用独立 USB 转串口：

- USB 转串口必须兼容 3.3 V TTL；
- MCU TX 接转换器 RX；
- 两端必须共地；
- 不要接 RS-232 电平接口。

## 9. USB-CAN 设置

PC 软件设置：

```text
波特率：500 kbps
帧类型：标准帧
帧格式：数据帧
工作模式：正常模式
DLC：8
```

不要使用：

```text
扩展帧
远程帧
只听模式
静默模式
```

只听模式不会向板子返回 ACK，板子会报告发送失败。

## 10. 完整接口表

### 板子 A

| 接口 | 引脚 | 连接 |
|---|---|---|
| CAN1_TX | PB9 | 收发器 A TXD |
| CAN1_RX | PB8 | 收发器 A RXD |
| KEY1 | PB0 | 按键到 GND |
| KEY2 | PB1 | 按键到 GND |
| KEY3 | PB2 | 按键到 GND |
| LED_RUN | PC0 | LED 电路 |
| LED_ROLE | PC1 | LED 电路 |
| UART7_TX | PE12 | USB 转串口 RXD |
| GND | GND | 全系统共地 |

### 板子 B

| 接口 | 引脚 | 连接 |
|---|---|---|
| CAN1_TX | PB9 | 收发器 B TXD |
| CAN1_RX | PB8 | 收发器 B RXD |
| KEY1 | PB0 | 按键到 GND |
| KEY2 | PB1 | 按键到 GND |
| KEY3 | PB2 | 按键到 GND |
| LED_RUN | PC0 | LED 电路 |
| LED_ROLE | PC1 | LED 电路 |
| UART7_TX | PE12 | USB 转串口 RXD |
| GND | GND | 全系统共地 |

### CAN 总线

| 设备接口 | 连接 |
|---|---|
| USB-CAN CANH | 收发器 A CANH、收发器 B CANH |
| USB-CAN CANL | 收发器 A CANL、收发器 B CANL |
| USB-CAN GND | 板 A、板 B、收发器 A/B GND |
| 总线首端 | CANH 与 CANL 间 120 Ω |
| 总线末端 | CANH 与 CANL 间 120 Ω |

## 11. 上电前检查

上电前逐项确认：

```text
[ ] 板子 A 烧录 NODE_ID=1
[ ] 板子 B 烧录 NODE_ID=2
[ ] PB9 接收发器 TXD
[ ] PB8 接收发器 RXD
[ ] CANH 没有接反
[ ] CANL 没有接反
[ ] 所有设备 GND 共地
[ ] 收发器逻辑电平兼容 3.3 V
[ ] 收发器处于正常模式
[ ] 总线只有两个 120 Ω 终端电阻
[ ] USB-CAN 设置为 500 kbps 正常模式
[ ] 按键另一端接 GND
[ ] LED 串联限流电阻
[ ] 摇杆使用 3.3 V 供电
```
