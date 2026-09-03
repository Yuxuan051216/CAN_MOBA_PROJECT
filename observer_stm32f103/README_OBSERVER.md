# STM32F103C8T6 CAN 监听声光节点

本工程是可独立编译和烧录的 STM32F103C8T6 工程。它只接收当前
Master 广播的游戏结果，将事件转换为 DFPlayer 语音、WS2812 灯效、
有源蜂鸣器提示和 OLED 状态显示。

STM32F103 不检查技能合法性，不计算伤害或治疗，不维护英雄位置，
不判断水晶范围，不发起角色切换，也不判定胜负。胜利语音只根据
`GLOBAL_STATE` 中已经广播的 `GAME_OVER` 和最终权威比分触发。

## 目录

```text
observer_stm32f103/
├── Core/Inc/                 应用接口和配置
├── Core/Src/                 Observer、队列和外设状态机
├── Drivers/CMSIS/            F103C8 启动和寄存器定义
├── Drivers/STM32F1xx_HAL_Driver/
│                              本工程所需的轻量 HAL 兼容子集
├── Tests/                    主机侧事件和隔离测试
├── observer_stm32f103.ioc    CubeMX/CubeIDE 引脚与时钟配置
├── STM32F103C8TX_FLASH.ld    64 KiB Flash / 20 KiB RAM
├── Makefile
└── build_firmware.ps1
```

`Drivers/STM32F1xx_HAL_Driver` 只实现本项目实际使用的 GPIO、CAN、
UART、NVIC 和时基 HAL 接口，不是完整 STM32CubeF1 驱动包。工程已经
使用 GNU Arm Embedded 9.3.1 实际链接；若后续需要完整 Cube 代码生成，
可按 `observer_stm32f103.ioc` 换入完整 STM32CubeF1 驱动。

## 构建

当前机器可直接运行：

```powershell
cd observer_stm32f103
.\build_firmware.ps1
```

也可手动构建：

```powershell
& "E:\MounRiver_Studio\toolchain\Build Tools\bin\make.exe" -j4
```

输出：

```text
build/observer_stm32f103.elf
build/observer_stm32f103.hex
build/observer_stm32f103.bin
build/observer_stm32f103.map
```

本次验证结果：

```text
FLASH: 7056 B / 64 KiB
RAM:   3648 B / 20 KiB
```

STM32CubeIDE 中使用 `File -> Import -> Existing Projects into Workspace`
导入本目录，然后执行 `Build Project`。项目采用外部 Makefile。

## 硬件配置

```text
HSE       8 MHz
SYSCLK   72 MHz
APB1     36 MHz
CAN      500 kbit/s, Normal, 11-bit standard, DLC=8
CAN TQ   36 MHz / 4 / (1 + 13 + 4) = 500 kbit/s
USART1   9600 8N1, DFPlayer
USART2   115200 8N1, reserved debug port
```

CAN 使用 PA11/PA12 默认映射。硬件过滤器只接收：

```text
0x001  0x010  0x080  0x100  0x110  0x120  0x381  0x382
```

## 非阻塞结构

CAN FIFO0 中断只执行：

1. 读取一帧。
2. 检查标准数据帧和 DLC=8。
3. 检查监听 ID。
4. 压入固定长度队列。

主循环持续执行：

```c
Observer_ProcessEvents();
VoiceQueue_Update();
RgbEffect_Update();
Buzzer_Update();
StatusDisplay_Update();
```

DFPlayer 通过 USART1 TX 中断发送 10 字节命令；WS2812 通过
TIM4_CH1 + DMA1 Channel 1 输出；OLED 每次循环只推进一个短 I2C
事务。CAN 中断中没有语音、动画、显示刷新或延时。

## 事件行为

- 首个有效 `DEATH_EVENT`：依次排队“第一滴血”和阵亡方语音。
- `CRYSTAL_ATTACK`：蓝白或红白灯效、短蜂鸣、低优先级提示语音。
- `ROLE_SWITCH`：播报节点 A/B 接管 Master，并显示新 Master/Player。
- `SKILL_RESULT`：只有 `SKILL_RESULT_ACCEPTED` 才触发技能灯效。
- `GLOBAL_STATE`：更新 HP、比分、Master、Player 和游戏状态。
- `POSITION_STATE`：仅接收并忽略，不参与坐标或范围计算。
- `GAME_OVER` 且最终比分首次达到 3：播放 PC 方或板端方胜利语音。
- `GAME_CTRL_RESET`：清空旧语音、临时灯效和事件指纹。

语音优先级：

```text
胜利 > 第一滴血/阵亡 > Master 切换 > 游戏状态 > 水晶提示
```

TF 卡使用 FAT32，并建立：

```text
MP3/0001.mp3  欢迎来到对战现场
MP3/0002.mp3  游戏开始
MP3/0003.mp3  第一滴血
MP3/0004.mp3  PC英雄已被击败
MP3/0005.mp3  板端英雄已被击败
MP3/0006.mp3  水晶正在攻击
MP3/0007.mp3  节点A已接管Master
MP3/0008.mp3  节点B已接管Master
MP3/0009.mp3  PC方胜利
MP3/0010.mp3  板端方胜利
MP3/0011.mp3  游戏暂停
MP3/0012.mp3  游戏重置
```

## ST-Link 烧录

连接 `SWDIO`、`SWCLK`、`GND` 和 `3.3V` 参考电压。在 STM32CubeProgrammer
中选择 ST-Link、SWD，连接后烧录：

```text
build/observer_stm32f103.hex
```

命令行示例：

```powershell
STM32_Programmer_CLI -c port=SWD -w build\observer_stm32f103.hex -v -rst
```

## 隔离证明

- `can_receiver.c` 只有过滤、启动、接收和队列写入，没有 CAN TX API。
- `can_moba_protocol.h/.c` 只镜像被监听的 ID 和枚举，没有发送函数。
- `observer_controller.c` 只处理事件、term、指纹、显示和播报状态。
- `DEVICE_ID_OBSERVER=3` 只位于本工程配置，未进入原工程的
  `NODE_COUNT=3`。
- 本工程没有心跳、Master Claim、Ready、角色候选列表或游戏计算模块。

Observer 与 Board A/B 共用相同协议 ID，但仍与 Master/Player、位置计算、
伤害计算和胜负状态机完全隔离。

## 数据链

```text
玩家输入
→ 当前Master接收
→ 当前Master进行裁决
→ 当前Master更新HP/比分/角色
→ 当前Master广播CAN事件
→ STM32F103被动监听
→ STM32F103播放语音、灯光、蜂鸣器并显示状态
```

## 验证边界

已完成交叉编译、主机侧事件测试、协议一致性检查和原有仲裁回归测试。
尚未在真实 STM32F103C8T6、SN65HVD230、DFPlayer、WS2812、蜂鸣器和
OLED 上进行电气与时序实测。首次上板应重点验证 HSE 起振、CAN ACK/
过滤器、WS2812 波形、OLED 地址以及 DFPlayer 音频时长对应的命令间隔。
