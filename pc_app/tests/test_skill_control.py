from __future__ import annotations

import unittest
from unittest.mock import patch

import pygame

from can_driver import BaseCanBus, MockCanBus
from game_model import GameModel
from pc_player import PcPlayer
from protocol import (
    CAN_ID_MOVE_INPUT_BASE,
    CAN_ID_SKILL_INPUT_BASE,
    CanFrame,
    GameControl,
    GameState,
    NODE_ID_BOARD_A,
    NODE_ID_PC,
    SkillResult,
    pack_game_ctrl,
    pack_skill_result,
)


class RecordingBus(BaseCanBus):
    def __init__(self) -> None:
        self.sent: list[CanFrame] = []

    def send(self, frame: CanFrame) -> None:
        self.sent.append(frame)

    def recv(self, timeout: float = 0.0) -> CanFrame | None:
        return None


def drain_into_model(bus: MockCanBus, model: GameModel) -> None:
    while True:
        frame = bus.recv(0.0)
        if frame is None:
            return
        model.handle_frame(frame)


class SkillControlTests(unittest.TestCase):
    def test_pc_skill_waits_for_master_confirmation(self) -> None:
        bus = RecordingBus()
        model = GameModel()
        model.game_state = int(GameState.RUNNING)
        player = PcPlayer(bus, model)

        player.send_skill(1)

        self.assertEqual(len(bus.sent), 1)
        self.assertEqual(bus.sent[0].can_id, CAN_ID_SKILL_INPUT_BASE + NODE_ID_PC)
        self.assertEqual(bus.sent[0].data[2], model.current_player)
        self.assertTrue(model.skill_pending(1))
        self.assertEqual(model.skill_cooldown_remaining("pc", 1), 0.0)

        model.handle_frame(
            pack_skill_result(
                NODE_ID_BOARD_A,
                NODE_ID_PC,
                1,
                int(SkillResult.ACCEPTED),
                bus.sent[0].data[5],
                100,
                90,
                1,
            )
        )
        self.assertFalse(model.skill_pending(1))
        self.assertGreater(model.skill_cooldown_remaining("pc", 1), 0.0)
        self.assertEqual(model.embedded_hp, 90)

    def test_pc_player_to_mock_master_end_to_end(self) -> None:
        bus = MockCanBus()
        model = GameModel()
        player = PcPlayer(bus, model)

        player.send_game_control(int(GameControl.START))
        self.assertEqual(model.game_state, int(GameState.IDLE))
        self.assertIsNotNone(model.pending_game_control)
        drain_into_model(bus, model)
        self.assertEqual(model.game_state, int(GameState.RUNNING))
        self.assertIsNone(model.pending_game_control)

        bus.pc_position[:] = (0.50, 0.0)
        bus.embedded_position[:] = (0.60, 0.0)
        player.send_skill(1)
        self.assertTrue(model.skill_pending(1))
        drain_into_model(bus, model)

        self.assertFalse(model.skill_pending(1))
        self.assertEqual(model.embedded_hp, 90)
        self.assertGreater(model.skill_cooldown_remaining("pc", 1), 0.0)
        self.assertIn("已确认", model.last_event)

    def test_idle_skill_is_sent_and_master_rejects_it(self) -> None:
        bus = MockCanBus()
        model = GameModel()
        player = PcPlayer(bus, model)

        player.send_skill(1)
        self.assertTrue(model.skill_pending(1))
        drain_into_model(bus, model)

        self.assertFalse(model.skill_pending(1))
        self.assertEqual(model.embedded_hp, 100)
        self.assertEqual(model.skill_cooldown_remaining("pc", 1), 0.0)

    def test_keyboard_jkl_emit_three_skill_frames(self) -> None:
        pygame.init()
        bus = RecordingBus()
        model = GameModel()
        player = PcPlayer(bus, model)

        events = [
            pygame.event.Event(pygame.KEYDOWN, key=pygame.K_j),
            pygame.event.Event(pygame.KEYDOWN, key=pygame.K_k),
            pygame.event.Event(pygame.KEYDOWN, key=pygame.K_l),
        ]
        self.assertTrue(player.handle_events(events))

        skill_frames = [
            frame
            for frame in bus.sent
            if frame.can_id == CAN_ID_SKILL_INPUT_BASE + NODE_ID_PC
        ]
        self.assertEqual(
            [frame.data[1] for frame in skill_frames],
            [1, 2, 3],
        )

    def test_unchanged_movement_is_periodically_refreshed(self) -> None:
        class ReleasedKeys:
            def __getitem__(self, key: int) -> int:
                return 0

        bus = RecordingBus()
        model = GameModel()
        player = PcPlayer(bus, model)
        player.last_move = (0, 0)
        player.last_move_send = 10.0

        with patch(
            "pygame.key.get_pressed",
            return_value=ReleasedKeys(),
        ):
            player._update_movement(10.30)

        self.assertEqual(len(bus.sent), 1)
        self.assertEqual(
            bus.sent[0].can_id,
            CAN_ID_MOVE_INPUT_BASE + NODE_ID_PC,
        )


if __name__ == "__main__":
    unittest.main()
