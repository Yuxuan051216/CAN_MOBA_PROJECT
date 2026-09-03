from __future__ import annotations

import time

import config
from can_log import CanLog
from entities import CrystalAttackEffect, HeroEntity
from game_rules import (
    CRYSTAL_EFFECT_DURATION,
    EMBEDDED_SPAWN,
    MOVE_SPEED_X_PER_SECOND,
    MOVE_SPEED_Y_PER_SECOND,
    PC_SPAWN,
    POSITION_PREDICTION_TIMEOUT_SECONDS,
    WIN_SCORE,
    WORLD_COORD_SCALE,
)
from protocol import (
    CanFrame,
    GameControl,
    GameState,
    HEARTBEAT_FLAG_JOYSTICK_ENABLED,
    HeroState,
    NODE_ID_BOARD_A,
    NODE_ID_BOARD_B,
    NODE_ID_PC,
    NodeRole,
    SKILL_COOLDOWN_SECONDS,
    SkillId,
    SkillResult,
    SwitchReason,
    unpack_frame,
)

ROLE_NAMES = {
    int(NodeRole.NONE): "None",
    int(NodeRole.MASTER): "Master",
    int(NodeRole.PLAYER): "Player",
    int(NodeRole.COOLDOWN): "Cooldown",
    int(NodeRole.BACKUP): "Backup",
    int(NodeRole.MASTER_PLAYER): "Master+Player",
}

HERO_STATE_NAMES = {
    int(HeroState.DEAD): "Dead",
    int(HeroState.ALIVE): "Alive",
    int(HeroState.COOLDOWN): "Cooldown",
    int(HeroState.READY): "Ready",
}

GAME_STATE_NAMES = {
    int(GameState.IDLE): "IDLE",
    int(GameState.RUNNING): "RUNNING",
    int(GameState.PAUSED): "PAUSED",
    int(GameState.OVER): "OVER",
}


class GameModel:
    """协议状态与地图表现之间的适配层。"""

    def __init__(self) -> None:
        self.current_master = NODE_ID_BOARD_A
        self.current_player = NODE_ID_BOARD_B
        self.term = 1
        self.game_state = int(GameState.IDLE)

        self.pc_hero = HeroEntity(
            role_label="PC",
            side="pc",
            spawn_x=PC_SPAWN[0],
            spawn_y=PC_SPAWN[1],
            team_color=(59, 155, 255),
        )
        self.embedded_hero = HeroEntity(
            role_label="板子",
            side="embedded",
            spawn_x=EMBEDDED_SPAWN[0],
            spawn_y=EMBEDDED_SPAWN[1],
            team_color=(239, 75, 82),
        )

        # 保留旧字段名，通信和输入层无需跟随 UI 重构。
        self.pc_skill_cd_end = self.pc_hero.cooldowns
        self.embedded_skill_cd_end = self.embedded_hero.cooldowns

        self.node_online = {
            NODE_ID_PC: True,
            NODE_ID_BOARD_A: False,
            NODE_ID_BOARD_B: False,
        }
        self.node_role = {
            NODE_ID_PC: "PC Player",
            NODE_ID_BOARD_A: "Master",
            NODE_ID_BOARD_B: "Player",
        }
        self.node_hero_state = {
            NODE_ID_PC: "Alive",
            NODE_ID_BOARD_A: "Ready",
            NODE_ID_BOARD_B: "Alive",
        }
        self.node_last_heartbeat = {
            NODE_ID_PC: time.monotonic(),
            NODE_ID_BOARD_A: 0.0,
            NODE_ID_BOARD_B: 0.0,
        }
        self.node_heartbeat_seq = {
            NODE_ID_PC: None,
            NODE_ID_BOARD_A: None,
            NODE_ID_BOARD_B: None,
        }
        self.node_cooldown_end = {
            NODE_ID_BOARD_A: 0.0,
            NODE_ID_BOARD_B: 0.0,
        }
        self.node_joystick_enabled: dict[int, bool | None] = {
            NODE_ID_BOARD_A: None,
            NODE_ID_BOARD_B: None,
        }

        self.log = CanLog()
        self.last_event = "等待 CAN 数据"
        self._pc_movement = (0, 0)
        self._embedded_movement = (0, 0)
        self._last_visual_update = time.monotonic()
        self._last_authoritative_position = self._last_visual_update
        self.pending_pc_skills: dict[int, tuple[int, float]] = {}
        self.pending_game_control: tuple[int, float] | None = None
        self.keyboard_focused = True
        self.crystal_attack: CrystalAttackEffect | None = None
        self._last_crystal_event: tuple[int, int, int] | None = None

    @property
    def pc_hp(self) -> int:
        return self.pc_hero.hp

    @pc_hp.setter
    def pc_hp(self, value: int) -> None:
        self.pc_hero.set_hp(value)

    @property
    def embedded_hp(self) -> int:
        return self.embedded_hero.hp

    @embedded_hp.setter
    def embedded_hp(self, value: int) -> None:
        self.embedded_hero.set_hp(value)

    @property
    def pc_score(self) -> int:
        return self.pc_hero.score

    @pc_score.setter
    def pc_score(self, value: int) -> None:
        self.pc_hero.score = int(value)

    @property
    def embedded_score(self) -> int:
        return self.embedded_hero.score

    @embedded_score.setter
    def embedded_score(self, value: int) -> None:
        self.embedded_hero.score = int(value)

    @property
    def can_logs(self) -> list[str]:
        return self.log.recent(8)

    @property
    def game_state_name(self) -> str:
        return GAME_STATE_NAMES.get(self.game_state, f"UNKNOWN({self.game_state})")

    @property
    def outcome_text(self) -> str:
        if self.game_state != int(GameState.OVER):
            return self.game_state_name
        if self.pc_score >= WIN_SCORE:
            return "PC方胜利"
        if self.embedded_score >= WIN_SCORE:
            return "板端方胜利"
        return "对局结束"

    @property
    def master_label(self) -> str:
        if self.current_master == NODE_ID_BOARD_A:
            return "A"
        if self.current_master == NODE_ID_BOARD_B:
            return "B"
        return "PC"

    @property
    def controlled_board_label(self) -> str:
        if self.current_player == NODE_ID_BOARD_A:
            return "A"
        if self.current_player == NODE_ID_BOARD_B:
            return "B"
        return "PC"

    @property
    def embedded_online(self) -> bool:
        return bool(self.node_online.get(self.current_player, False))

    @property
    def pc_status(self) -> str:
        if not self.pc_hero.alive:
            return f"复活倒计时 {self.pc_hero.respawn_remaining():.1f}s"
        return "键盘控制 / 在线"

    @property
    def embedded_status(self) -> str:
        if not self.embedded_online:
            return f"节点 {self.controlled_board_label} 离线"
        if not self.embedded_hero.alive:
            return f"切换接管 {self.embedded_hero.respawn_remaining():.1f}s"
        return f"节点 {self.controlled_board_label} / {self.node_role[self.current_player]}"

    @property
    def backend_label(self) -> str:
        if config.CAN_BACKEND.lower() == "mock":
            return "MOCK 演示"
        return "REAL CAN"

    @property
    def mode_notice(self) -> str:
        if config.CAN_BACKEND.lower() != "mock":
            return "REAL CAN 模式：技能结果以 Master 回帧为准"
        if config.DEMO_AI:
            return "MOCK 演示模式：板子攻击/移动为自动模拟"
        return "MOCK 模式：自动攻击/移动已关闭，不代表真实板子输入"

    @property
    def joystick_status(self) -> str:
        values = []
        for node_id, label in (
            (NODE_ID_BOARD_A, "A"),
            (NODE_ID_BOARD_B, "B"),
        ):
            enabled = self.node_joystick_enabled[node_id]
            state = "?" if enabled is None else ("ON" if enabled else "OFF")
            values.append(f"{label}:{state}")
        return "Joystick " + " ".join(values)

    def _start_skill_cooldown(
        self,
        node_id: int,
        skill_id: int,
        now: float | None = None,
    ) -> None:
        try:
            skill = SkillId(skill_id)
        except ValueError:
            return
        if skill == SkillId.NONE:
            return
        now = time.monotonic() if now is None else now
        end_time = now + SKILL_COOLDOWN_SECONDS[skill]
        hero = self.pc_hero if node_id == NODE_ID_PC else self.embedded_hero
        hero.cooldowns[skill_id] = end_time

    def _reset_visual_state(self) -> None:
        self.pc_hero.reset()
        self.embedded_hero.reset()
        self._pc_movement = (0, 0)
        self._embedded_movement = (0, 0)
        self.pending_pc_skills.clear()
        self.pending_game_control = None
        self.crystal_attack = None
        self._last_crystal_event = None
        self._last_authoritative_position = time.monotonic()
        self.last_event = "对局已重置"

    def handle_local_tx(self, frame: CanFrame) -> None:
        self.log.add_frame(frame, "TX")
        parsed = unpack_frame(frame)
        if parsed["type"] == "skill_input":
            if int(parsed["node_id"]) == NODE_ID_PC:
                skill_id = int(parsed["skill_id"])
                input_seq = int(parsed["input_seq"])
                self.pending_pc_skills[skill_id] = (
                    input_seq,
                    time.monotonic(),
                )
                self.last_event = (
                    f"技能 {skill_id} 请求已发往 CAN，等待 Master 确认"
                )
        elif parsed["type"] == "game_ctrl":
            command = int(parsed["command"])
            self.pending_game_control = (command, time.monotonic())
            name = {
                int(GameControl.START): "START",
                int(GameControl.PAUSE): "PAUSE",
                int(GameControl.RESET): "RESET",
            }.get(command, str(command))
            self.last_event = (
                f"{name} 已发送，等待 Master 的 0x100 确认"
            )
            self.log.add_text(self.last_event)

    def handle_frame(self, frame: CanFrame) -> None:
        self.log.add_frame(frame, "RX")
        parsed = unpack_frame(frame)
        frame_type = parsed["type"]

        if frame_type == "heartbeat":
            self.on_heartbeat(parsed)
        elif frame_type == "global_state":
            self.on_global_state(parsed)
        elif frame_type == "position_state":
            self.on_position_state(parsed)
        elif frame_type == "role_switch":
            self.on_role_switch(parsed)
        elif frame_type == "death":
            self.on_death_event(parsed)
        elif frame_type == "ready":
            self.on_ready(parsed)
        elif frame_type == "skill_input":
            self.on_skill_input(parsed)
        elif frame_type == "skill_result":
            self.on_skill_result(parsed)
        elif frame_type == "crystal_attack":
            self.on_crystal_attack(parsed)
        elif frame_type == "move_input":
            self.on_move_input(parsed)
        elif frame_type == "master_claim":
            self.last_event = (
                f"节点 {parsed['node_id']} 申请 Master，term={parsed['term']}"
            )

    def on_heartbeat(self, parsed: dict[str, int | str]) -> None:
        node_id = int(parsed["node_id"])
        self.node_online[node_id] = True
        self.node_last_heartbeat[node_id] = time.monotonic()
        self.node_heartbeat_seq[node_id] = int(
            parsed.get("heartbeat_seq", 0)
        )
        self.node_role[node_id] = ROLE_NAMES.get(
            int(parsed["role"]),
            f"Role({parsed['role']})",
        )
        self.node_hero_state[node_id] = HERO_STATE_NAMES.get(
            int(parsed["hero_state"]),
            f"State({parsed['hero_state']})",
        )
        if node_id in self.node_joystick_enabled:
            flags = int(parsed["fault_flags"])
            self.node_joystick_enabled[node_id] = bool(
                flags & HEARTBEAT_FLAG_JOYSTICK_ENABLED
            )
        cooldown_s = int(parsed["cooldown_s"])
        if cooldown_s:
            self.node_cooldown_end[node_id] = time.monotonic() + cooldown_s
        incoming_term = int(parsed["term"])
        role = int(parsed["role"])
        if (
            incoming_term > self.term
            and role in (
                int(NodeRole.MASTER),
                int(NodeRole.MASTER_PLAYER),
            )
        ):
            self.term = incoming_term
            self.current_master = node_id

    def on_global_state(self, parsed: dict[str, int | str]) -> None:
        incoming_term = int(parsed["term"])
        incoming_master = int(parsed["master"])
        if incoming_term < self.term:
            return
        if incoming_term == self.term and incoming_master != self.current_master:
            return
        previous_state = self.game_state
        self.term = incoming_term
        self.current_master = incoming_master
        self.current_player = int(parsed["player"])
        self.pc_hp = int(parsed["pc_hp"])
        self.embedded_hp = int(parsed["embedded_hp"])
        self.pc_score = int(parsed["pc_score"])
        self.embedded_score = int(parsed["embedded_score"])
        self.game_state = int(parsed["game_state"])
        if self.game_state == int(GameState.OVER):
            self._pc_movement = (0, 0)
            self._embedded_movement = (0, 0)
            self.pending_pc_skills.clear()
            if previous_state != int(GameState.OVER):
                self.pc_hero.status = self.outcome_text
                self.embedded_hero.status = self.outcome_text
                self.last_event = self.outcome_text
                self.log.add_text(self.last_event)

        pending = self.pending_game_control
        if pending is not None:
            command = pending[0]
            expected_state = {
                int(GameControl.START): int(GameState.RUNNING),
                int(GameControl.PAUSE): int(GameState.PAUSED),
                int(GameControl.RESET): int(GameState.IDLE),
            }.get(command)
            if expected_state == self.game_state:
                self.pending_game_control = None
                if command == int(GameControl.RESET):
                    self._reset_visual_state()
                elif command == int(GameControl.START):
                    self.pc_hero.status = "战斗中"
                    self.embedded_hero.status = "战斗中"
                    self.last_event = "Master 已确认游戏开始"
                elif command == int(GameControl.PAUSE):
                    self.last_event = "Master 已确认游戏暂停"
                self.log.add_text(self.last_event)
        elif (
            previous_state != self.game_state
            and self.game_state == int(GameState.RUNNING)
        ):
            self.pc_hero.status = "战斗中"
            self.embedded_hero.status = "战斗中"

    def on_position_state(self, parsed: dict[str, int | str]) -> None:
        self.pc_hero.x = int(parsed["pc_x"]) / WORLD_COORD_SCALE
        self.pc_hero.y = int(parsed["pc_y"]) / WORLD_COORD_SCALE
        self.embedded_hero.x = (
            int(parsed["embedded_x"]) / WORLD_COORD_SCALE
        )
        self.embedded_hero.y = (
            int(parsed["embedded_y"]) / WORLD_COORD_SCALE
        )
        self._last_authoritative_position = time.monotonic()

    def on_role_switch(self, parsed: dict[str, int | str]) -> None:
        incoming_term = int(parsed["term"])
        if incoming_term < self.term:
            return
        self.term = incoming_term
        self.current_master = int(parsed["master"])
        self.current_player = int(parsed["player"])
        target = int(parsed["target_node"])
        cooldown_s = int(parsed["cooldown_s"])

        for node_id in (NODE_ID_BOARD_A, NODE_ID_BOARD_B):
            if node_id == self.current_master and node_id == self.current_player:
                self.node_role[node_id] = "Master+Player"
            elif node_id == self.current_master:
                self.node_role[node_id] = "Master"
            elif node_id == self.current_player:
                self.node_role[node_id] = "Player"
            else:
                self.node_role[node_id] = "Backup"

        if cooldown_s and target in self.node_cooldown_end:
            self.node_cooldown_end[target] = time.monotonic() + cooldown_s
            self.node_hero_state[target] = "Cooldown"

        reason = int(parsed["reason"])
        if reason == int(SwitchReason.MANUAL_RESET):
            self._reset_visual_state()
        else:
            self.embedded_hero.mark_switch()
            if not self.embedded_hero.alive:
                # 板端英雄死亡后由另一节点接管，短暂展示切换动画后回到战场。
                self.embedded_hero.respawn_at = min(
                    self.embedded_hero.respawn_at,
                    time.monotonic() + 1.4,
                )
        self.last_event = (
            f"角色切换: Master={self.current_master}, "
            f"Player={self.current_player}, reason={parsed['reason']}"
        )
        self.log.add_text(self.last_event)

    def on_death_event(self, parsed: dict[str, int | str]) -> None:
        incoming_term = int(parsed.get("term", self.term))
        if incoming_term < self.term:
            return
        dead_node = int(parsed["dead_node"])
        cooldown_s = int(parsed["cooldown_s"])
        now = time.monotonic()
        if dead_node == NODE_ID_PC:
            self.pc_hero.mark_dead(max(1.0, cooldown_s), now)
        else:
            # 板端会进行角色轮换，所以地图上只短暂显示阵亡。
            self.embedded_hero.mark_dead(
                min(2.0, max(1.2, cooldown_s)),
                now,
                "阵亡 / 等待接管",
            )
        if dead_node in self.node_cooldown_end and cooldown_s:
            self.node_cooldown_end[dead_node] = now + cooldown_s
            self.node_hero_state[dead_node] = "Cooldown"
        self.last_event = (
            f"死亡事件: dead={dead_node}, killer={parsed['killer_node']}"
        )
        self.log.add_text(self.last_event)

    def on_ready(self, parsed: dict[str, int | str]) -> None:
        node_id = int(parsed["node_id"])
        if node_id in self.node_cooldown_end:
            self.node_cooldown_end[node_id] = 0.0
            self.node_hero_state[node_id] = "Ready"
        if node_id == self.current_player and not self.embedded_hero.alive:
            self.embedded_hero.mark_respawn()
        self.last_event = f"节点 {node_id} 冷却完成 READY"
        self.log.add_text(self.last_event)

    def on_skill_input(self, parsed: dict[str, int | str]) -> None:
        node_id = int(parsed["node_id"])
        skill_id = int(parsed["skill_id"])
        if node_id != NODE_ID_PC:
            self.last_event = f"收到节点 {node_id} 技能请求 {skill_id}"

    def on_skill_result(self, parsed: dict[str, int | str]) -> None:
        source_node = int(parsed["source_node"])
        skill_id = int(parsed["skill_id"])
        result = int(parsed["result"])
        input_seq = int(parsed["input_seq"])
        incoming_term = int(parsed["term"])
        master_node = int(parsed["master_node"])
        if incoming_term < self.term:
            return
        if incoming_term == self.term and master_node != self.current_master:
            return
        self.term = incoming_term
        self.pc_hp = int(parsed["pc_hp"])
        self.embedded_hp = int(parsed["embedded_hp"])

        if source_node == NODE_ID_PC:
            pending = self.pending_pc_skills.get(skill_id)
            if pending is not None and pending[0] == input_seq:
                del self.pending_pc_skills[skill_id]

        if result == int(SkillResult.ACCEPTED):
            now = time.monotonic()
            self._start_skill_cooldown(source_node, skill_id, now)
            caster = (
                self.pc_hero
                if source_node == NODE_ID_PC
                else self.embedded_hero
            )
            target_node = (
                source_node
                if skill_id == int(SkillId.SKILL_2)
                else (
                    self.current_player
                    if source_node == NODE_ID_PC
                    else NODE_ID_PC
                )
            )
            value = {
                int(SkillId.SKILL_1): 10,
                int(SkillId.SKILL_2): 15,
                int(SkillId.SKILL_3): 30,
            }.get(skill_id, 0)
            caster.start_skill(
                skill_id,
                source_node,
                target_node,
                value,
                now,
            )
            self.last_event = (
                f"Master {parsed['master_node']} 已确认节点 "
                f"{source_node} 的技能 {skill_id}"
            )
            self.log.add_text(self.last_event)
            return

        reason = {
            int(SkillResult.GAME_NOT_RUNNING): "对局未开始或已暂停",
            int(SkillResult.COOLDOWN): "技能仍在冷却",
            int(SkillResult.NOT_CURRENT_PLAYER): "不是当前 Player",
            int(SkillResult.OUT_OF_RANGE): "目标超出技能范围",
        }.get(result, f"未知原因 {result}")
        self.last_event = f"技能 {skill_id} 被 Master 拒绝：{reason}"
        self.log.add_text(self.last_event)

    def on_crystal_attack(self, parsed: dict[str, int | str]) -> None:
        incoming_term = int(parsed["term"])
        if incoming_term < self.term:
            return
        event_key = (
            incoming_term,
            int(parsed["crystal_id"]),
            int(parsed["hit_seq"]),
        )
        if event_key == self._last_crystal_event:
            return
        self._last_crystal_event = event_key
        self.term = incoming_term
        self.pc_hp = int(parsed["pc_hp"])
        self.embedded_hp = int(parsed["embedded_hp"])
        now = time.monotonic()
        self.crystal_attack = CrystalAttackEffect(
            crystal_id=int(parsed["crystal_id"]),
            target_node=int(parsed["target_node"]),
            damage=int(parsed["damage"]),
            hit_seq=int(parsed["hit_seq"]),
            term=incoming_term,
            started_at=now,
            ends_at=now + CRYSTAL_EFFECT_DURATION,
        )
        self.last_event = (
            f"水晶 {parsed['crystal_id']} 命中节点 "
            f"{parsed['target_node']}，伤害 {parsed['damage']}"
        )
        self.log.add_text(self.last_event)

    def on_move_input(self, parsed: dict[str, int | str]) -> None:
        node_id = int(parsed["node_id"])
        movement = (int(parsed["x_dir"]), -int(parsed["y_dir"]))
        if node_id == NODE_ID_PC:
            self._pc_movement = movement
        else:
            self._embedded_movement = movement

    def set_pc_movement(self, x_dir: int, y_dir: int) -> None:
        """立即更新本地地图移动，同时继续由 PcPlayer 发送 CAN 输入帧。"""

        self._pc_movement = (int(x_dir), int(y_dir))

    def update_timeouts(self) -> None:
        now = time.monotonic()
        self.node_online[NODE_ID_PC] = True
        for node_id in (NODE_ID_BOARD_A, NODE_ID_BOARD_B):
            last_seen = self.node_last_heartbeat[node_id]
            self.node_online[node_id] = (
                last_seen > 0.0
                and now - last_seen <= config.NODE_TIMEOUT_SECONDS
            )

    def update_visuals(self) -> None:
        now = time.monotonic()
        delta = min(0.05, max(0.0, now - self._last_visual_update))
        self._last_visual_update = now
        self.pc_hero.update(now)
        self.embedded_hero.update(now)
        timed_out = [
            skill_id
            for skill_id, (_, sent_at) in self.pending_pc_skills.items()
            if now - sent_at > 1.2
        ]
        for skill_id in timed_out:
            del self.pending_pc_skills[skill_id]
            self.last_event = (
                f"技能 {skill_id} 未收到 Master 确认，请检查真实 CAN 链路"
            )
            self.log.add_text(self.last_event)

        if (
            self.pending_game_control is not None
            and now - self.pending_game_control[1] > 1.2
        ):
            command = self.pending_game_control[0]
            self.pending_game_control = None
            name = {
                int(GameControl.START): "START",
                int(GameControl.PAUSE): "PAUSE",
                int(GameControl.RESET): "RESET",
            }.get(command, str(command))
            self.last_event = (
                f"{name} 未收到 Master 的 0x100 确认，请检查 CAN 链路"
            )
            self.log.add_text(self.last_event)

        if self.game_state != int(GameState.RUNNING):
            return
        if (
            now - self._last_authoritative_position
            > POSITION_PREDICTION_TIMEOUT_SECONDS
        ):
            return

        self.pc_hero.move(
            self._pc_movement[0],
            self._pc_movement[1],
            MOVE_SPEED_X_PER_SECOND * delta,
            MOVE_SPEED_Y_PER_SECOND * delta,
        )
        self.embedded_hero.move(
            self._embedded_movement[0],
            self._embedded_movement[1],
            MOVE_SPEED_X_PER_SECOND * delta,
            MOVE_SPEED_Y_PER_SECOND * delta,
        )
        if (
            self.crystal_attack is not None
            and now >= self.crystal_attack.ends_at
        ):
            self.crystal_attack = None

    def skill_cooldown_remaining(self, side: str, skill_id: int) -> float:
        hero = self.pc_hero if side == "pc" else self.embedded_hero
        return hero.cooldown_remaining(skill_id)

    def skill_pending(self, skill_id: int) -> bool:
        return skill_id in self.pending_pc_skills

    def can_request_pc_skill(self, skill_id: int) -> tuple[bool, str]:
        if self.skill_pending(skill_id):
            return False, "上一帧技能仍在等待 Master 确认"
        return True, ""

    def node_cooldown_remaining(self, node_id: int) -> float:
        return max(
            0.0,
            self.node_cooldown_end.get(node_id, 0.0) - time.monotonic(),
        )
