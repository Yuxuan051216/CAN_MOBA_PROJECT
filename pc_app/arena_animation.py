from __future__ import annotations

import math
from dataclasses import dataclass

import pygame

from arena_asset_loader import ArenaAssetLoader
from game_rules import WORLD_MAP_X_PIXELS, WORLD_MAP_Y_PIXELS
from ui_theme import FontSet, Palette, draw_hp_bar, draw_text


@dataclass(slots=True)
class AnimationState:
    last_position: tuple[int, int] | None = None
    last_world_position: tuple[float, float] | None = None
    is_moving: bool = False
    last_direction: str = "right"
    current_state: str = "idle"
    frame_index: int = 0
    frame_elapsed: float = 0.0
    attack_started_at: float = 0.0
    hit_started_at: float = 0.0
    death_started_at: float = 0.0
    delayed_hp_ratio: float = 1.0


class HeroAnimationController:
    """Control eight-direction ArenaofValor hero animation frames."""

    def __init__(
        self,
        hero_name: str,
        display_name: str,
        assets: ArenaAssetLoader,
        palette: Palette,
        fonts: FontSet,
        team_color: tuple[int, int, int],
    ) -> None:
        self.hero_name = hero_name
        self.display_name = display_name
        self.assets = assets
        self.palette = palette
        self.fonts = fonts
        self.team_color = team_color
        self.animations = assets.hero_animations(hero_name)
        self.state = AnimationState(last_direction="right" if hero_name == "HouYi" else "left")
        self._tint_cache: dict[tuple[int, int], pygame.Surface] = {}

    def update(self, hero, opponent, foot_position: tuple[int, int], opponent_position: tuple[int, int], online: bool, now: float, dt: float) -> None:
        direction = self._direction_from_motion(hero)
        if direction is not None:
            self.state.last_direction = direction
        if now < hero.action_flash_until:
            self.state.last_direction = self._direction_to_target(foot_position, opponent_position)
        elif now < hero.respawn_flash_until:
            self.state.last_direction = "right" if hero.x < opponent.x else "left"

        next_state = self._state_for(hero, online, now)
        if next_state != self.state.current_state:
            self.state.current_state = next_state
            self.state.frame_index = 0
            self.state.frame_elapsed = 0.0
            if next_state == "attack":
                self.state.attack_started_at = now
            elif next_state == "hit":
                self.state.hit_started_at = now
            elif next_state == "dead":
                self.state.death_started_at = now

        frames = self._frames()
        if next_state in ("idle", "offline", "dead"):
            self.state.frame_index = 0
            self.state.frame_elapsed = 0.0
        else:
            fps = {"attack": 13.0, "move": 10.0, "hit": 10.0}.get(next_state, 10.0)
            self.state.frame_elapsed += dt
            while self.state.frame_elapsed >= 1.0 / fps:
                self.state.frame_elapsed -= 1.0 / fps
                if next_state == "attack":
                    self.state.frame_index = min(
                        self.state.frame_index + 1,
                        len(frames) - 1,
                    )
                else:
                    self.state.frame_index = (
                        self.state.frame_index + 1
                    ) % len(frames)

        ratio = hero.hp_ratio
        if ratio < self.state.delayed_hp_ratio:
            self.state.delayed_hp_ratio = max(ratio, self.state.delayed_hp_ratio - dt * 0.7)
        else:
            self.state.delayed_hp_ratio = ratio

    def current_frame(self) -> pygame.Surface:
        frames = self._frames()
        return frames[self.state.frame_index % len(frames)]

    def draw(
        self,
        target: pygame.Surface,
        hero,
        foot_position: tuple[int, int],
        online: bool,
        scale: float,
        now: float,
    ) -> None:
        image = self.current_frame()
        scaled_size = (max(1, int(image.get_width() * scale)), max(1, int(image.get_height() * scale)))
        image = self.assets.scaled(image, scaled_size).copy()
        if not online or self.state.current_state == "dead":
            image.set_alpha(120 if not online else 150)
        elif self.state.current_state == "hit":
            flash = pygame.Surface(image.get_size(), pygame.SRCALPHA)
            flash.fill((255, 255, 255, 90))
            image.blit(flash, (0, 0), special_flags=pygame.BLEND_RGBA_ADD)
        rect = image.get_rect(midbottom=foot_position)
        rect.y += int(18 * scale)
        target.blit(image, rect)
        self._draw_nameplate(target, hero, foot_position, online, image.get_width(), now)

    def _frames(self) -> list[pygame.Surface]:
        direction = self.state.last_direction
        if self.state.current_state == "attack":
            return self.animations["attack"][direction]
        return self.animations["move"][direction]

    def _state_for(self, hero, online: bool, now: float) -> str:
        if not hero.alive:
            return "dead"
        if not online:
            return "offline"
        if now < hero.hit_flash_until:
            return "hit"
        if now < hero.action_flash_until:
            return "attack"
        if self.state.is_moving:
            return "move"
        return "idle"

    def _direction_from_motion(self, hero) -> str | None:
        current = (hero.x, hero.y)
        previous = self.state.last_world_position
        self.state.last_world_position = current
        if previous is None:
            self.state.is_moving = False
            return None
        dx = (current[0] - previous[0]) * WORLD_MAP_X_PIXELS
        dy = (current[1] - previous[1]) * WORLD_MAP_Y_PIXELS
        self.state.is_moving = math.hypot(dx, dy) >= 0.1
        if not self.state.is_moving:
            return None
        return self._direction_from_vector(dx, dy)

    def _direction_to_target(self, source: tuple[int, int], target: tuple[int, int]) -> str:
        return self._direction_from_vector(target[0] - source[0], target[1] - source[1])

    @staticmethod
    def _direction_from_vector(dx: float, dy: float) -> str:
        if abs(dx) < 0.01 and abs(dy) < 0.01:
            return "down"
        angle = math.degrees(math.atan2(dy, dx))
        if -22.5 <= angle < 22.5:
            return "right"
        if 22.5 <= angle < 67.5:
            return "downRight"
        if 67.5 <= angle < 112.5:
            return "down"
        if 112.5 <= angle < 157.5:
            return "downLeft"
        if angle >= 157.5 or angle < -157.5:
            return "left"
        if -157.5 <= angle < -112.5:
            return "upLeft"
        if -112.5 <= angle < -67.5:
            return "up"
        return "upRight"

    def _draw_nameplate(
        self,
        surface: pygame.Surface,
        hero,
        foot_position: tuple[int, int],
        online: bool,
        hero_width: int,
        now: float,
    ) -> None:
        bar_width = max(54, min(96, int(hero_width * 0.34)))
        x = foot_position[0] - bar_width // 2
        y = foot_position[1] - int(hero_width * 0.42)
        name_color = self.palette.gray if not online else self.team_color
        draw_text(surface, self.fonts.tiny, self.display_name, (foot_position[0], y - 17), name_color, "midtop")
        if hero.alive:
            bar = pygame.Rect(x, y, bar_width, 8)
            draw_hp_bar(
                surface,
                bar,
                hero.hp_ratio,
                self.palette,
                self.team_color,
                self.state.delayed_hp_ratio,
                offline=not online,
            )
        else:
            draw_text(
                surface,
                self.fonts.tiny,
                f"复活 {hero.respawn_remaining(now):.1f}s",
                (foot_position[0], y),
                self.palette.red_soft,
                "midtop",
            )
