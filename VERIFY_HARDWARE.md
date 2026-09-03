# CAN MOBA 真实硬件链路验收

真实硬件验收时，PC 界面必须显示：

```text
REAL CAN 模式：技能结果以 Master 回帧为准
```

如果界面显示 `MOCK`，当前数据不是来自 USB-CAN 硬件。

## 1. 烧录两块板

先生成独立固件：

```powershell
cd CAN_MOBA_PROJECT\firmware_ch32v307
.\build_firmware_variants.ps1
```

分别烧录：

```text
板子 A <- obj/D_CAN_MOBA_A.hex，NODE_ID=1
板子 B <- obj/D_CAN_MOBA_B.hex，NODE_ID=2
```

两块板不能使用相同的 HEX。烧录后串口应分别输出：

```text
Firmware variant=BOARD_A CAN1=PB8_RX/PB9_TX
Firmware variant=BOARD_B CAN1=PB8_RX/PB9_TX
```

默认 `USE_JOYSTICK=1`，启动信息应显示：

```text
Joystick=ON, skill keys PB0/PB1/PB2 enabled
```

## 2. 连接 CAN 总线

MCU 与 CAN 收发器默认使用 `PB8=CAN1_RX`、`PB9=CAN1_TX`。
板子 A、板子 B、USB-CAN 的 `CANH`、`CANL`、`GND` 分别相连。总线物理
两端各安装一个 `120 Ω` 终端电阻，断电测量 CANH 与 CANL 的典型阻值约为
`60 Ω`。

USB-CAN 参数：

```text
500 kbps
11 位标准数据帧
DLC=8
正常模式，不能使用只听模式
```

## 3. 检查心跳

打开 USB-CAN 调试软件，两块板上电后应周期看到：

| CAN ID | 来源 |
|---:|---|
| `0x701` | 板子 A |
| `0x702` | 板子 B |

正常周期应为：

```text
0x700  PC 心跳，约 500 ms
0x701  板子 A 心跳，约 500 ms
0x702  板子 B 心跳，约 500 ms
0x100  当前 Master 全局状态，约 100 ms
0x110  当前 Master 权威坐标，约 100 ms
```

心跳 `data[7]` 是递增序号。若 PCAN-View 中同一个 ID 每 3~10 ms
重复出现，但 `data[7]` 不变，这是 CAN 自动重发，不是应用层新心跳。
应检查 PCAN 是否为正常模式、总线是否有 ACK、CANH/CANL/GND 和两个
120 Ω 终端是否正确；断电测量 CANH-CANL 应约为 60 Ω。

心跳 `data[6]` bit0 表示该固件是否启用了摇杆：

```text
0 = Joystick OFF
1 = Joystick ON
```

PC UI 底部会显示 `Joystick A:OFF/ON B:OFF/ON`。

## 4. 启动真实 CAN 模式

以 PEAK PCAN-USB 为例：

```powershell
cd CAN_MOBA_PROJECT\pc_app
$env:CAN_MOBA_BACKEND="python-can"
$env:CAN_MOBA_INTERFACE="pcan"
$env:CAN_MOBA_CHANNEL="PCAN_USBBUS1"
$env:CAN_MOBA_BITRATE="500000"
python main.py
```

界面必须显示 `REAL CAN` 并看到板 A/B 在线。按 `SPACE` 后应看到 PC 发出
`0x010`，随后收到 Master 的 `0x100`，其中 `data[7]=1` 表示
`RUNNING`。

## 5. 验证 PC 键盘技能

分别按 `J`、`K`、`L`：

| PC 按键 | CAN ID | `data[1]` |
|---|---:|---:|
| J | `0x300` | `1` |
| K | `0x300` | `2` |
| L | `0x300` | `3` |

`data[0]=0` 表示来源是 PC，`data[2]` 是当前
`model.current_player`，不再固定为板 A。

当前 Master 接受技能后会发送：

```text
0x380 + MasterNode  技能执行确认
0x100               最新全局状态
```

PC 技能按钮在收到确认前显示“等待 CAN”。如果 1.2 秒内没有确认，界面会
提示检查真实 CAN 链路，不会假装技能成功。

## 6. 验证板子实体按键

只有当前 Player 板子的技能输入会被 Master 接受。PB0/PB1/PB2 均为内部
上拉，按下时接地。

| 板端按键 | 技能 |
|---|---|
| PB0 | SKILL_1 |
| PB1 | SKILL_2 |
| PB2 | SKILL_3 |

板子 A 应发送：

```text
0x301，data[0]=1，data[1]=1/2/3
```

板子 B 应发送：

```text
0x302，data[0]=2，data[1]=1/2/3
```

板子串口示例：

```text
KEY1 -> SKILL_1 TX
Skill TX node=2 skill=1 target=0 CAN_ID=0x302 result=OK
```

Master 串口示例：

```text
Master RX node=2 skill=1 seq=... pc_hp=100 embedded_hp=100
Node 2 skill=1 pc_hp=90 embedded_hp=100
```

随后总线上应出现 `0x380+n` 和 `0x100`，PC 地图血条必须随状态帧变化。
将双方移动到技能 1 的 700 地图像素范围外时，Master 应返回
`SKILL_RESULT_OUT_OF_RANGE=5`，HP 和冷却不变；技能 3 范围为 1400。

## 7. 排除 Mock 自动演示

默认 Mock 模式只模拟心跳和状态，不自动攻击、不自动移动。只有显式设置：

```powershell
$env:CAN_MOBA_DEMO_AI="1"
python main.py
```

才会启用演示 AI。此时界面明确显示：

```text
MOCK 演示模式：板子攻击/移动为自动模拟
```

该模式只能用于 UI 演示，不能用于硬件链路验收。

## 8. 通过条件

1. 收到 `0x701` 和 `0x702`。
2. PC 按键产生 `0x300`。
3. 板子按键产生 `0x301` 或 `0x302`。
4. Master 返回 `0x380+n` 技能确认。
5. Master 返回 `0x100` 全局状态。
6. PC 地图 HP 只在收到确认/状态帧后变化。
7. 停止按键后没有周期性技能帧或自动扣血。
8. `0x110` 中四个 int16 小端坐标与PC地图一致，切换Master后连续。
9. 进入敌方水晶圆满 1秒才首次扣10，离圈再进入重新计时。
10. PC/板端任一方首次达到3分后，`0x100 data[7]=3`，所有攻击和移动停止。
11. RESET 后比分、HP、冷却、坐标和水晶计时清零，可再次按 SPACE 开始。

## 9. Master / Player 固定规则

PC 永远是电脑玩家，不参与 Master 选举。板子 A/B 在物理连接正常时应
始终同时在线，在线状态与角色状态是两件事：

```text
Player = 场上控制嵌入式英雄，可以发送技能
Master = 场下裁判，处理技能并发送 0x100
```

只有嵌入式 Player 阵亡才交换 A/B 角色：阵亡 Player 转为新 Master，
原 Master 转为新 Player 接管英雄。PC 阵亡只计分和复活，不交换角色。
Reset 清空 HP、分数、技能冷却、胜利、水晶计时、移动和坐标，保持当前
Master/Player 不变并返回 IDLE。
