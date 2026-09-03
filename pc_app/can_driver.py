from __future__ import annotations

from abc import ABC, abstractmethod
from collections import deque
import math
import time

import config
from game_rules import (
    BLUE_CRYSTAL_POSITION,
    CRYSTAL_ATTACK_INTERVAL_SECONDS,
    CRYSTAL_ATTACK_RANGE_MAP_PIXELS,
    CRYSTAL_BLUE,
    CRYSTAL_DAMAGE,
    CRYSTAL_RED,
    distance_map_pixels,
    EMBEDDED_SPAWN,
    MASTER_TICK_SECONDS,
    MOVE_STEP_X_PER_TICK,
    MOVE_STEP_Y_PER_TICK,
    PC_SPAWN,
    POSITION_STATE_PERIOD_SECONDS,
    RED_CRYSTAL_POSITION,
    skill_in_range,
    WIN_SCORE,
    WORLD_COORD_SCALE,
    WORLD_X_BOUNDS,
    WORLD_MAP_X_PIXELS,
    WORLD_MAP_Y_PIXELS,
    WORLD_Y_BOUNDS,
)
from protocol import (
    CAN_ID_GAME_CTRL,
    CAN_ID_MOVE_INPUT_BASE,
    CAN_ID_SKILL_INPUT_BASE,
    CanFrame,
    GameControl,
    GameState,
    HeroState,
    NODE_COUNT,
    NODE_ID_BOARD_A,
    NODE_ID_BOARD_B,
    NODE_ID_PC,
    NodeRole,
    SkillId,
    SkillResult,
    SwitchReason,
    pack_death_event,
    pack_crystal_attack,
    pack_global_state,
    pack_heartbeat,
    pack_master_claim,
    pack_move_input,
    pack_position_state,
    pack_ready,
    pack_role_switch,
    pack_skill_input,
    pack_skill_result,
)


class BaseCanBus(ABC):
    @abstractmethod
    def send(self, frame: CanFrame) -> None:
        raise NotImplementedError

    @abstractmethod
    def recv(self, timeout: float = 0.0) -> CanFrame | None:
        raise NotImplementedError

    def close(self) -> None:
        pass


class MockCanBus(BaseCanBus):
    """无硬件演示总线，内置两块虚拟 CH32V307 节点。"""

    def __init__(self) -> None:
        self._rx: deque[CanFrame] = deque()
        self.node_online = {
            NODE_ID_BOARD_A: True,
            NODE_ID_BOARD_B: True,
        }
        self.current_master = NODE_ID_BOARD_A
        self.current_player = NODE_ID_BOARD_B
        self.term = 1
        self.game_state = GameState.IDLE
        self.pc_hp = 100
        self.embedded_hp = 100
        self.pc_score = 0
        self.embedded_score = 0
        self.winner = 0
        self.pc_skill_cd_end = [0.0] * 4
        self.embedded_skill_cd_end = [0.0] * 4
        self.node_cooldown_end: dict[int, float] = {}
        self.last_heartbeat = 0.0
        self.last_global = 0.0
        self.last_position_state = 0.0
        self.last_embedded_attack = time.monotonic()
        self.last_demo_move = 0.0
        self.master_offline_since: float | None = None
        self.pc_position = [PC_SPAWN[0], PC_SPAWN[1]]
        self.embedded_position = [
            EMBEDDED_SPAWN[0],
            EMBEDDED_SPAWN[1],
        ]
        self.pc_movement = [0, 0]
        self.embedded_movement = [0, 0]
        now = time.monotonic()
        self.last_position_update = now
        self.crystal_last_attack = [now, now]
        self.crystal_in_range = [False, False]
        self.crystal_hit_seq = 0
        self.pc_respawn_end = 0.0
        self.embedded_move_seq = 0
        self.last_skill_input_seq: dict[int, int] = {}
        self.heartbeat_seq = {
            NODE_ID_BOARD_A: 0,
            NODE_ID_BOARD_B: 0,
        }

    def _queue(self, frame: CanFrame) -> None:
        self._rx.append(frame)

    def _global_frame(self) -> CanFrame:
        return pack_global_state(
            self.term,
            self.current_master,
            self.current_player,
            self.pc_hp,
            self.embedded_hp,
            self.pc_score,
            self.embedded_score,
            int(self.game_state),
        )

    def _position_frame(self) -> CanFrame:
        return pack_position_state(
            round(self.pc_position[0] * WORLD_COORD_SCALE),
            round(self.pc_position[1] * WORLD_COORD_SCALE),
            round(self.embedded_position[0] * WORLD_COORD_SCALE),
            round(self.embedded_position[1] * WORLD_COORD_SCALE),
        )

    def _node_role(self, node_id: int) -> NodeRole:
        if node_id == self.current_master and node_id == self.current_player:
            return NodeRole.MASTER_PLAYER
        if node_id == self.current_master:
            return NodeRole.MASTER
        if node_id == self.current_player:
            return NodeRole.PLAYER
        if node_id in self.node_cooldown_end:
            return NodeRole.COOLDOWN
        return NodeRole.BACKUP

    def _node_hero_state(self, node_id: int) -> HeroState:
        if node_id in self.node_cooldown_end:
            return HeroState.COOLDOWN
        return HeroState.ALIVE if node_id == self.current_player else HeroState.READY

    def _emit_heartbeats(self, now: float) -> None:
        if now - self.last_heartbeat < 0.5:
            return
        self.last_heartbeat = now

        for node_id in (NODE_ID_BOARD_A, NODE_ID_BOARD_B):
            if not self.node_online[node_id]:
                continue
            remaining = max(0, int(self.node_cooldown_end.get(node_id, now) - now + 0.999))
            heartbeat_seq = self.heartbeat_seq[node_id]
            self.heartbeat_seq[node_id] = (heartbeat_seq + 1) & 0xFF
            self._queue(
                pack_heartbeat(
                    node_id,
                    int(self._node_role(node_id)),
                    int(self._node_hero_state(node_id)),
                    self.embedded_hp,
                    self.term,
                    remaining,
                    heartbeat_seq=heartbeat_seq,
                )
            )

    def _emit_global(self, now: float) -> None:
        if (
            self.node_online.get(self.current_master, False)
            and now - self.last_global >= 0.1
        ):
            self.last_global = now
            self._queue(self._global_frame())

    def _emit_position(self, now: float) -> None:
        if (
            self.node_online.get(self.current_master, False)
            and now - self.last_position_state
            >= POSITION_STATE_PERIOD_SECONDS
        ):
            self.last_position_state = now
            self._queue(self._position_frame())

    def _update_cooldowns(self, now: float) -> None:
        ready_nodes = [
            node_id
            for node_id, end_time in self.node_cooldown_end.items()
            if now >= end_time
        ]
        for node_id in ready_nodes:
            del self.node_cooldown_end[node_id]
            if self.node_online.get(node_id, False):
                self._queue(pack_ready(node_id, self.term))

    def _handle_master_timeout(self, now: float) -> None:
        if self.node_online.get(self.current_master, False):
            self.master_offline_since = None
            return

        if self.master_offline_since is None:
            self.master_offline_since = now
            return
        if now - self.master_offline_since < 1.5:
            return

        candidates = [
            node_id
            for node_id in (NODE_ID_BOARD_A, NODE_ID_BOARD_B)
            if self.node_online[node_id]
        ]
        if not candidates:
            return

        old_master = self.current_master
        winner = min(candidates)
        new_player = self.current_player
        if not self.node_online.get(new_player, False) or new_player == old_master:
            new_player = winner

        self.term = (self.term + 1) & 0xFF
        self.current_master = winner
        self.current_player = new_player
        self.master_offline_since = None

        self._queue(
            pack_master_claim(
                winner,
                self.term,
                winner,
                int(self._node_hero_state(winner)),
            )
        )
        self._queue(
            pack_role_switch(
                self.term,
                winner,
                new_player,
                int(SwitchReason.MASTER_TIMEOUT),
                old_master,
                0,
            )
        )
        self._queue(self._global_frame())

    def _recover_split_roles(self) -> None:
        """双板在线后退出 Master+Player 降级态。"""
        if self.current_master != self.current_player:
            return

        recovered_player = (
            NODE_ID_BOARD_B
            if self.current_master == NODE_ID_BOARD_A
            else NODE_ID_BOARD_A
        )
        if not self.node_online.get(recovered_player, False):
            return

        self.term = (self.term + 1) & 0xFF
        self.current_player = recovered_player
        self._queue(
            pack_role_switch(
                self.term,
                self.current_master,
                self.current_player,
                int(SwitchReason.NODE_RECOVERY),
                recovered_player,
                0,
            )
        )
        self._queue(self._global_frame())

    def _skill_result(
        self,
        source_node: int,
        skill_id: int,
        result: SkillResult,
        input_seq: int,
    ) -> None:
        self._queue(
            pack_skill_result(
                self.current_master,
                source_node,
                skill_id,
                int(result),
                input_seq,
                self.pc_hp,
                self.embedded_hp,
                self.term,
            )
        )

    def _is_duplicate_skill(self, node_id: int, input_seq: int) -> bool:
        if self.last_skill_input_seq.get(node_id) == input_seq:
            return True
        self.last_skill_input_seq[node_id] = input_seq
        return False

    @staticmethod
    def _in_crystal_range(
        position: list[float],
        crystal_position: tuple[float, float],
    ) -> bool:
        return (
            distance_map_pixels(position, crystal_position)
            <= CRYSTAL_ATTACK_RANGE_MAP_PIXELS
        )

    def _reset_positions(self) -> None:
        self.pc_position[:] = PC_SPAWN
        self.embedded_position[:] = EMBEDDED_SPAWN
        self.pc_movement[:] = (0, 0)
        self.embedded_movement[:] = (0, 0)

    def _reset_crystal_timers(self, now: float) -> None:
        self.crystal_last_attack[:] = (now, now)
        self.crystal_in_range[:] = (False, False)
        self.crystal_hit_seq = 0

    def _enter_game_over(self, winner: int) -> None:
        if self.game_state == GameState.OVER:
            return
        self.winner = winner
        self.game_state = GameState.OVER
        self.pc_movement[:] = (0, 0)
        self.embedded_movement[:] = (0, 0)
        self._reset_crystal_timers(time.monotonic())
        self._queue(self._global_frame())

    def _update_positions(self, now: float) -> None:
        elapsed = now - self.last_position_update
        ticks = min(4, int(elapsed / MASTER_TICK_SECONDS))
        if ticks <= 0:
            return
        self.last_position_update += ticks * MASTER_TICK_SECONDS
        if self.game_state != GameState.RUNNING:
            return

        for position, movement in (
            (self.pc_position, self.pc_movement),
            (self.embedded_position, self.embedded_movement),
        ):
            position[0] = max(
                WORLD_X_BOUNDS[0],
                min(
                    WORLD_X_BOUNDS[1],
                    position[0]
                    + movement[0] * MOVE_STEP_X_PER_TICK * ticks,
                ),
            )
            position[1] = max(
                WORLD_Y_BOUNDS[0],
                min(
                    WORLD_Y_BOUNDS[1],
                    position[1]
                    + movement[1] * MOVE_STEP_Y_PER_TICK * ticks,
                ),
            )

    def _handle_pc_death(self, now: float) -> None:
        if self.pc_hp != 0:
            return
        self.embedded_score = (self.embedded_score + 1) & 0xFF
        self._queue(
            pack_death_event(
                NODE_ID_PC,
                self.current_player,
                3,
                self.term,
            )
        )
        self.pc_hp = 100
        self.pc_skill_cd_end = [0.0] * 4
        self.pc_position[:] = PC_SPAWN
        self.pc_movement[:] = (0, 0)
        self.pc_respawn_end = now + 3.0
        self._queue(self._position_frame())
        if self.embedded_score >= WIN_SCORE:
            self._enter_game_over(NODE_ID_BOARD_B)

    def _crystal_attack(
        self,
        crystal_id: int,
        target_node: int,
        target_index: int,
    ) -> None:
        if target_node == NODE_ID_PC:
            self.pc_hp = max(0, self.pc_hp - CRYSTAL_DAMAGE)
        else:
            self.embedded_hp = max(
                0,
                self.embedded_hp - CRYSTAL_DAMAGE,
            )
        self._queue(
            pack_crystal_attack(
                crystal_id,
                target_node,
                CRYSTAL_DAMAGE,
                self.crystal_hit_seq,
                self.pc_hp,
                self.embedded_hp,
                self.term,
            )
        )
        self.crystal_hit_seq = (self.crystal_hit_seq + 1) & 0xFF
        if target_index == 0 and self.embedded_hp == 0:
            self._handle_embedded_death(time.monotonic())
        elif target_index == 1 and self.pc_hp == 0:
            self._handle_pc_death(time.monotonic())
        if self.game_state != GameState.OVER:
            self._queue(self._global_frame())

    def _process_one_crystal(
        self,
        now: float,
        index: int,
        crystal_id: int,
        target_node: int,
        position: list[float],
        crystal_position: tuple[float, float],
        attackable: bool,
    ) -> None:
        in_range = attackable and self._in_crystal_range(
            position,
            crystal_position,
        )
        was_in_range = self.crystal_in_range[index]
        self.crystal_in_range[index] = in_range
        if not in_range:
            self.crystal_last_attack[index] = now
            return
        if not was_in_range:
            self.crystal_last_attack[index] = now
            return
        if (
            now - self.crystal_last_attack[index]
            < CRYSTAL_ATTACK_INTERVAL_SECONDS
        ):
            return
        self.crystal_last_attack[index] = now
        self._crystal_attack(crystal_id, target_node, index)

    def _process_crystal_attacks(self, now: float) -> None:
        if self.game_state != GameState.RUNNING:
            self._reset_crystal_timers(now)
            return

        embedded_attackable = (
            self.embedded_hp > 0
            and self.node_online.get(self.current_player, False)
            and self.current_player not in self.node_cooldown_end
        )
        self._process_one_crystal(
            now,
            0,
            CRYSTAL_BLUE,
            self.current_player,
            self.embedded_position,
            BLUE_CRYSTAL_POSITION,
            embedded_attackable,
        )
        self._process_one_crystal(
            now,
            1,
            CRYSTAL_RED,
            NODE_ID_PC,
            self.pc_position,
            RED_CRYSTAL_POSITION,
            self.pc_hp > 0 and now >= self.pc_respawn_end,
        )

    def _set_embedded_movement(
        self,
        x_dir: int,
        y_dir: int,
        emit_frame: bool,
    ) -> None:
        self.embedded_movement[:] = (x_dir, -y_dir)
        if emit_frame:
            self._queue(
                pack_move_input(
                    self.current_player,
                    x_dir,
                    y_dir,
                    self.embedded_move_seq,
                )
            )
            self.embedded_move_seq = (
                self.embedded_move_seq + 1
            ) & 0xFF

    def _update_demo_movement(self, now: float) -> None:
        if not config.DEMO_AI or self.game_state != GameState.RUNNING:
            return
        if now - self.last_demo_move < 0.2:
            return
        if not self.node_online.get(self.current_player, False):
            return
        self.last_demo_move = now
        gap = self.pc_position[0] - self.embedded_position[0]
        x_dir = 0 if abs(gap) <= 0.22 else (1 if gap > 0 else -1)
        target_y = math.sin(now * 0.85) * 0.22
        y_world = target_y - self.embedded_position[1]
        y_dir = 0 if abs(y_world) < 0.04 else (-1 if y_world > 0 else 1)
        if [x_dir, -y_dir] != self.embedded_movement:
            self._set_embedded_movement(x_dir, y_dir, True)

    def _embedded_attack(self, now: float) -> None:
        if self.game_state != GameState.RUNNING:
            return
        if not self.node_online.get(self.current_player, False):
            return
        if self.current_player in self.node_cooldown_end:
            return
        if now - self.last_embedded_attack < 2.0:
            return
        if now < self.embedded_skill_cd_end[SkillId.SKILL_1]:
            return
        if not skill_in_range(
            int(SkillId.SKILL_1),
            self.embedded_position,
            self.pc_position,
        ):
            return

        input_seq = int(now * 10) & 0xFF
        if self._is_duplicate_skill(self.current_player, input_seq):
            return
        self.last_embedded_attack = now
        self.embedded_skill_cd_end[SkillId.SKILL_1] = now + 1.0
        self._queue(
            pack_skill_input(
                self.current_player,
                int(SkillId.SKILL_1),
                NODE_ID_PC,
                input_seq,
            )
        )
        self.pc_hp = max(0, self.pc_hp - 10)
        self._skill_result(
            self.current_player,
            int(SkillId.SKILL_1),
            SkillResult.ACCEPTED,
            input_seq,
        )
        if self.pc_hp == 0:
            self._handle_pc_death(now)
        if self.game_state != GameState.OVER:
            self._queue(self._global_frame())

    def _handle_embedded_death(self, now: float) -> None:
        old_master = self.current_master
        old_player = self.current_player
        self.pc_score = (self.pc_score + 1) & 0xFF
        self._queue(pack_death_event(old_player, NODE_ID_PC, 10, self.term))

        self.node_cooldown_end[old_player] = now + 10.0
        self.embedded_hp = 100
        self.embedded_skill_cd_end = [0.0] * 4
        self.embedded_position[:] = EMBEDDED_SPAWN
        self.embedded_movement[:] = (0, 0)
        self.term = (self.term + 1) & 0xFF
        self.current_master = old_player
        self.current_player = old_master
        # 降级态阵亡时，在线的另一块板接管 Player。
        if old_master == old_player:
            recovered_player = (
                NODE_ID_BOARD_B
                if old_player == NODE_ID_BOARD_A
                else NODE_ID_BOARD_A
            )
            if self.node_online.get(recovered_player, False):
                self.current_player = recovered_player

        self._queue(
            pack_role_switch(
                self.term,
                self.current_master,
                self.current_player,
                int(SwitchReason.PLAYER_DEATH),
                old_player,
                10,
            )
        )
        self._queue(self._position_frame())
        if self.pc_score >= WIN_SCORE:
            self._enter_game_over(NODE_ID_PC)
        else:
            self._queue(self._global_frame())

    def _process_pc_skill(
        self,
        skill_id: int,
        input_seq: int,
        now: float,
    ) -> None:
        if self._is_duplicate_skill(NODE_ID_PC, input_seq):
            return
        if self.game_state != GameState.RUNNING:
            self._skill_result(
                NODE_ID_PC,
                skill_id,
                SkillResult.GAME_NOT_RUNNING,
                input_seq,
            )
            return
        if skill_id not in (1, 2, 3):
            return
        if now < self.pc_skill_cd_end[skill_id]:
            self._skill_result(
                NODE_ID_PC,
                skill_id,
                SkillResult.COOLDOWN,
                input_seq,
            )
            return
        if not skill_in_range(
            skill_id,
            self.pc_position,
            self.embedded_position,
        ):
            self._skill_result(
                NODE_ID_PC,
                skill_id,
                SkillResult.OUT_OF_RANGE,
                input_seq,
            )
            return

        cooldown = {1: 1.0, 2: 5.0, 3: 10.0}[skill_id]
        self.pc_skill_cd_end[skill_id] = now + cooldown

        if skill_id == SkillId.SKILL_1:
            self.embedded_hp = max(0, self.embedded_hp - 10)
        elif skill_id == SkillId.SKILL_2:
            self.pc_hp = min(100, self.pc_hp + 15)
        elif skill_id == SkillId.SKILL_3:
            self.embedded_hp = max(0, self.embedded_hp - 30)

        self._skill_result(
            NODE_ID_PC,
            skill_id,
            SkillResult.ACCEPTED,
            input_seq,
        )
        if self.embedded_hp == 0:
            self._handle_embedded_death(now)
        else:
            self._queue(self._global_frame())

    def _process_embedded_skill(
        self,
        node_id: int,
        skill_id: int,
        input_seq: int,
        now: float,
    ) -> None:
        if self._is_duplicate_skill(node_id, input_seq):
            return
        if self.game_state != GameState.RUNNING:
            self._skill_result(
                node_id,
                skill_id,
                SkillResult.GAME_NOT_RUNNING,
                input_seq,
            )
            return
        if node_id != self.current_player:
            self._skill_result(
                node_id,
                skill_id,
                SkillResult.NOT_CURRENT_PLAYER,
                input_seq,
            )
            return
        if skill_id not in (1, 2, 3):
            return
        if now < self.embedded_skill_cd_end[skill_id]:
            self._skill_result(
                node_id,
                skill_id,
                SkillResult.COOLDOWN,
                input_seq,
            )
            return
        if not skill_in_range(
            skill_id,
            self.embedded_position,
            self.pc_position,
        ):
            self._skill_result(
                node_id,
                skill_id,
                SkillResult.OUT_OF_RANGE,
                input_seq,
            )
            return

        cooldown = {1: 1.0, 2: 5.0, 3: 10.0}[skill_id]
        self.embedded_skill_cd_end[skill_id] = now + cooldown
        if skill_id == SkillId.SKILL_1:
            self.pc_hp = max(0, self.pc_hp - 10)
        elif skill_id == SkillId.SKILL_2:
            self.embedded_hp = min(100, self.embedded_hp + 15)
        elif skill_id == SkillId.SKILL_3:
            self.pc_hp = max(0, self.pc_hp - 30)

        self._skill_result(
            node_id,
            skill_id,
            SkillResult.ACCEPTED,
            input_seq,
        )
        if self.pc_hp == 0:
            self._handle_pc_death(now)
        if self.game_state != GameState.OVER:
            self._queue(self._global_frame())

    def _reset_game(self) -> None:
        now = time.monotonic()
        self.term = (self.term + 1) & 0xFF
        # Reset 保留正常角色；双板在线时会退出同节点降级态。
        if self.current_master == self.current_player:
            recovered_player = (
                NODE_ID_BOARD_B
                if self.current_master == NODE_ID_BOARD_A
                else NODE_ID_BOARD_A
            )
            if self.node_online.get(recovered_player, False):
                self.current_player = recovered_player

        self.game_state = GameState.IDLE
        self.pc_hp = 100
        self.embedded_hp = 100
        self.pc_score = 0
        self.embedded_score = 0
        self.winner = 0
        self.pc_skill_cd_end = [0.0] * 4
        self.embedded_skill_cd_end = [0.0] * 4
        self.last_skill_input_seq.clear()
        self.node_cooldown_end.clear()
        self.pc_respawn_end = 0.0
        self._reset_positions()
        self._reset_crystal_timers(now)
        self._queue(
            pack_role_switch(
                self.term,
                self.current_master,
                self.current_player,
                int(SwitchReason.MANUAL_RESET),
                NODE_ID_PC,
                0,
            )
        )
        self._queue(self._position_frame())
        self._queue(self._global_frame())

    def _update(self) -> None:
        now = time.monotonic()
        self._update_demo_movement(now)
        self._update_positions(now)
        self._update_cooldowns(now)
        self._handle_master_timeout(now)
        self._recover_split_roles()
        self._emit_heartbeats(now)
        self._emit_global(now)
        self._emit_position(now)
        if config.DEMO_AI:
            self._embedded_attack(now)
        self._process_crystal_attacks(now)

    def send(self, frame: CanFrame) -> None:
        self._queue(frame)
        now = time.monotonic()

        if frame.can_id == CAN_ID_GAME_CTRL:
            command = frame.data[0]
            if command == GameControl.START and self.game_state == GameState.IDLE:
                self.game_state = GameState.RUNNING
                self._queue(self._global_frame())
            elif command == GameControl.PAUSE and self.game_state == GameState.RUNNING:
                self.game_state = GameState.PAUSED
                self.pc_movement[:] = (0, 0)
                self.embedded_movement[:] = (0, 0)
                self._reset_crystal_timers(now)
                self._queue(self._global_frame())
            elif command == GameControl.RESET:
                self._reset_game()
            return

        if frame.can_id == CAN_ID_SKILL_INPUT_BASE + NODE_ID_PC:
            self._process_pc_skill(frame.data[1], frame.data[5], now)
            return
        if frame.can_id in (
            CAN_ID_SKILL_INPUT_BASE + NODE_ID_BOARD_A,
            CAN_ID_SKILL_INPUT_BASE + NODE_ID_BOARD_B,
        ):
            self._process_embedded_skill(
                frame.data[0],
                frame.data[1],
                frame.data[5],
                now,
            )
            return

        if frame.can_id == CAN_ID_MOVE_INPUT_BASE + NODE_ID_PC:
            if self.game_state != GameState.RUNNING:
                return
            self.pc_movement[:] = (
                frame.data[1] - 1,
                1 - frame.data[2],
            )
            return
        if frame.can_id in (
            CAN_ID_MOVE_INPUT_BASE + NODE_ID_BOARD_A,
            CAN_ID_MOVE_INPUT_BASE + NODE_ID_BOARD_B,
        ):
            if (
                self.game_state == GameState.RUNNING
                and frame.data[0] == self.current_player
            ):
                self.embedded_movement[:] = (
                    frame.data[1] - 1,
                    1 - frame.data[2],
                )
            return

    def recv(self, timeout: float = 0.0) -> CanFrame | None:
        deadline = time.monotonic() + max(0.0, timeout)
        while True:
            self._update()
            if self._rx:
                return self._rx.popleft()
            if time.monotonic() >= deadline:
                return None
            time.sleep(0.001)

    def set_node_online(self, node_id: int, online: bool) -> None:
        if node_id not in (NODE_ID_BOARD_A, NODE_ID_BOARD_B):
            raise ValueError("mock node must be board A or board B")
        self.node_online[node_id] = online
        if online:
            self.master_offline_since = None

    def debug_force_embedded_death(self) -> None:
        self.embedded_hp = 0
        self._handle_embedded_death(time.monotonic())


class PythonCanBus(BaseCanBus):
    def __init__(
        self,
        interface: str,
        channel: str,
        bitrate: int,
    ) -> None:
        try:
            import can
        except ImportError as exc:
            raise RuntimeError(
                "python-can 未安装，请执行: python -m pip install python-can"
            ) from exc

        self._can = can
        self._bus = can.Bus(
            interface=interface,
            channel=channel,
            bitrate=bitrate,
        )

    def send(self, frame: CanFrame) -> None:
        message = self._can.Message(
            arbitration_id=frame.can_id,
            data=frame.data,
            is_extended_id=False,
            is_remote_frame=False,
        )
        self._bus.send(message)

    def recv(self, timeout: float = 0.0) -> CanFrame | None:
        message = self._bus.recv(timeout)
        if message is None:
            return None
        if message.is_extended_id or message.is_remote_frame:
            return None
        data = bytes(message.data)
        if len(data) != 8:
            return None
        return CanFrame(
            can_id=message.arbitration_id,
            data=data,
            timestamp=message.timestamp or time.time(),
        )

    def close(self) -> None:
        self._bus.shutdown()


class UsbCanPlaceholder(BaseCanBus):
    def __init__(self) -> None:
        raise RuntimeError(
            "UsbCanPlaceholder 需要按 USB-CAN 厂商 DLL/串口协议实现。"
            "当前可先使用 mock 或 python-can 后端。"
        )

    def send(self, frame: CanFrame) -> None:
        raise NotImplementedError

    def recv(self, timeout: float = 0.0) -> CanFrame | None:
        raise NotImplementedError


def create_can_bus() -> BaseCanBus:
    backend = config.CAN_BACKEND.lower()
    if backend == "mock":
        return MockCanBus()
    if backend == "python-can":
        return PythonCanBus(
            config.PYTHON_CAN_INTERFACE,
            config.PYTHON_CAN_CHANNEL,
            config.PYTHON_CAN_BITRATE,
        )
    if backend == "usbcan-placeholder":
        return UsbCanPlaceholder()
    raise ValueError(f"未知 CAN_BACKEND: {config.CAN_BACKEND}")
