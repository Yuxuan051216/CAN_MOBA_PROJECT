# CAN_MOBA_PROJECT

完整课程设计项目由两个部分组成：

```text
CAN_MOBA_PROJECT/
├── firmware_ch32v307/  # 两块 CH32V307 共用的裸机固件
└── pc_app/             # PC 玩家、CAN 适配层、Mock 仿真和 pygame 显示
```

推荐先阅读：

- `firmware_ch32v307/README_FIRMWARE.md`
- `pc_app/README_PC.md`
- `WIRING_INTERFACES.md`（只包含硬件接口与接线）
- `VERIFY_HARDWARE.md`（真实 CAN、按键和技能逐帧验收）

协议唯一来源是固件 `App/game_protocol.h/c` 与 PC
`pc_app/protocol.py`。所有帧使用 11 位标准 ID、DLC=8、500 kbps。
