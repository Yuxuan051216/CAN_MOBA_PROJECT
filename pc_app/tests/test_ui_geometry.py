from __future__ import annotations

import os
import time
import unittest

os.environ.setdefault("SDL_VIDEODRIVER", "dummy")

import pygame

from arena_asset_loader import ArenaAssetLoader
from arena_hud import ArenaHud
from arena_map_renderer import ArenaCamera, ArenaMapRenderer, ArenaWorldMapper
from game_model import GameModel
from game_rules import (
    BLUE_CRYSTAL_POSITION,
    CRYSTAL_ATTACK_RANGE_MAP_PIXELS,
    MOVE_STEP_X_PER_TICK,
    MOVE_STEP_Y_PER_TICK,
    SKILL1_RANGE_MAP_PIXELS,
    SKILL3_RANGE_MAP_PIXELS,
)
from ui_theme import Palette, create_fonts


class UiGeometryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        pygame.init()
        pygame.display.set_mode((1, 1))
        cls.palette = Palette()
        cls.fonts = create_fonts()
        cls.assets = ArenaAssetLoader()

    @classmethod
    def tearDownClass(cls) -> None:
        pygame.quit()

    def test_axis_steps_cover_equal_map_distance(self) -> None:
        mapper = ArenaWorldMapper()
        origin = mapper.world_to_map(0.5, 0.0)
        x_step = mapper.world_to_map(0.5 + MOVE_STEP_X_PER_TICK, 0.0)
        y_step = mapper.world_to_map(0.5, MOVE_STEP_Y_PER_TICK)
        self.assertLessEqual(
            abs((x_step[0] - origin[0]) - (y_step[1] - origin[1])),
            1,
        )

    def test_crystal_range_is_drawn_as_a_circle(self) -> None:
        renderer = ArenaMapRenderer(self.palette, self.fonts, self.assets)
        viewport = pygame.Rect(0, 0, 500, 500)
        map_center = renderer.mapper.world_to_map(*BLUE_CRYSTAL_POSITION)
        camera = ArenaCamera(
            1.0,
            map_center[0] - viewport.centerx,
            map_center[1] - viewport.centery,
            viewport,
        )
        surface = pygame.Surface(viewport.size, pygame.SRCALPHA)
        renderer._draw_crystal_ranges(surface, camera)
        radius = CRYSTAL_ATTACK_RANGE_MAP_PIXELS - 3
        self.assertGreater(surface.get_at((viewport.centerx + radius, viewport.centery)).a, 0)
        self.assertGreater(surface.get_at((viewport.centerx, viewport.centery + radius)).a, 0)
        self.assertEqual(
            surface.get_at(
                (
                    viewport.centerx + CRYSTAL_ATTACK_RANGE_MAP_PIXELS + 3,
                    viewport.centery,
                )
            ).a,
            0,
        )

    def test_skill_ranges_use_authoritative_circle_radii(self) -> None:
        renderer = ArenaMapRenderer(self.palette, self.fonts, self.assets)
        viewport = pygame.Rect(0, 0, 3200, 3200)
        center = viewport.center
        camera = ArenaCamera(1.0, 0, 0, viewport)
        model = GameModel()
        now = time.monotonic()

        model.pc_hero.start_skill(1, 0, 2, 10, now)
        surface = pygame.Surface(viewport.size, pygame.SRCALPHA)
        renderer._draw_skill_ranges(
            surface,
            camera,
            now,
            ((model.pc_hero, center, model.pc_hero.team_color),),
        )
        self.assertGreater(surface.get_at((center[0] + SKILL1_RANGE_MAP_PIXELS - 3, center[1])).a, 0)
        self.assertEqual(surface.get_at((center[0] + SKILL1_RANGE_MAP_PIXELS + 3, center[1])).a, 0)

        model.pc_hero.start_skill(3, 0, 2, 30, now)
        surface = pygame.Surface(viewport.size, pygame.SRCALPHA)
        renderer._draw_skill_ranges(
            surface,
            camera,
            now,
            ((model.pc_hero, center, model.pc_hero.team_color),),
        )
        self.assertGreater(surface.get_at((center[0] + SKILL3_RANGE_MAP_PIXELS - 3, center[1])).a, 0)
        self.assertEqual(surface.get_at((center[0] + SKILL3_RANGE_MAP_PIXELS + 3, center[1])).a, 0)

    def test_stationary_hero_freezes_idle_frame(self) -> None:
        renderer = ArenaMapRenderer(self.palette, self.fonts, self.assets)
        model = GameModel()
        controller = renderer.pc_hero
        now = time.monotonic()
        controller.update(
            model.pc_hero,
            model.embedded_hero,
            (200, 200),
            (400, 200),
            True,
            now,
            0.2,
        )
        controller.update(
            model.pc_hero,
            model.embedded_hero,
            (200, 200),
            (400, 200),
            True,
            now + 0.2,
            0.2,
        )
        self.assertEqual(controller.state.current_state, "idle")
        self.assertEqual(controller.state.frame_index, 0)

        model.pc_hero.x += MOVE_STEP_X_PER_TICK
        controller.update(
            model.pc_hero,
            model.embedded_hero,
            (219, 200),
            (400, 200),
            True,
            now + 0.4,
            0.2,
        )
        self.assertEqual(controller.state.current_state, "move")
        self.assertGreater(controller.state.frame_index, 0)

    def test_hud_places_pc_and_board_skills_on_opposite_sides(self) -> None:
        hud = ArenaHud(self.palette, self.fonts, self.assets)
        viewport = pygame.Rect(0, 0, 1280, 720)
        layout = hud.layout(viewport)
        self.assertLess(layout.pc_skill_rects[-1].right, viewport.centerx)
        self.assertGreater(layout.embedded_skill_rects[0].left, viewport.centerx)

    def test_crystal_asset_has_no_opaque_rectangular_bottom_edge(self) -> None:
        renderer = ArenaMapRenderer(self.palette, self.fonts, self.assets)
        for crystal in (renderer.blue_crystal, renderer.red_crystal):
            width, height = crystal.get_size()
            self.assertEqual(crystal.get_at((0, 0)).a, 0)
            self.assertEqual(crystal.get_at((width - 1, 0)).a, 0)
            self.assertEqual(crystal.get_at((0, height - 1)).a, 0)
            self.assertEqual(crystal.get_at((width - 1, height - 1)).a, 0)

    def test_old_building_placeholders_are_replaced_with_grass(self) -> None:
        renderer = ArenaMapRenderer(self.palette, self.fonts, self.assets)
        for rect in renderer.MAP_PLACEHOLDER_RECTS:
            color = renderer.map_image.get_at(rect.center)
            self.assertGreater(color.g, color.r)
        for point in (
            (renderer.map_image.get_width() // 2, 20),
            (renderer.map_image.get_width() // 2, renderer.map_image.get_height() - 20),
        ):
            color = renderer.map_image.get_at(point)
            self.assertGreater(color.g, color.r)


if __name__ == "__main__":
    unittest.main()
