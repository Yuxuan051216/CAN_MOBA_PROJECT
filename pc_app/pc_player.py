from __future__ import annotations

import time

import config
from can_driver import BaseCanBus
from game_model import GameModel
from protocol import (
    GameControl,
    HeroState,
    NODE_ID_BOARD_A,
    NODE_ID_BOARD_B,
    NODE_ID_PC,
    NodeRole,
    SkillId,
    pack_game_ctrl,
    pack_heartbeat,
    pack_move_input,
    pack_skill_input,
)


class PcPlayer:
    def __init__(self, bus: BaseCanBus, model: GameModel) -> None:
        self.bus = bus
        self.model = model
        self.input_seq = 0
        self.heartbeat_seq = 0
        self.last_heartbeat = 0.0
        self.last_move_send = 0.0
        self.last_move = (0, 0)

    def _send(self, frame) -> None:
        self.model.handle_local_tx(frame)
        self.bus.send(frame)

    def _next_seq(self) -> int:
        value = self.input_seq
        self.input_seq = (self.input_seq + 1) & 0xFF
        return value

    def send_skill(self, skill_id: int) -> None:
        allowed, reason = self.model.can_request_pc_skill(skill_id)
        if not allowed:
            self.model.last_event = reason
            self.model.log.add_text(f"技能 {skill_id} 未发送：{reason}")
            return
        self._send(
            pack_skill_input(
                NODE_ID_PC,
                skill_id,
                self.model.current_player,
                self._next_seq(),
            )
        )

    def send_game_control(self, command: int) -> None:
        if self.model.pending_game_control is not None:
            self.model.last_event = "上一条游戏控制仍在等待 Master 确认"
            self.model.log.add_text(self.model.last_event)
            return
        self._send(pack_game_ctrl(command))

    def _toggle_mock_node(self, node_id: int) -> None:
        setter = getattr(self.bus, "set_node_online", None)
        states = getattr(self.bus, "node_online", None)
        if setter is None or states is None:
            self.model.log.add_text("节点上下线快捷键只在 mock 模式可用")
            return
        new_state = not bool(states[node_id])
        setter(node_id, new_state)
        self.model.log.add_text(
            f"Mock 节点 {node_id} -> {'ONLINE' if new_state else 'OFFLINE'}"
        )

    def handle_events(self, events) -> bool:
        import pygame

        for event in events:
            if event.type == pygame.QUIT:
                return False
            if event.type != pygame.KEYDOWN:
                continue

            if event.key == pygame.K_ESCAPE:
                return False
            if event.key == pygame.K_j:
                self.model.log.add_text("键盘 J -> PC SKILL_1")
                self.send_skill(int(SkillId.SKILL_1))
            elif event.key == pygame.K_k:
                self.model.log.add_text("键盘 K -> PC SKILL_2")
                self.send_skill(int(SkillId.SKILL_2))
            elif event.key == pygame.K_l:
                self.model.log.add_text("键盘 L -> PC SKILL_3")
                self.send_skill(int(SkillId.SKILL_3))
            elif event.key == pygame.K_SPACE:
                self.model.log.add_text("键盘 SPACE -> START")
                self.send_game_control(int(GameControl.START))
            elif event.key == pygame.K_p:
                self.model.log.add_text("键盘 P -> PAUSE")
                self.send_game_control(int(GameControl.PAUSE))
            elif event.key == pygame.K_r:
                self.model.log.add_text("键盘 R -> RESET")
                self.send_game_control(int(GameControl.RESET))
            elif event.key == pygame.K_F1:
                self._toggle_mock_node(NODE_ID_BOARD_A)
            elif event.key == pygame.K_F2:
                self._toggle_mock_node(NODE_ID_BOARD_B)

        return True

    def _update_movement(self, now: float) -> None:
        import pygame

        keys = pygame.key.get_pressed()
        x_dir = int(keys[pygame.K_d]) - int(keys[pygame.K_a])
        y_dir = int(keys[pygame.K_w]) - int(keys[pygame.K_s])
        movement = (x_dir, y_dir)
        # pygame 屏幕向下为正，因此本地地图坐标需要翻转 Y 方向。
        self.model.set_pc_movement(x_dir, -y_dir)

        changed = movement != self.last_move
        refresh_due = (
            now - self.last_move_send
            >= config.MOVE_INPUT_REFRESH_SECONDS
        )
        if not changed and not refresh_due:
            return
        if changed and now - self.last_move_send < 0.05:
            return

        self.last_move = movement
        self.last_move_send = now
        self._send(
            pack_move_input(
                NODE_ID_PC,
                x_dir,
                y_dir,
                self._next_seq(),
            )
        )

    def update(self) -> None:
        now = time.monotonic()
        import pygame

        self.model.keyboard_focused = bool(pygame.key.get_focused())
        self._update_movement(now)

        if now - self.last_heartbeat >= config.PC_HEARTBEAT_SECONDS:
            self.last_heartbeat = now
            heartbeat_seq = self.heartbeat_seq
            self.heartbeat_seq = (heartbeat_seq + 1) & 0xFF
            hero_state = (
                HeroState.ALIVE if self.model.pc_hero.alive else HeroState.DEAD
            )
            self._send(
                pack_heartbeat(
                    NODE_ID_PC,
                    int(NodeRole.PLAYER),
                    int(hero_state),
                    self.model.pc_hp,
                    self.model.term,
                    0,
                    heartbeat_seq=heartbeat_seq,
                )
            )
