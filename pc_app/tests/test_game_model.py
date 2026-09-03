from __future__ import annotations

import time
import unittest

from game_model import GameModel
from protocol import (
    GameState,
    HEARTBEAT_FLAG_JOYSTICK_ENABLED,
    NODE_ID_BOARD_A,
    NODE_ID_PC,
    SkillResult,
    pack_position_state,
)


class GameModelVisualTests(unittest.TestCase):
    def test_global_state_updates_hero_entities(self) -> None:
        model = GameModel()
        model.on_global_state(
            {
                "term": 2,
                "master": 1,
                "player": 2,
                "pc_hp": 75,
                "embedded_hp": 60,
                "pc_score": 1,
                "embedded_score": 2,
                "game_state": int(GameState.RUNNING),
            }
        )
        self.assertEqual(model.pc_hero.hp, 75)
        self.assertEqual(model.embedded_hero.hp, 60)
        self.assertEqual(model.pc_hero.score, 1)
        self.assertEqual(model.embedded_hero.score, 2)

    def test_pc_death_and_respawn_visual(self) -> None:
        model = GameModel()
        model.on_death_event(
            {
                "dead_node": NODE_ID_PC,
                "killer_node": 2,
                "cooldown_s": 3,
            }
        )
        self.assertFalse(model.pc_hero.alive)
        model.pc_hero.respawn_at = time.monotonic() - 0.01
        model.update_visuals()
        self.assertTrue(model.pc_hero.alive)
        self.assertEqual(model.pc_hero.hp, model.pc_hero.max_hp)

    def test_local_movement_changes_map_position(self) -> None:
        model = GameModel()
        model.game_state = int(GameState.RUNNING)
        model.set_pc_movement(1, 0)
        start_x = model.pc_hero.x
        model._last_visual_update = time.monotonic() - 0.05
        model.update_visuals()
        self.assertGreater(model.pc_hero.x, start_x)

    def test_authoritative_position_corrects_local_drift(self) -> None:
        model = GameModel()
        model.pc_hero.x = 0.82
        model.pc_hero.y = -0.44
        model.embedded_hero.x = 0.18
        model.embedded_hero.y = 0.51

        model.handle_frame(pack_position_state(310, -120, 680, 220))

        self.assertAlmostEqual(model.pc_hero.x, 0.310)
        self.assertAlmostEqual(model.pc_hero.y, -0.120)
        self.assertAlmostEqual(model.embedded_hero.x, 0.680)
        self.assertAlmostEqual(model.embedded_hero.y, 0.220)

    def test_stale_position_stops_long_term_local_integration(self) -> None:
        model = GameModel()
        model.game_state = int(GameState.RUNNING)
        model.set_pc_movement(1, 0)
        model._last_authoritative_position = time.monotonic() - 1.0
        start_x = model.pc_hero.x
        model._last_visual_update = time.monotonic() - 0.05
        model.update_visuals()
        self.assertEqual(model.pc_hero.x, start_x)

    def test_heartbeat_reports_joystick_capability(self) -> None:
        model = GameModel()
        model.on_heartbeat(
            {
                "node_id": NODE_ID_BOARD_A,
                "role": 1,
                "hero_state": 1,
                "hp": 100,
                "term": 1,
                "cooldown_s": 0,
                "fault_flags": HEARTBEAT_FLAG_JOYSTICK_ENABLED,
                "heartbeat_seq": 9,
            }
        )
        self.assertTrue(model.node_joystick_enabled[NODE_ID_BOARD_A])
        self.assertEqual(model.node_heartbeat_seq[NODE_ID_BOARD_A], 9)
        self.assertIn("A:ON", model.joystick_status)

    def test_rejected_skill_has_no_effect_or_cooldown(self) -> None:
        model = GameModel()
        model.on_skill_result(
            {
                "master_node": NODE_ID_BOARD_A,
                "source_node": NODE_ID_PC,
                "skill_id": 1,
                "result": int(SkillResult.GAME_NOT_RUNNING),
                "input_seq": 3,
                "pc_hp": 100,
                "embedded_hp": 100,
                "term": 1,
            }
        )
        self.assertEqual(model.pc_hero.active_skill_id, 0)
        self.assertEqual(model.skill_cooldown_remaining("pc", 1), 0.0)

    def test_accepted_skills_record_caster_and_target(self) -> None:
        model = GameModel()
        model.on_skill_result(
            {
                "master_node": NODE_ID_BOARD_A,
                "source_node": NODE_ID_PC,
                "skill_id": 3,
                "result": int(SkillResult.ACCEPTED),
                "input_seq": 4,
                "pc_hp": 100,
                "embedded_hp": 70,
                "term": 1,
            }
        )
        self.assertEqual(model.pc_hero.active_skill_id, 3)
        self.assertEqual(model.pc_hero.active_skill_source_node, NODE_ID_PC)
        self.assertEqual(
            model.pc_hero.active_skill_target_node,
            model.current_player,
        )

        model.on_skill_result(
            {
                "master_node": NODE_ID_BOARD_A,
                "source_node": model.current_player,
                "skill_id": 2,
                "result": int(SkillResult.ACCEPTED),
                "input_seq": 5,
                "pc_hp": 100,
                "embedded_hp": 85,
                "term": 1,
            }
        )
        self.assertEqual(model.embedded_hero.active_skill_id, 2)
        self.assertEqual(
            model.embedded_hero.active_skill_target_node,
            model.current_player,
        )

    def test_crystal_event_updates_authoritative_hp_and_effect(self) -> None:
        model = GameModel()
        model.on_crystal_attack(
            {
                "crystal_id": 2,
                "target_node": NODE_ID_PC,
                "damage": 10,
                "hit_seq": 8,
                "pc_hp": 90,
                "embedded_hp": 100,
                "term": 1,
            }
        )
        self.assertEqual(model.pc_hp, 90)
        self.assertIsNotNone(model.crystal_attack)
        self.assertGreater(
            model.pc_hero.hit_flash_until,
            time.monotonic(),
        )

    def test_reset_clears_effects_and_movement(self) -> None:
        model = GameModel()
        model.pc_hero.start_skill(1, 0, 2, 10)
        model.on_crystal_attack(
            {
                "crystal_id": 2,
                "target_node": NODE_ID_PC,
                "damage": 10,
                "hit_seq": 8,
                "pc_hp": 90,
                "embedded_hp": 100,
                "term": 1,
            }
        )
        model.set_pc_movement(1, 1)
        model.on_role_switch(
            {
                "term": 2,
                "master": 1,
                "player": 2,
                "reason": 3,
                "target_node": 0,
                "cooldown_s": 0,
            }
        )
        self.assertEqual(model.pc_hero.active_skill_id, 0)
        self.assertIsNone(model.crystal_attack)
        self.assertEqual(model._pc_movement, (0, 0))

    def test_game_over_displays_winner_and_stops_prediction(self) -> None:
        model = GameModel()
        model.set_pc_movement(1, 1)
        model.on_global_state(
            {
                "term": 1,
                "master": NODE_ID_BOARD_A,
                "player": 2,
                "pc_hp": 100,
                "embedded_hp": 100,
                "pc_score": 3,
                "embedded_score": 1,
                "game_state": int(GameState.OVER),
            }
        )
        self.assertEqual(model.outcome_text, "PC方胜利")
        self.assertEqual(model._pc_movement, (0, 0))
        self.assertEqual(model.last_event, "PC方胜利")

    def test_out_of_range_skill_message(self) -> None:
        model = GameModel()
        model.on_skill_result(
            {
                "master_node": NODE_ID_BOARD_A,
                "source_node": NODE_ID_PC,
                "skill_id": 1,
                "result": int(SkillResult.OUT_OF_RANGE),
                "input_seq": 7,
                "pc_hp": 100,
                "embedded_hp": 100,
                "term": 1,
            }
        )
        self.assertIn("目标超出技能范围", model.last_event)
        self.assertEqual(model.skill_cooldown_remaining("pc", 1), 0.0)


if __name__ == "__main__":
    unittest.main()
