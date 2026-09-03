# STM32F103 CAN 监听声光节点接线

所有模块必须共地。接线和断电阻值确认完成后再上电。

## CAN

| STM32F103 | SN65HVD230 | 说明 |
|---|---|---|
| PA12 | TXD | bxCAN TX |
| PA11 | RXD | bxCAN RX |
| 3.3V | VCC | 收发器使用 3.3V |
| GND | GND | 本地共地 |

| SN65HVD230 | 系统总线 |
|---|---|
| CANH | 公共 CANH |
| CANL | 公共 CANL |
| GND | 全系统公共 GND |

新增节点不增加第三个 `120 ohm` 终端电阻。整条总线仍只在物理两端各放
一个 `120 ohm`。断电测量 CANH 与 CANL 应约为 `60 ohm`。

## DFPlayer Mini

| STM32F103/电源 | DFPlayer |
|---|---|
| PA9 USART1_TX | 经过 1 kohm 串联电阻到 RX |
| PA10 USART1_RX | TX |
| 5V | VCC |
| GND | GND |
| 扬声器 | SPK1/SPK2，4 ohm 3 W |

DFPlayer 电源建议使用独立稳定的 5V 支路，并在模块附近放置
`100 uF + 100 nF` 去耦。STM32 与 DFPlayer 必须共地。

## 灯、蜂鸣器和显示

| STM32F103 | 外设 | 说明 |
|---|---|---|
| PC13 | 运行 LED | Blue Pill 板载 LED，低电平点亮 |
| PB0 | 有源蜂鸣器输入 | 高电平响；电流较大时加三极管 |
| PB6 | WS2812B DIN | 建议串联 330 ohm |
| PB8 | SSD1306 SCL | 软件 I2C，需上拉到 3.3V |
| PB9 | SSD1306 SDA | 软件 I2C，需上拉到 3.3V |

WS2812B 通常使用 5V。数据线建议通过 `74AHCT125/74HCT14` 做 3.3V 到
5V 电平转换，并在灯带电源入口放置至少 `470 uF` 电容。不要从 Blue Pill
的 3.3V 稳压器给多颗 WS2812 供电。

OLED 默认地址为 `0x3C`。没有 OLED 时，将
`Core/Inc/observer_config.h` 中 `OBSERVER_USE_OLED` 改为 `0U`。

## 调试串口

| STM32F103 | USB-TTL |
|---|---|
| PA2 USART2_TX | RX |
| PA3 USART2_RX | TX |
| GND | GND |

参数为 `115200, 8N1`。当前固件保留该端口，未在中断中输出日志。

## ST-Link

| ST-Link | STM32F103 |
|---|---|
| SWDIO | PA13 |
| SWCLK | PA14 |
| GND | GND |
| 3.3V reference | 3.3V |

烧录时可由 ST-Link 提供参考电压；若整机已供电，避免两路电源互相反灌。

## 上电检查

1. 断电测 CANH-CANL 约 `60 ohm`。
2. 确认 SN65HVD230 使用 3.3V，DFPlayer/WS2812 使用合适的 5V。
3. 确认 CANH、CANL 没有接反，所有节点共地。
4. 先只连接 ST-Link，烧录并确认 PC13 运行 LED 翻转。
5. 再连接 CAN，确认能收到七类目标帧且总线上没有 Observer 应用层帧。
6. 最后逐项接入 DFPlayer、蜂鸣器、WS2812 和 OLED。
