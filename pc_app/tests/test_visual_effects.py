from __future__ import annotations

import os
import time
import unittest

os.environ.setdefault("SDL_VIDEODRIVER", "dummy")

import pygame

from arena_asset_loader import ArenaAssetLoader
from arena_map_renderer import ArenaMapRenderer
from game_model import GameModel
from protocol import GameState, NODE_ID_BOARD_A, NODE_ID_BOARD_B
from ui_theme import Palette, create_fonts
from visual_effects import VisualEffects


class VisualEffectTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        pygame.init()
        pygame.display.set_mode((1, 1))

    @classmethod
    def tearDownClass(cls) -> None:
        pygame.quit()

    def test_three_skills_have_distinct_effect_states(self) -> None:
        self.assertEqual(
            {
                VisualEffects.effect_kind(1),
                VisualEffects.effect_kind(2),
                VisualEffects.effect_kind(3),
            },
            {"projectile", "healing_aura", "shockwave"},
        )

    def test_offscreen_renderer_draws_skill_and_crystal_effects(self) -> None:
        palette = Palette()
        fonts = create_fonts()
        renderer = ArenaMapRenderer(
            palette,
            fonts,
            ArenaAssetLoader(),
        )
        model = GameModel()
        model.node_online[NODE_ID_BOARD_A] = True
        model.node_online[NODE_ID_BOARD_B] = True
        model.on_global_state(
            {
                "term": 1,
                "master": NODE_ID_BOARD_A,
                "player": NODE_ID_BOARD_B,
                "pc_hp": 100,
                "embedded_hp": 100,
                "pc_score": 0,
                "embedded_score": 0,
                "game_state": int(GameState.RUNNING),
            }
        )
        model.on_skill_result(
            {
                "master_node": NODE_ID_BOARD_A,
                "source_node": 0,
                "skill_id": 3,
                "result": 1,
                "input_seq": 1,
                "pc_hp": 100,
                "embedded_hp": 70,
                "term": 1,
            }
        )
        model.on_crystal_attack(
            {
                "crystal_id": 1,
                "target_node": NODE_ID_BOARD_B,
                "damage": 10,
                "hit_seq": 1,
                "pc_hp": 100,
                "embedded_hp": 60,
                "term": 1,
            }
        )
        now = time.monotonic()
        model.pc_hero.active_skill_started_at = now - 0.55
        model.pc_hero.active_skill_until = now + 0.60
        assert model.crystal_attack is not None
        model.crystal_attack.started_at = now - 0.40
        model.crystal_attack.ends_at = now + 0.35

        surface = pygame.Surface((960, 540))
        renderer.draw(surface, surface.get_rect(), model)

        self.assertGreater(sum(surface.get_at((480, 270))[:3]), 0)
        self.assertFalse(hasattr(renderer, "blue_tower"))
        self.assertFalse(hasattr(renderer, "red_tower"))


if __name__ == "__main__":
    unittest.main()
