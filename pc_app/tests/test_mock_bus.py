from __future__ import annotations

import time
import unittest
from unittest.mock import patch

from can_driver import MockCanBus
from protocol import (
    GameControl,
    GameState,
    NODE_ID_BOARD_A,
    NODE_ID_BOARD_B,
    NODE_ID_PC,
    SwitchReason,
    pack_game_ctrl,
    pack_move_input,
    pack_skill_input,
    unpack_frame,
)


def drain(bus: MockCanBus) -> list[dict]:
    frames: list[dict] = []
    while True:
        frame = bus.recv(0.0)
        if frame is None:
            return frames
        frames.append(unpack_frame(frame))


class MockBusTests(unittest.TestCase):
    @staticmethod
    def put_heroes_in_skill1_range(bus: MockCanBus) -> None:
        bus.pc_position[:] = (0.50, 0.0)
        bus.embedded_position[:] = (0.60, 0.0)

    def test_skill_damage_and_role_rotation(self) -> None:
        bus = MockCanBus()
        bus.send(pack_game_ctrl(int(GameControl.START)))
        drain(bus)
        self.put_heroes_in_skill1_range(bus)

        bus.send(pack_skill_input(NODE_ID_PC, 1, 1, 0))
        events = drain(bus)
        self.assertEqual(bus.embedded_hp, 90)
        self.assertTrue(
            any(
                event["type"] == "skill_result"
                and event["result"] == 1
                for event in events
            )
        )

        bus.debug_force_embedded_death()
        events = drain(bus)
        self.assertEqual(bus.current_master, NODE_ID_BOARD_B)
        self.assertEqual(bus.current_player, NODE_ID_BOARD_A)
        self.assertTrue(any(event["type"] == "death" for event in events))
        self.assertTrue(any(event["type"] == "role_switch" for event in events))

    def test_reset_keeps_current_master_and_player(self) -> None:
        bus = MockCanBus()
        bus.debug_force_embedded_death()
        drain(bus)
        roles_before_reset = (bus.current_master, bus.current_player)

        bus.send(pack_game_ctrl(int(GameControl.RESET)))
        events = drain(bus)

        self.assertEqual(
            (bus.current_master, bus.current_player),
            roles_before_reset,
        )
        role_switch = next(
            event for event in events if event["type"] == "role_switch"
        )
        self.assertEqual(role_switch["master"], roles_before_reset[0])
        self.assertEqual(role_switch["player"], roles_before_reset[1])

    def test_pc_death_does_not_rotate_board_roles(self) -> None:
        with patch("config.DEMO_AI", True):
            bus = MockCanBus()
            bus.send(pack_game_ctrl(int(GameControl.START)))
            drain(bus)
            roles_before_death = (bus.current_master, bus.current_player)
            bus.pc_hp = 10
            self.put_heroes_in_skill1_range(bus)
            bus.last_embedded_attack = time.monotonic() - 10.0
            bus.embedded_skill_cd_end[1] = 0.0

            drain(bus)

            self.assertEqual(
                (bus.current_master, bus.current_player),
                roles_before_death,
            )
            self.assertEqual(bus.pc_hp, 100)
            self.assertEqual(bus.embedded_score, 1)

    def test_autoplay_is_disabled_by_default(self) -> None:
        with patch("config.DEMO_AI", False):
            bus = MockCanBus()
            bus.send(pack_game_ctrl(int(GameControl.START)))
            drain(bus)
            bus.last_embedded_attack = time.monotonic() - 10.0
            drain(bus)
            self.assertEqual(bus.pc_hp, 100)

    def test_master_timeout_takeover(self) -> None:
        bus = MockCanBus()
        bus.set_node_online(NODE_ID_BOARD_A, False)
        bus.master_offline_since = time.monotonic() - 2.0
        events = drain(bus)
        self.assertEqual(bus.current_master, NODE_ID_BOARD_B)
        self.assertEqual(bus.current_player, NODE_ID_BOARD_B)
        self.assertTrue(any(event["type"] == "master_claim" for event in events))
        self.assertTrue(any(event["type"] == "role_switch" for event in events))

    def test_recovered_board_exits_master_player_degraded_mode(self) -> None:
        bus = MockCanBus()
        bus.set_node_online(NODE_ID_BOARD_A, False)
        bus.master_offline_since = time.monotonic() - 2.0
        drain(bus)
        self.assertEqual(
            (bus.current_master, bus.current_player),
            (NODE_ID_BOARD_B, NODE_ID_BOARD_B),
        )

        bus.set_node_online(NODE_ID_BOARD_A, True)
        events = drain(bus)

        self.assertEqual(
            (bus.current_master, bus.current_player),
            (NODE_ID_BOARD_B, NODE_ID_BOARD_A),
        )
        self.assertTrue(
            any(
                event["type"] == "role_switch"
                and event["reason"] == int(SwitchReason.NODE_RECOVERY)
                for event in events
            )
        )

    def test_death_recovers_same_node_roles_when_other_board_online(self) -> None:
        bus = MockCanBus()
        bus.current_master = NODE_ID_BOARD_A
        bus.current_player = NODE_ID_BOARD_A

        bus.debug_force_embedded_death()
        events = drain(bus)

        self.assertEqual(
            (bus.current_master, bus.current_player),
            (NODE_ID_BOARD_A, NODE_ID_BOARD_B),
        )
        role_switch = next(
            event for event in events if event["type"] == "role_switch"
        )
        self.assertEqual(
            (role_switch["master"], role_switch["player"]),
            (NODE_ID_BOARD_A, NODE_ID_BOARD_B),
        )

    def test_reset_recovers_same_node_roles_when_other_board_online(self) -> None:
        bus = MockCanBus()
        bus.current_master = NODE_ID_BOARD_A
        bus.current_player = NODE_ID_BOARD_A

        bus.send(pack_game_ctrl(int(GameControl.RESET)))
        drain(bus)

        self.assertEqual(
            (bus.current_master, bus.current_player),
            (NODE_ID_BOARD_A, NODE_ID_BOARD_B),
        )

    def test_crystal_outside_range_does_not_damage(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        now = time.monotonic()
        bus.crystal_last_attack[:] = (now - 10.0, now - 10.0)
        bus._process_crystal_attacks(now)
        self.assertEqual(bus.pc_hp, 100)
        self.assertEqual(bus.embedded_hp, 100)

    def test_crystal_damage_and_interval(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.embedded_position[:] = (0.193, 0.0)
        now = time.monotonic()

        bus._process_crystal_attacks(now)
        self.assertEqual(bus.embedded_hp, 100)
        bus._process_crystal_attacks(now + 1.01)
        self.assertEqual(bus.embedded_hp, 90)
        bus._process_crystal_attacks(now + 0.2)
        self.assertEqual(bus.embedded_hp, 90)
        bus._process_crystal_attacks(now + 2.02)
        self.assertEqual(bus.embedded_hp, 80)

    def test_crystal_range_matches_map_pixel_circle_on_both_axes(self) -> None:
        from game_rules import (
            BLUE_CRYSTAL_POSITION,
            CRYSTAL_ATTACK_RANGE_MAP_PIXELS,
            WORLD_MAP_X_PIXELS,
            WORLD_MAP_Y_PIXELS,
        )

        for axis, map_scale in ((0, WORLD_MAP_X_PIXELS), (1, WORLD_MAP_Y_PIXELS)):
            bus = MockCanBus()
            bus.game_state = GameState.RUNNING
            bus.embedded_position[:] = BLUE_CRYSTAL_POSITION
            bus.embedded_position[axis] += (
                CRYSTAL_ATTACK_RANGE_MAP_PIXELS - 2
            ) / map_scale
            now = time.monotonic()
            bus._process_crystal_attacks(now)
            bus._process_crystal_attacks(now + 1.01)
            self.assertEqual(bus.embedded_hp, 90)

            bus = MockCanBus()
            bus.game_state = GameState.RUNNING
            bus.embedded_position[:] = BLUE_CRYSTAL_POSITION
            bus.embedded_position[axis] += (
                CRYSTAL_ATTACK_RANGE_MAP_PIXELS + 2
            ) / map_scale
            now = time.monotonic()
            bus.crystal_last_attack[0] = now - 1.01
            bus._process_crystal_attacks(now)
            self.assertEqual(bus.embedded_hp, 100)

    def test_crystal_disabled_for_non_running_and_dead_player(self) -> None:
        for state in (GameState.IDLE, GameState.PAUSED, GameState.OVER):
            bus = MockCanBus()
            bus.game_state = state
            bus.embedded_position[:] = (0.193, 0.0)
            now = time.monotonic()
            bus.crystal_last_attack[0] = now - 10.0
            bus._process_crystal_attacks(now)
            self.assertEqual(bus.embedded_hp, 100)

        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.embedded_position[:] = (0.193, 0.0)
        bus.node_cooldown_end[bus.current_player] = time.monotonic() + 10
        bus.crystal_last_attack[0] = time.monotonic() - 10.0
        bus._process_crystal_attacks(time.monotonic())
        self.assertEqual(bus.embedded_hp, 100)

    def test_crystal_kill_uses_existing_role_rotation(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.embedded_hp = 3
        bus.embedded_position[:] = (0.193, 0.0)
        now = time.monotonic()

        bus._process_crystal_attacks(now)
        bus._process_crystal_attacks(now + 1.01)
        events = drain(bus)

        self.assertEqual(bus.pc_score, 1)
        self.assertEqual(bus.current_master, NODE_ID_BOARD_B)
        self.assertEqual(bus.current_player, NODE_ID_BOARD_A)
        self.assertTrue(
            any(event["type"] == "crystal_attack" for event in events)
        )
        self.assertTrue(any(event["type"] == "death" for event in events))
        self.assertTrue(
            any(event["type"] == "role_switch" for event in events)
        )

    def test_embedded_skill_and_duplicate_sequence(self) -> None:
        bus = MockCanBus()
        bus.send(pack_game_ctrl(int(GameControl.START)))
        drain(bus)
        self.put_heroes_in_skill1_range(bus)
        frame = pack_skill_input(
            bus.current_player,
            1,
            NODE_ID_PC,
            44,
        )
        bus.send(frame)
        drain(bus)
        self.assertEqual(bus.pc_hp, 90)

        bus.send(frame)
        drain(bus)
        self.assertEqual(bus.pc_hp, 90)

    def test_reset_clears_positions_movement_and_crystal_timer(self) -> None:
        bus = MockCanBus()
        bus.pc_position[:] = (0.8, 0.4)
        bus.embedded_position[:] = (0.2, -0.4)
        bus.pc_movement[:] = (1, 1)
        bus.embedded_movement[:] = (-1, -1)
        bus.crystal_hit_seq = 9
        bus.send(pack_game_ctrl(int(GameControl.RESET)))
        drain(bus)
        self.assertEqual(bus.pc_position, [0.29, 0.16])
        self.assertEqual(bus.embedded_position, [0.71, -0.16])
        self.assertEqual(bus.pc_movement, [0, 0])
        self.assertEqual(bus.embedded_movement, [0, 0])
        self.assertEqual(bus.crystal_hit_seq, 0)

    def test_leaving_and_reentering_crystal_range_restarts_timer(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        now = time.monotonic()
        bus.embedded_position[:] = (0.193, 0.0)
        bus._process_crystal_attacks(now)
        bus.embedded_position[:] = (0.71, -0.16)
        bus._process_crystal_attacks(now + 0.8)
        bus.embedded_position[:] = (0.193, 0.0)
        bus._process_crystal_attacks(now + 0.9)
        bus._process_crystal_attacks(now + 1.8)
        self.assertEqual(bus.embedded_hp, 100)
        bus._process_crystal_attacks(now + 1.91)
        self.assertEqual(bus.embedded_hp, 90)

    def test_crystals_only_attack_enemy_hero(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        now = time.monotonic()
        bus.pc_position[:] = (0.193, 0.0)
        bus.embedded_position[:] = (0.193, 0.0)
        bus._process_crystal_attacks(now)
        bus._process_crystal_attacks(now + 1.01)
        self.assertEqual(bus.embedded_hp, 90)
        self.assertEqual(bus.pc_hp, 100)

        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        now = time.monotonic()
        bus.pc_position[:] = (0.807, 0.0)
        bus.embedded_position[:] = (0.807, 0.0)
        bus._process_crystal_attacks(now)
        bus._process_crystal_attacks(now + 1.01)
        self.assertEqual(bus.pc_hp, 90)
        self.assertEqual(bus.embedded_hp, 100)

    def test_position_state_and_master_switch_preserve_coordinates(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.pc_position[:] = (0.44, -0.22)
        bus.embedded_position[:] = (0.62, 0.31)
        position_before = (
            tuple(bus.pc_position),
            tuple(bus.embedded_position),
        )
        bus.last_position_state = 0.0
        events = drain(bus)
        position = next(
            event for event in events if event["type"] == "position_state"
        )
        self.assertEqual(position["pc_x"], 440)
        self.assertEqual(position["embedded_y"], 310)

        bus.set_node_online(NODE_ID_BOARD_A, False)
        bus.master_offline_since = time.monotonic() - 2.0
        drain(bus)
        self.assertEqual(tuple(bus.pc_position), position_before[0])
        self.assertEqual(tuple(bus.embedded_position), position_before[1])

    def test_skill_range_rejects_without_cooldown_and_accepts_inside(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.send(pack_skill_input(NODE_ID_PC, 1, bus.current_player, 10))
        events = drain(bus)
        result = next(
            event for event in events if event["type"] == "skill_result"
        )
        self.assertEqual(result["result"], 5)
        self.assertEqual(bus.embedded_hp, 100)
        self.assertEqual(bus.pc_skill_cd_end[1], 0.0)

        self.put_heroes_in_skill1_range(bus)
        bus.send(pack_skill_input(NODE_ID_PC, 1, bus.current_player, 11))
        events = drain(bus)
        result = next(
            event for event in events if event["type"] == "skill_result"
        )
        self.assertEqual(result["result"], 1)
        self.assertEqual(bus.embedded_hp, 90)
        self.assertGreater(bus.pc_skill_cd_end[1], time.monotonic())

    def test_three_kills_game_over_blocks_actions_and_reset_restarts(self) -> None:
        bus = MockCanBus()
        bus.game_state = GameState.RUNNING
        bus.pc_score = 2
        bus.debug_force_embedded_death()
        events = drain(bus)
        self.assertEqual(bus.pc_score, 3)
        self.assertEqual(bus.game_state, GameState.OVER)
        self.assertTrue(
            any(
                event["type"] == "global_state"
                and event["game_state"] == int(GameState.OVER)
                for event in events
            )
        )

        hp_before = (bus.pc_hp, bus.embedded_hp)
        bus.send(pack_skill_input(NODE_ID_PC, 1, bus.current_player, 33))
        bus.send(pack_move_input(NODE_ID_PC, 1, 0, 34))
        bus.embedded_position[:] = (0.193, 0.0)
        now = time.monotonic()
        bus._process_crystal_attacks(now)
        bus._process_crystal_attacks(now + 2.0)
        self.assertEqual((bus.pc_hp, bus.embedded_hp), hp_before)
        self.assertEqual(bus.pc_movement, [0, 0])

        bus.send(pack_game_ctrl(int(GameControl.RESET)))
        drain(bus)
        self.assertEqual(bus.game_state, GameState.IDLE)
        self.assertEqual((bus.pc_score, bus.embedded_score), (0, 0))
        bus.send(pack_game_ctrl(int(GameControl.START)))
        drain(bus)
        self.assertEqual(bus.game_state, GameState.RUNNING)


if __name__ == "__main__":
    unittest.main()
