# CH32V307 CAN MOBA 固件

## 1. 工程说明

本目录是完整 MOBA 项目的统一双板固件，基于已有
`Dmx_CH32V307VCT6_Library_V1.0.0` 工程和已经验证通过的
`D_CAN_PC_TEST` 通信层整理。保留 WCH 标准外设库、启动文件、链接脚本和
MounRiver 工程配置，不使用 HAL、RTOS 或动态内存。

两块 CH32V307 使用同一套源码，只通过 `NODE_ID` 区分：

```c
NODE_ID=1  // 板子 A，初始 Master
NODE_ID=2  // 板子 B，初始 Player
```

## 2. 目录结构

```text
firmware_ch32v307/
├── App/
│   ├── app_config.h
│   ├── build_variant.h
│   ├── game_types.h
│   ├── game_common.h/c
│   ├── game_protocol.h/c
│   ├── node_role.h/c
│   ├── game_master.h/c
│   └── game_player.h/c
├── BSP/
│   ├── bsp_can.h/c
│   ├── bsp_key.h/c
│   ├── bsp_joystick.h/c
│   ├── bsp_led.h/c
│   └── bsp_time.h/c
├── User/
│   ├── main.c
│   ├── ch32v30x_conf.h
│   ├── isr.c/h
│   └── system_ch32v30x.c/h
└── QhLibraries/
    └── Peripheral/src/ch32v30x_can.c
```

`.cproject` 已包含 `App` 和 `BSP` 头文件路径，项目会递归编译这些目录。
`ch32v30x_can.c` 已在标准外设库源码目录中。

## 3. 一次生成板子 A/B 固件

在 PowerShell 中运行：

```powershell
cd CAN_MOBA_PROJECT\firmware_ch32v307
.\build_firmware_variants.ps1
```

脚本会分别 Clean Build 两次，并生成：

```text
obj/D_CAN_MOBA_A.hex  // NODE_ID=1，初始 Master
obj/D_CAN_MOBA_B.hex  // NODE_ID=2，初始 Player
```

脚本会校验两份 HEX 的 SHA-256 不同，完成后将
`App/build_variant.h` 和通用 `obj/D_CAN_MOBA.hex` 恢复为板子 A。

普通 MounRiver Studio Build 默认仍生成板子 A。不要把同一个 HEX 同时烧入
两块板，否则两块板会使用相同节点 ID。

## 4. CAN 参数与接线

固定参数：

```text
CAN1
11 位标准数据帧
DLC = 8
正常模式
500 kbps
过滤器全部接收
```

当前工程 SYSCLK=144 MHz，PCLK1=72 MHz：

```text
Prescaler=9, SJW=1 tq, BS1=12 tq, BS2=3 tq
72 MHz / 9 / (1 + 12 + 3) = 500 kbps
```

默认板端接线：

| CH32V307 | CAN 转换器/收发器 |
|---|---|
| PB9 / CAN1_TX | TXD |
| PB8 / CAN1_RX | RXD |
| GND | GND |
| 3.3 V 或模块要求电源 | VCC |

总线连接：

```text
板 A 收发器 CANH ─┬─ 板 B 收发器 CANH ─ USB-CAN CANH
板 A 收发器 CANL ─┴─ 板 B 收发器 CANL ─ USB-CAN CANL
三端 GND 必须共地
```

总线物理两端各接一个 120 Ω 终端电阻。断电测量 CANH 与 CANL，通常约为
60 Ω。不要把 PB8/PB9 直接接到 CANH/CANL。

当前默认配置是：

```c
#define CAN1_USE_REMAP_PB8_PB9 1
```

如确实需要恢复 PA11/PA12，将该值改为 `0`。

## 5. 按键、LED 和摇杆

技能按键均为内部上拉、按下接地：

| 引脚 | 功能 |
|---|---|
| PB0 | 技能 1，伤害 10，范围 700 地图像素，CD 1 s |
| PB1 | 技能 2，治疗 15，CD 5 s |
| PB2 | 技能 3，伤害 30，范围 1400 地图像素，CD 10 s |

默认 LED：

| 引脚 | 功能 |
|---|---|
| PC0 | RUN，500 ms 翻转 |
| PC1 | 角色指示 |

`PC1`：Master 慢闪、Player 快闪、Cooldown 双闪。默认低电平点亮；若板子
电路不同，修改 `LED_ACTIVE_LOW` 和对应端口宏。

摇杆默认启用：

```c
#define USE_JOYSTICK 1
```

默认配置同时启用 PB0/PB1/PB2 三个技能按键和摇杆移动。

只有 UI 中标记为 `Player` 的板子能控制场上英雄，`Master` 板子的
按键按下后只输出忽略原因。PB0/PB1/PB2 是内部上拉输入，必须由外部
按键短接到 GND；它们不等同于开发板上任意一个默认用户按键。

当前 Player 按键后会始终发送 `0x301` 或 `0x302`。即使游戏尚未
START，Master 也会返回明确的拒绝结果，不再在 Player 本地静默丢弃。
如需关闭板子摇杆移动，改成：

```c
#define USE_JOYSTICK 0
```

摇杆使用 PA0/ADC1_CH0 和 PA1/ADC1_CH1。方向编码在 CAN 中为
0=负方向、1=停止、2=正方向。

板子会通过心跳 `data[6]` bit0 上报摇杆编译开关，PC UI 底部显示
`Joystick A:OFF/ON B:OFF/ON`。

## 6. 串口调试

波特率为 115200。当前 Library 的 `printf` 默认使用 UART7_TX=PE12。
如果你的板载串口接在其他 UART，修改 `QhLibraries/Debug/debug.h` 中的
`DEBUG`。

启动信息：

```text
CAN MOBA Node Start
NODE_ID=1
Initial Master=1 Player=2 Term=1
```

`CAN_DEBUG_PRINT=1` 会打印每一帧。正式演示时若日志过多，可设为 0。

## 7. 游戏状态机

初始状态：

```text
A = Master
B = Player
PC = 玩家 0
Game = IDLE
```

PC 发送 `0x010`、命令 1 后游戏开始。

Master 每 50 ms 执行裁判，每 100 ms 广播 `0x100` 和 `0x110`。只有
Master 积分移动并判定技能、水晶、HP、比分和胜负；备用板只保存最近收到的
四个权威坐标。Master 切换不会恢复出生点。

技能 1/3 分别使用 700/1400 地图像素范围，技能 2 为自身治疗。攻击技能
超范围返回 `SKILL_RESULT_OUT_OF_RANGE`，不扣血、不进入冷却。

蓝水晶只攻击板端英雄，红水晶只攻击 PC 英雄。判定半径与 PC UI 的 234
地图像素圆完全一致；连续停留 1000 ms 后首次造成 10 点伤害，离圈重新计时。

嵌入式英雄死亡时：

```text
old Player -> new Master，英雄进入 10 s Cooldown
old Master -> new Player，立即接管嵌入式英雄
term + 1
```

角色与英雄状态分开保存，因此“新 Master 的英雄仍处于 Cooldown”是合法状态。
冷却结束发送 `0x400 + NodeID` READY，但不会自动抢占当前 Player。

任一方比分首次达到 `WIN_SCORE=3` 后，当前 Master 立即广播最终
`GAME_OVER`。此后移动、技能、水晶和重复计分全部停止。RESET 清空比分、
HP、技能冷却、胜利标志、水晶计时、移动和坐标，返回 `GAME_IDLE`。

## 8. Master 掉线重选

节点超过 1500 ms 没收到当前 Master 心跳或全局状态后：

1. 发送 `0x020 + NodeID` Master Claim。
2. 等待 120 ms 仲裁窗口。
3. 优先级数值小、NodeID 小的节点胜出。
4. 新 Master 广播 `0x001`。

只有一块板存活时，它进入 `ROLE_MASTER_PLAYER`，同时承担裁判和输入职责。

## 9. 固定协议

| ID | 功能 |
|---:|---|
| `0x001` | 角色切换 |
| `0x010` | 游戏开始/暂停/重置 |
| `0x020+n` | Master Claim |
| `0x080` | 死亡事件 |
| `0x100` | 全局状态 |
| `0x110` | 权威绝对坐标，4 个 int16 小端序 |
| `0x120` | 水晶命中结果 |
| `0x180+n` | 英雄状态 |
| `0x200+n` | 移动输入 |
| `0x300+n` | 技能输入 |
| `0x380+n` | Master 技能执行确认 |
| `0x400+n` | READY |
| `0x700+n` | 心跳 |

详细 DATA 字节定义见 `App/game_protocol.c` 和 PC 端 `protocol.py`。

## 10. 烧录与验收顺序

1. 只接板 A 和 USB-CAN，确认收到 `0x701`。
2. 只接板 B，确认收到 `0x702`。
3. 两板同时在线，确认 A 心跳角色为 Master，B 为 Player。
4. PC 按 SPACE，确认 `0x100` 中游戏状态为 1。
5. 观察当前 Master 每约 100 ms 发送 `0x110`。
6. PC 按 J/K/L，在范围内命中、范围外返回结果 5。
7. B 按 PB0/PB1/PB2，观察 PC HP。
8. 击杀嵌入式英雄，确认 B=Master、A=Player 且坐标不回出生点。
9. 任一方达到 3 分，确认 `0x100 data[7]=3`。
10. RESET 后确认回到 IDLE，并可再次 START。

建议同时观察原始 CAN 帧：

- PC 技能：`0x300`，`data[1]=1/2/3`。
- 板 A 技能：`0x301`，`data[1]=1/2/3`。
- 板 B 技能：`0x302`，`data[1]=1/2/3`。
- Master 接受后：`0x380+n` 确认和 `0x100` 全局状态。

完整步骤见 `../VERIFY_HARDWARE.md`。

## 11. 常见问题

- **收不到 CAN 帧**：检查收发器供电、STB/EN、共地、500 kbps 和标准帧。
- **CANH/CANL 接反**：CANH 对 CANH，CANL 对 CANL。
- **没有 ACK**：USB-CAN 不得处于只听模式。
- **没有终端电阻**：总线两端各 120 Ω。
- **终端过多**：断电测得明显低于 60 Ω 时检查多余电阻。
- **节点 ID 配错**：总线上不能同时存在两块 `NODE_ID=1` 或两块 `NODE_ID=2`。
- **角色不切换**：确认游戏已开始，并检查 `0x080`、`0x001`、`0x100` 是否出现。
- **串口没有输出**：确认 Library 默认 UART7_TX=PE12 是否与板载串口一致。
