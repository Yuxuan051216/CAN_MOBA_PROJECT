# CAN MOBA PC 上位机

## 1. 环境安装

建议 Python 3.10 或更高版本：

```powershell
cd CAN_MOBA_PROJECT\pc_app
python -m pip install -r requirements.txt
```

依赖：

```text
pygame
python-can
```

Mock 模式本身不需要 CAN 硬件，但界面仍需要 pygame。

## 2. 运行 Mock 模式

`config.py` 默认：

```python
CAN_BACKEND = "mock"
```

运行：

```powershell
python main.py
```

默认 Mock 只模拟心跳、状态机和用户主动触发的技能，不自动攻击或自动移动。
只有显式启用演示 AI 才会循环模拟板端行为：

```powershell
$env:CAN_MOBA_DEMO_AI="1"
python main.py
```

界面会明确显示 `MOCK` 或 `REAL CAN`，不能用 Mock 模式代替硬件验收。

Mock 模式内置两个虚拟 CH32V307 节点，能够演示：

- `0x701`、`0x702` 心跳；
- `0x100` 全局状态；
- PC 按键经过技能请求、Master 确认和状态回传；
- 嵌入式英雄死亡与角色互换；
- 10 秒 Cooldown 和 READY；
- 当前 Master 掉线后的接管。
- `0x110` 权威绝对坐标、Master 切换继承坐标；
- 与固件相同的水晶范围攻击、三杀结束和 `0x120` 命中事件。

## 3. 切换 python-can

PowerShell 示例：

```powershell
$env:CAN_MOBA_BACKEND="python-can"
$env:CAN_MOBA_INTERFACE="pcan"
$env:CAN_MOBA_CHANNEL="PCAN_USBBUS1"
$env:CAN_MOBA_BITRATE="500000"
python main.py
```

真实模式启动后，地图底部必须显示：

```text
REAL CAN 模式：技能结果以 Master 回帧为准
```

这组参数适用于常见 PEAK PCAN-USB。其他 USB-CAN 设备请根据 python-can
文档修改 `interface` 和 `channel`，例如 `slcan`、`socketcan`、
`vector` 等。

如果设备只能通过厂商 DLL 或自定义串口协议访问，在
`can_driver.py` 的 `UsbCanPlaceholder` 中实现 `send()` 和 `recv()`，
上层协议、模型和 UI 不需要修改。

## 4. PC 按键

| 按键 | 功能 |
|---|---|
| W/A/S/D | 移动 |
| J | 技能 1，10 伤害，700 地图像素范围，1 s CD |
| K | 技能 2，自身治疗 15，5 s CD |
| L | 技能 3，30 伤害，1400 地图像素范围，10 s CD |
| SPACE | 开始游戏 |
| P | 暂停游戏 |
| R | 重置游戏 |
| F1 | Mock 模式切换板 A 上下线 |
| F2 | Mock 模式切换板 B 上下线 |
| ESC | 退出 |

键盘事件只会发送给当前获得焦点的 pygame 窗口。底部状态栏显示
`键盘已聚焦`；若显示 `请点击窗口激活键盘`，先单击地图区域。

J/K/L 每次按下都会发送 `0x300`，由 Master 决定接受或返回
“游戏未开始 / 冷却中 / 目标超出技能范围”等拒绝结果。超范围不扣血，
也不进入冷却。SPACE/P/R 只有收到 Master 的
`0x100` 状态确认后，UI 才会改变 RUNNING/PAUSED/IDLE 状态。

## 5. 界面说明

- 主区域：横向 1v1 地图，只保留双方水晶，不绘制防御塔。
- 地图英雄：PC 与板子英雄都直接显示在地图上，头顶带名称和 HP 血条。
- 左上/右上：双方简要状态、比分、HP 和当前控制信息。
- 顶部中央：对局状态、Master、Term 和总比分。
- 底部左侧：PC 三个技能、按键、效果和实时冷却遮罩。
- 底部中间：板 A/B 在线状态、角色和死亡冷却。
- 底部右侧：最近 5 条精简 CAN 日志。

技能 1、2、3 分别显示弹道、治疗光环和大范围冲击波；水晶命中显示
光束、帧内实际 `damage`（当前为 `-10`）和受击闪烁。全部正式特效都由
Master 确认帧触发。只有设置
`CAN_MOBA_DEMO_AI=1` 时，Mock 板子英雄才会自动巡航和攻击。

## 6. PC UI 模块

| 文件 | 作用 |
|---|---|
| `main.py` | 主循环入口，保持原有运行方式 |
| `game_model.py` | CAN 协议状态、英雄状态和地图位置 |
| `entities.py` | 英雄实体、HP、坐标、冷却和视觉状态 |
| `game_ui.py` | pygame 窗口与视图组合 |
| `map_view.py` | 地图、水晶、英雄和头顶血条 |
| `hud.py` | 顶部双方状态、比分、技能栏和节点状态 |
| `log_panel.py` | 底部最近 5 条 CAN 日志 |
| `ui_theme.py` | 配色、字体和通用绘制函数 |
| `pc_player.py` | 键盘输入和原有 CAN 发帧逻辑 |
| `can_driver.py` | Mock/python-can 后端，保持原接口 |

完整真实硬件逐帧验收见
`CAN_MOBA_PROJECT/VERIFY_HARDWARE.md`。

## 7. CAN 日志

日志格式：

```text
12:34:56.789 RX ID=0x100 [01 01 02 64 5A 00 00 01]
```

其中：

- `TX`：PC 发出的帧；
- `RX`：USB-CAN 或 Mock 收到的帧；
- ID 后面固定显示 8 字节 DATA。
- `J/K/L` 分别发送 `0x300` 且 `data[1]` 为 `1/2/3`。
- 技能只有收到 Master 的 `0x380+n` 确认后才在 UI 中进入冷却。
- 水晶命中使用 `0x120`，字节为
  `crystal,target,damage,seq,pc_hp,embedded_hp,term,reserved`。
- 权威坐标使用 `0x110`，四个 `int16` 均为小端序、坐标缩放为 1000。
- `0x100` 始终是最终全局状态来源。

PC 仅在相邻位置帧之间进行最多 250 ms 的短预测；收到新 `0x110` 后立即
校正，超时后停止自行积分。任一方比分首次达到 3 时，`0x100` 进入
`GAME_OVER`，顶部显示“PC方胜利”或“板端方胜利”。必须按 R 完成 RESET
后才能再次按 SPACE 开始。

## 8. 验证角色切换

Mock 模式：

1. 按 SPACE 开始。
2. 使用 J/L 攻击嵌入式英雄。
3. HP 归零时日志出现 `0x080`。
4. 随后出现 `0x001`，初始 B=Player 会变成 B=Master、A=Player。
5. B 的英雄状态显示 Cooldown，10 秒后出现 READY。

真实硬件的流程相同，所有显示都来自 CAN 帧。

## 9. 验证 Master 掉线重选

Mock 模式按 F1 让初始 Master A 离线：

1. A 状态变为 OFFLINE。
2. 约 1.5 秒后出现 B 的 Master Claim。
3. 出现 `0x001`，B 变为 Master+Player 降级模式。

真实硬件可直接断开当前 Master 的电源或 CAN 连接。不要断开整个总线的
终端电阻；否则无法区分“节点掉线”和“总线物理故障”。

## 10. 自动测试

不启动 pygame 界面也能检查协议和 Mock 状态机：

```powershell
python -m unittest discover -s tests -v
```

测试覆盖固定帧布局、有符号小端坐标、权威纠漂、技能距离、水晶驻留计时与
阵营、死亡轮换、三杀结束、RESET 重开、Master 超时接管和 UI 离屏渲染。

## 11. 常见问题

- **pygame 未安装**：执行 `python -m pip install pygame`。
- **python-can 未安装**：执行 `python -m pip install python-can`。
- **找不到 PCAN 通道**：确认 PEAK 驱动已安装，并检查通道名。
- **只能接收不能发送**：USB-CAN 可能处于只听模式。
- **心跳全部离线**：检查 500 kbps、终端电阻、CANH/CANL 和共地。
- **中文显示为方框**：确认 Windows 字体目录中存在微软雅黑或黑体。
