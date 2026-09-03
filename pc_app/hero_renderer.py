from __future__ import annotations

import math
import time
from dataclasses import dataclass, field

import pygame

from asset_manager import AssetManager
from ui_theme import FontSet, Palette, draw_hp_bar, draw_text


@dataclass(slots=True)
class HeroVisualState:
    last_pos: tuple[float, float] | None = None
    facing: int = 1
    state: str = "idle"
    frame_time: float = 0.0
    frame_index: int = 0
    delayed_hp_ratio: float = 1.0
    trail: list[tuple[pygame.Surface, tuple[int, int], float]] = field(default_factory=list)


class HeroRenderer:
    """Render heroes using cached artwork and model-derived animation states."""

    def __init__(
        self,
        palette: Palette,
        fonts: FontSet,
        assets: AssetManager,
        hero_key: str,
        display_name: str,
    ) -> None:
        self.palette = palette
        self.fonts = fonts
        self.assets = assets
        self.hero_key = hero_key
        self.display_name = display_name
        self.frames = assets.hero_frames(hero_key)
        self.visuals: dict[int, HeroVisualState] = {}
        self._transform_cache: dict[tuple[int, int, int, int, int, int], pygame.Surface] = {}

    def update(self, hero, now: float, dt: float, online: bool = True) -> None:
        state = self._state_for(hero, now, online)
        visual = self._visual(hero)
        pos = (float(hero.x), float(hero.y))
        if visual.last_pos is not None:
            dx = pos[0] - visual.last_pos[0]
            if abs(dx) > 0.0004:
                visual.facing = 1 if dx > 0 else -1
        visual.last_pos = pos
        if state != visual.state:
            visual.state = state
            visual.frame_time = now
            visual.frame_index = 0
        fps = 10.0 if state in ("move", "attack", "hit") else 8.0
        frames = self.frames.get(state if state != "offline" else "idle", self.frames["idle"])
        elapsed_frames = int((now - visual.frame_time) * fps)
        if frames:
            visual.frame_index = elapsed_frames % len(frames)
        ratio = float(hero.hp_ratio)
        if ratio < visual.delayed_hp_ratio:
            visual.delayed_hp_ratio = max(ratio, visual.delayed_hp_ratio - dt * 0.45)
        else:
            visual.delayed_hp_ratio = ratio

    def draw(
        self,
        surface: pygame.Surface,
        hero,
        screen_position: tuple[int, int],
        online: bool,
        detail: str,
    ) -> None:
        now = time.monotonic()
        visual = self._visual(hero)
        frame = self._current_frame(visual)
        image, offset = self._animated_image(hero, frame, visual, now, online)
        center = (screen_position[0] + offset[0], screen_position[1] + offset[1])
        self._draw_shadow(surface, screen_position, visual.state, online)
        if visual.state == "move":
            self._draw_trail(surface, image, screen_position, visual)
        surface.blit(image, image.get_rect(center=center))
        self._draw_overlays(surface, hero, screen_position, online, detail, visual, now)

    def _visual(self, hero) -> HeroVisualState:
        key = id(hero)
        if key not in self.visuals:
            self.visuals[key] = HeroVisualState()
        return self.visuals[key]

    def _state_for(self, hero, now: float, online: bool) -> str:
        if not online:
            return "offline"
        if not hero.alive:
            return "death"
        if now < hero.respawn_flash_until:
            return "respawn"
        if now < hero.hit_flash_until:
            return "hit"
        if now < hero.action_flash_until:
            return "attack"
        visual = self._visual(hero)
        if visual.last_pos is not None:
            moved = abs(hero.x - visual.last_pos[0]) + abs(hero.y - visual.last_pos[1])
            if moved > 0.001:
                return "move"
        return "idle"

    def _current_frame(self, visual: HeroVisualState) -> pygame.Surface:
        key = visual.state if visual.state != "offline" else "idle"
        frames = self.frames.get(key, self.frames["idle"])
        return frames[visual.frame_index % len(frames)]

    def _animated_image(
        self,
        hero,
        frame: pygame.Surface,
        visual: HeroVisualState,
        now: float,
        online: bool,
    ) -> tuple[pygame.Surface, tuple[int, int]]:
        base_w, base_h = frame.get_size()
        target_h = 92
        scale = target_h / max(1, base_h)
        bob = math.sin(now * 5.2) * 3.0
        breath = 1.0 + math.sin(now * 3.0) * 0.025
        angle = 0.0
        alpha = 255
        offset = [0, int(bob)]
        if visual.state == "move":
            angle = -5.0 * visual.facing
            offset[0] += 3 * visual.facing
        elif visual.state == "attack":
            progress = max(0.0, min(1.0, (hero.action_flash_until - now) / 0.35))
            offset[0] += int((1.0 - abs(progress - 0.5) * 2.0) * 20 * visual.facing)
        elif visual.state == "hit":
            offset[0] += int(math.sin(now * 80.0) * 4)
        elif visual.state == "death":
            progress = 1.0 - max(0.0, min(1.0, hero.respawn_remaining(now) / 3.0))
            angle = 25.0 * progress * visual.facing
            breath = max(0.6, 1.0 - progress * 0.25)
            alpha = 145
            offset[1] += int(progress * 18)
        elif visual.state == "respawn":
            progress = 1.0 - max(0.0, min(1.0, (hero.respawn_flash_until - now) / 1.2))
            alpha = int(95 + 160 * progress)
            breath = 0.82 + 0.18 * progress
        if not online:
            alpha = 135
        size = (max(1, int(base_w * scale * breath)), max(1, int(base_h * scale * breath)))
        image = self._transform(frame, size, visual.facing < 0, int(angle), alpha, visual.state == "hit")
        return image, (offset[0], offset[1])

    def _transform(
        self,
        frame: pygame.Surface,
        size: tuple[int, int],
        flip: bool,
        angle: int,
        alpha: int,
        hit_flash: bool,
    ) -> pygame.Surface:
        key = (id(frame), size[0], size[1], int(flip), angle, alpha + (1000 if hit_flash else 0))
        if key in self._transform_cache:
            return self._transform_cache[key]
        image = self.assets.scaled(frame, size)
        if flip:
            image = pygame.transform.flip(image, True, False)
        if angle:
            image = pygame.transform.rotate(image, angle)
        image = image.copy()
        if hit_flash:
            flash = pygame.Surface(image.get_size(), pygame.SRCALPHA)
            flash.fill((255, 255, 255, 95))
            image.blit(flash, (0, 0), special_flags=pygame.BLEND_RGBA_ADD)
        if alpha < 255:
            image.set_alpha(alpha)
        if len(self._transform_cache) > 256:
            self._transform_cache.clear()
        self._transform_cache[key] = image
        return image

    def _draw_shadow(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
        state: str,
        online: bool,
    ) -> None:
        width = 92 if state != "death" else 72
        alpha = 95 if online else 55
        shadow = pygame.Surface((width + 12, 32), pygame.SRCALPHA)
        pygame.draw.ellipse(shadow, (*self.palette.shadow, alpha), shadow.get_rect())
        surface.blit(shadow, (center[0] - shadow.get_width() // 2, center[1] + 30))

    def _draw_trail(
        self,
        surface: pygame.Surface,
        image: pygame.Surface,
        center: tuple[int, int],
        visual: HeroVisualState,
    ) -> None:
        ghost = image.copy()
        ghost.set_alpha(45)
        surface.blit(ghost, ghost.get_rect(center=(center[0] - visual.facing * 14, center[1] + 4)))

    def _draw_overlays(
        self,
        surface: pygame.Surface,
        hero,
        center: tuple[int, int],
        online: bool,
        detail: str,
        visual: HeroVisualState,
        now: float,
    ) -> None:
        label_y = center[1] - 94
        name_color = self.palette.gray if not online else hero.team_color
        draw_text(surface, self.fonts.body, self.display_name, (center[0], label_y), name_color, "midtop")
        if hero.alive:
            bar = pygame.Rect(center[0] - 54, center[1] - 68, 108, 12)
            draw_hp_bar(
                surface,
                bar,
                hero.hp_ratio,
                self.palette,
                hero.team_color,
                visual.delayed_hp_ratio,
                offline=not online,
            )
            draw_text(surface, self.fonts.tiny, f"{hero.hp}/{hero.max_hp}", bar.center, self.palette.text, "center")
        else:
            draw_text(
                surface,
                self.fonts.small,
                f"复活 {hero.respawn_remaining(now):.1f}s",
                (center[0], center[1] - 66),
                self.palette.red_soft,
                "midtop",
            )
        status = "节点离线" if not online else hero.transient_status(now)
        if status not in ("待命", "战斗中"):
            color = self.palette.red_soft if not online or not hero.alive else self.palette.gold
            draw_text(surface, self.fonts.tiny, status, (center[0], center[1] + 62), color, "midtop")
        draw_text(surface, self.fonts.tiny, detail, (center[0], center[1] + 78), self.palette.muted, "midtop")
