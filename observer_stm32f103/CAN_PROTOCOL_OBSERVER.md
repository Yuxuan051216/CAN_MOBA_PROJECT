# CAN 监听协议对应表

协议来源保持为：

```text
firmware_ch32v307/App/game_protocol.h
firmware_ch32v307/App/game_protocol.c
firmware_ch32v307/App/game_types.h
firmware_ch32v307/App/app_config.h
pc_app/protocol.py
pc_app/game_rules.py
```

监听工程没有重新分配 CAN ID，也不发送任何游戏应用层帧。

| CAN ID | 名称 | 字段 | Observer 行为 |
|---:|---|---|---|
| `0x001` | `ROLE_SWITCH` | term, new_master, new_player, reason, target, cooldown | 更新显示并播报 A/B 接管 Master |
| `0x010` | `GAME_CTRL` | command, source, reserved | RESET 时清空本地临时效果；状态仍以 `0x100` 为准 |
| `0x080` | `DEATH_EVENT` | dead, killer, cooldown, term | 第一滴血、阵亡语音、死亡灯效和蜂鸣 |
| `0x100` | `GLOBAL_STATE` | term, master, player, pc_hp, board_hp, pc_score, board_score, state | 保存并显示权威状态，比分到 3 时播报 |
| `0x110` | `POSITION_STATE` | pc_x, pc_y, embedded_x, embedded_y，int16 小端 | 仅接收并忽略，不参与裁判 |
| `0x120` | `CRYSTAL_ATTACK` | crystal, target, damage, hit_seq, pc_hp, board_hp, term, reserved | 水晶灯效、蜂鸣和提示语音 |
| `0x381` | `SKILL_RESULT` from A | master, source, skill, result, input_seq, pc_hp, board_hp, term | 接受结果才显示技能灯效 |
| `0x382` | `SKILL_RESULT` from B | 同上 | 接受结果才显示技能灯效 |

## 字段布局

### ROLE_SWITCH `0x001`

```text
data[0] term
data[1] new_master
data[2] new_player
data[3] reason
data[4] target_node
data[5] cooldown_s
data[6..7] reserved
```

### DEATH_EVENT `0x080`

```text
data[0] dead_node
data[1] killer_node
data[2] cooldown_s
data[3] term
data[4..7] reserved
```

死亡指纹：`dead_node + killer_node + term`。Observer 不根据 HP 生成死亡事件。

### GLOBAL_STATE `0x100`

```text
data[0] term
data[1] master
data[2] player
data[3] pc_hp
data[4] embedded_hp
data[5] pc_score
data[6] embedded_score
data[7] game_state
```

周期状态帧只更新显示，不重复播放相同状态或胜利语音。只有
`game_state=GAME_OVER` 且最终比分达到 3 才播报胜利。旧 term 不会覆盖新
term。

### CRYSTAL_ATTACK `0x120`

```text
data[0] crystal_id
data[1] target_node
data[2] damage
data[3] hit_seq
data[4] pc_hp
data[5] embedded_hp
data[6] term
data[7] reserved
```

水晶指纹：`crystal_id + target_node + hit_seq + term`。Observer 只展示帧内
结果，不按本地时间产生攻击。

水晶伤害由帧内 `damage` 给出，当前权威配置为 10。Observer 不重算伤害。

### SKILL_RESULT `0x381/0x382`

```text
data[0] master_node
data[1] source_node
data[2] skill_id
data[3] result
data[4] input_seq
data[5] pc_hp
data[6] embedded_hp
data[7] term
```

技能指纹：`source_node + skill_id + input_seq + term`。

```text
result == SKILL_RESULT_ACCEPTED
```

才允许启动：

```text
技能1  快速单闪
技能2  绿色治疗效果
技能3  快速强闪三次
```

## 来源与可信边界

`0x381/0x382` 的 ID 可直接识别 Board A/B Master。`0x080`、`0x100` 和
`0x120` 是共享 ID，没有物理发送者字段，因此依赖现有协议的单写者约束：
只有当前 Master 发布这些帧。Observer 使用 term、Master 字段和事件指纹
拒绝旧帧、矛盾状态和重复帧，但不会创建第二套仲裁机制。

Normal 模式产生的 CAN ACK 是 bxCAN 硬件应答，不是应用层发送，也不改变
Observer 的角色。
