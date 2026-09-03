from __future__ import annotations

from dataclasses import dataclass
import time

import pygame

from arena_animation import HeroAnimationController
from arena_asset_loader import ArenaAssetLoader
from entities import HeroEntity
from game_model import GameModel
from game_rules import (
    BLUE_CRYSTAL_POSITION,
    CRYSTAL_ATTACK_RANGE_MAP_PIXELS,
    CRYSTAL_BLUE,
    RED_CRYSTAL_POSITION,
    SKILL1_RANGE_MAP_PIXELS,
    SKILL3_RANGE_MAP_PIXELS,
    WORLD_MAP_X_PIXELS,
    WORLD_MAP_Y_PIXELS,
)
from ui_theme import FontSet, Palette
from visual_effects import VisualEffects


@dataclass(frozen=True, slots=True)
class ArenaCamera:
    scale: float
    offset_x: int
    offset_y: int
    viewport: pygame.Rect


class ArenaWorldMapper:
    """Map game model normalized positions into ArenaofValor map pixels."""

    def __init__(self) -> None:
        self.left_x = 1260
        self.right_x = self.left_x + WORLD_MAP_X_PIXELS
        self.center_y = 768
        self.y_range = WORLD_MAP_Y_PIXELS

    def world_to_map(self, normalized_x: float, normalized_y: float) -> tuple[int, int]:
        x = self.left_x + int((self.right_x - self.left_x) * normalized_x)
        y = self.center_y + int(normalized_y * self.y_range)
        return x, y

    def map_to_screen(self, point: tuple[int, int], camera: ArenaCamera) -> tuple[int, int]:
        return (
            camera.viewport.left + int(point[0] * camera.scale) - camera.offset_x,
            camera.viewport.top + int(point[1] * camera.scale) - camera.offset_y,
        )

    def world_to_screen(self, normalized_x: float, normalized_y: float, camera: ArenaCamera) -> tuple[int, int]:
        return self.map_to_screen(self.world_to_map(normalized_x, normalized_y), camera)


class ArenaMapRenderer:
    """Render the reference ArenaofValor map, buildings, and animated heroes."""

    MAP_PLAYFIELD = pygame.Rect(224, 160, 5952, 1216)
    MAP_PLACEHOLDER_RECTS = (
        pygame.Rect(384, 736, 480, 224),
        pygame.Rect(1792, 736, 288, 224),
        pygame.Rect(4320, 736, 288, 224),
        pygame.Rect(5568, 736, 416, 224),
    )

    def __init__(self, palette: Palette, fonts: FontSet, assets: ArenaAssetLoader) -> None:
        self.palette = palette
        self.fonts = fonts
        self.assets = assets
        self.mapper = ArenaWorldMapper()
        self.map_image = self._prepare_map(
            assets.image("map/battle_map.png")
        )
        self.blue_crystal = self._prepare_crystal(
            assets.image("buildings/blueShuiJin.png")
        )
        self.red_crystal = self._prepare_crystal(
            assets.image("buildings/redShuiJin.png")
        )
        self.pc_hero = HeroAnimationController("HouYi", "后羿", assets, palette, fonts, palette.blue)
        self.embedded_hero = HeroAnimationController("YaSe", "亚瑟", assets, palette, fonts, palette.red)
        self.effects = VisualEffects(palette, fonts)
        self._map_cache: dict[tuple[int, int, float], pygame.Surface] = {}
        self._last_time = time.monotonic()

    def camera_for(self, viewport: pygame.Rect, model: GameModel) -> ArenaCamera:
        scale = max(
            viewport.width / self.map_image.get_width(),
            viewport.height / self.map_image.get_height(),
        )
        pc_map = self.mapper.world_to_map(model.pc_hero.x, model.pc_hero.y)
        embedded_map = self.mapper.world_to_map(model.embedded_hero.x, model.embedded_hero.y)
        focus_x = (pc_map[0] + embedded_map[0]) // 2
        focus_y = (pc_map[1] + embedded_map[1]) // 2
        scaled_w = int(self.map_image.get_width() * scale)
        scaled_h = int(self.map_image.get_height() * scale)
        offset_x = max(
            0,
            min(
                scaled_w - viewport.width,
                int(focus_x * scale) - viewport.width // 2,
            ),
        )
        offset_y = max(
            0,
            min(
                scaled_h - viewport.height,
                int(focus_y * scale) - viewport.height // 2,
            ),
        )
        return ArenaCamera(scale, offset_x, offset_y, viewport)

    def draw(self, surface: pygame.Surface, viewport: pygame.Rect, model: GameModel) -> None:
        now = time.monotonic()
        dt = min(0.05, now - self._last_time)
        self._last_time = now
        camera = self.camera_for(viewport, model)
        old_clip = surface.get_clip()
        surface.set_clip(viewport)
        self._draw_map(surface, camera)
        self._draw_buildings(surface, camera)
        self._draw_crystal_ranges(surface, camera)

        pc_pos = self.mapper.world_to_screen(model.pc_hero.x, model.pc_hero.y, camera)
        embedded_pos = self.mapper.world_to_screen(model.embedded_hero.x, model.embedded_hero.y, camera)
        self.pc_hero.update(model.pc_hero, model.embedded_hero, pc_pos, embedded_pos, True, now, dt)
        self.embedded_hero.update(
            model.embedded_hero,
            model.pc_hero,
            embedded_pos,
            pc_pos,
            model.embedded_online,
            now,
            dt,
        )
        self._draw_skill_ranges(
            surface,
            camera,
            now,
            (
                (model.pc_hero, pc_pos, model.pc_hero.team_color),
                (model.embedded_hero, embedded_pos, model.embedded_hero.team_color),
            ),
        )
        hero_scale = max(0.30, min(0.42, camera.scale * 0.78))
        heroes = [
            (pc_pos[1], self.pc_hero, model.pc_hero, pc_pos, True),
            (embedded_pos[1], self.embedded_hero, model.embedded_hero, embedded_pos, model.embedded_online),
        ]
        for _, controller, hero, pos, online in sorted(heroes, key=lambda item: item[0]):
            self._draw_shadow(surface, pos, hero_scale)
            controller.draw(surface, hero, pos, online, hero_scale, now)
        self.effects.draw_hero(
            surface,
            model.pc_hero,
            pc_pos,
            (
                pc_pos
                if model.pc_hero.active_skill_id == 2
                else embedded_pos
            ),
            now,
            True,
        )
        self.effects.draw_hero(
            surface,
            model.embedded_hero,
            embedded_pos,
            (
                embedded_pos
                if model.embedded_hero.active_skill_id == 2
                else pc_pos
            ),
            now,
            model.embedded_online,
        )
        if model.crystal_attack is not None:
            source = (
                BLUE_CRYSTAL_POSITION
                if model.crystal_attack.crystal_id == CRYSTAL_BLUE
                else RED_CRYSTAL_POSITION
            )
            source_pos = self.mapper.world_to_screen(
                source[0],
                source[1],
                camera,
            )
            target_pos = (
                pc_pos
                if model.crystal_attack.target_node == 0
                else embedded_pos
            )
            self.effects.draw_crystal_attack(
                surface,
                model.crystal_attack,
                source_pos,
                target_pos,
                now,
            )
        surface.set_clip(old_clip)

    def _draw_map(self, surface: pygame.Surface, camera: ArenaCamera) -> None:
        scaled = self._scaled_map(camera)
        source = pygame.Rect(camera.offset_x, camera.offset_y, camera.viewport.width, camera.viewport.height)
        surface.blit(scaled, camera.viewport, source)

    def _scaled_map(self, camera: ArenaCamera) -> pygame.Surface:
        key = (camera.viewport.width, camera.viewport.height, round(camera.scale, 4))
        if key not in self._map_cache:
            size = (
                int(self.map_image.get_width() * camera.scale),
                int(self.map_image.get_height() * camera.scale),
            )
            self._map_cache[key] = self.assets.scaled(self.map_image, size)
        return self._map_cache[key]

    def _draw_buildings(self, surface: pygame.Surface, camera: ArenaCamera) -> None:
        buildings = [
            (self.blue_crystal, BLUE_CRYSTAL_POSITION, 0.34),
            (self.red_crystal, RED_CRYSTAL_POSITION, 0.34),
        ]
        for image, world_pos, factor in buildings:
            size = (
                max(1, int(image.get_width() * camera.scale * factor)),
                max(1, int(image.get_height() * camera.scale * factor)),
            )
            scaled = self.assets.scaled(image, size)
            screen_pos = self.mapper.world_to_screen(
                world_pos[0],
                world_pos[1],
                camera,
            )
            rect = scaled.get_rect(midbottom=(screen_pos[0], screen_pos[1] + int(90 * camera.scale)))
            surface.blit(scaled, rect)

    def _draw_crystal_ranges(
        self,
        surface: pygame.Surface,
        camera: ArenaCamera,
    ) -> None:
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        for position, color in (
            (BLUE_CRYSTAL_POSITION, self.palette.blue),
            (RED_CRYSTAL_POSITION, self.palette.red),
        ):
            center = self.mapper.world_to_screen(
                position[0],
                position[1],
                camera,
            )
            radius = max(
                1,
                round(CRYSTAL_ATTACK_RANGE_MAP_PIXELS * camera.scale),
            )
            pygame.draw.circle(overlay, (*color, 24), center, radius)
            pygame.draw.circle(overlay, (*color, 85), center, radius, 2)
        surface.blit(overlay, (0, 0))

    def _draw_skill_ranges(
        self,
        surface: pygame.Surface,
        camera: ArenaCamera,
        now: float,
        heroes: tuple[tuple[HeroEntity, tuple[int, int], tuple[int, int, int]], ...],
    ) -> None:
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        for hero, center, color in heroes:
            if not (
                hero.active_skill_id in (1, 3)
                and now < hero.active_skill_until
            ):
                continue
            range_map_pixels = (
                SKILL1_RANGE_MAP_PIXELS
                if hero.active_skill_id == 1
                else SKILL3_RANGE_MAP_PIXELS
            )
            radius = max(1, round(range_map_pixels * camera.scale))
            pygame.draw.circle(overlay, (*color, 20), center, radius)
            pygame.draw.circle(overlay, (*color, 115), center, radius, 2)
        surface.blit(overlay, (0, 0))

    @staticmethod
    def _prepare_map(image: pygame.Surface) -> pygame.Surface:
        cleaned = image.copy()
        playfield = ArenaMapRenderer.MAP_PLAYFIELD
        top = pygame.Rect(0, 0, image.get_width(), playfield.top)
        bottom = pygame.Rect(
            0,
            playfield.bottom,
            image.get_width(),
            image.get_height() - playfield.bottom,
        )
        cleaned.blit(
            image,
            top,
            pygame.Rect(
                0,
                playfield.top,
                image.get_width(),
                top.height,
            ),
        )
        cleaned.blit(
            image,
            bottom,
            pygame.Rect(
                0,
                playfield.bottom - bottom.height,
                image.get_width(),
                bottom.height,
            ),
        )
        for destination in ArenaMapRenderer.MAP_PLACEHOLDER_RECTS:
            source = destination.move(0, -destination.height - 32)
            cleaned.blit(image, destination, source)
        border_source = cleaned.copy()
        left = pygame.Rect(
            0,
            0,
            playfield.left,
            image.get_height(),
        )
        right = pygame.Rect(
            playfield.right,
            0,
            image.get_width() - playfield.right,
            image.get_height(),
        )
        cleaned.blit(
            border_source,
            left,
            pygame.Rect(
                playfield.left,
                0,
                left.width,
                image.get_height(),
            ),
        )
        cleaned.blit(
            border_source,
            right,
            pygame.Rect(
                playfield.right - right.width,
                0,
                right.width,
                image.get_height(),
            ),
        )
        return cleaned

    @staticmethod
    def _prepare_crystal(image: pygame.Surface) -> pygame.Surface:
        width, height = image.get_size()
        crop = pygame.Rect(
            round(width * 0.08),
            round(height * 0.02),
            round(width * 0.84),
            round(height * 0.80),
        ).clip(image.get_rect())
        crystal = image.subsurface(crop).copy()
        mask_scale = 4
        mask = pygame.Surface(
            (
                crystal.get_width() * mask_scale,
                crystal.get_height() * mask_scale,
            ),
            pygame.SRCALPHA,
        )
        margin_x = round(mask.get_width() * 0.035)
        margin_y = round(mask.get_height() * 0.01)
        pygame.draw.ellipse(
            mask,
            (255, 255, 255, 255),
            pygame.Rect(
                margin_x,
                margin_y,
                mask.get_width() - margin_x * 2,
                mask.get_height() - margin_y * 2,
            ),
        )
        mask = pygame.transform.smoothscale(mask, crystal.get_size())
        crystal.blit(mask, (0, 0), special_flags=pygame.BLEND_RGBA_MULT)
        return crystal

    def _draw_shadow(self, surface: pygame.Surface, foot_position: tuple[int, int], hero_scale: float) -> None:
        shadow = pygame.Surface((int(210 * hero_scale), int(58 * hero_scale)), pygame.SRCALPHA)
        pygame.draw.ellipse(shadow, (0, 0, 0, 92), shadow.get_rect())
        surface.blit(shadow, shadow.get_rect(center=(foot_position[0], foot_position[1] - int(8 * hero_scale))))
