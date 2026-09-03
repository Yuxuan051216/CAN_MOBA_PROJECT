from __future__ import annotations

import math

import pygame

from game_rules import CRYSTAL_BLUE
from ui_theme import FontSet, Palette, draw_text


class VisualEffects:
    """Render timestamp-driven effects created by authoritative CAN events."""

    def __init__(self, palette: Palette, fonts: FontSet) -> None:
        self.palette = palette
        self.fonts = fonts

    @staticmethod
    def effect_kind(skill_id: int) -> str:
        return {
            1: "projectile",
            2: "healing_aura",
            3: "shockwave",
        }.get(int(skill_id), "none")

    def draw_hero(
        self,
        surface: pygame.Surface,
        hero,
        center: tuple[int, int],
        target: tuple[int, int],
        now: float,
        online: bool,
    ) -> None:
        if not online:
            self._offline(surface, center)
            return
        if hero.active_skill_id and now < hero.active_skill_until:
            duration = max(
                0.001,
                hero.active_skill_until - hero.active_skill_started_at,
            )
            progress = max(
                0.0,
                min(1.0, (now - hero.active_skill_started_at) / duration),
            )
            if hero.active_skill_id == 1:
                self._skill_one(
                    surface,
                    center,
                    target,
                    hero.team_color,
                    progress,
                    hero.active_skill_value,
                )
            elif hero.active_skill_id == 2:
                self._skill_two(
                    surface,
                    center,
                    hero.team_color,
                    progress,
                    hero.active_skill_value,
                )
            elif hero.active_skill_id == 3:
                self._skill_three(
                    surface,
                    center,
                    target,
                    hero.team_color,
                    progress,
                    hero.active_skill_value,
                )
        if now < hero.hit_flash_until:
            self._hit(surface, center, hero.hit_flash_until - now)
        if now < hero.respawn_flash_until:
            self._respawn(
                surface,
                center,
                hero.team_color,
                hero.respawn_flash_until - now,
            )
        if now < hero.switch_flash_until:
            self._switch(surface, center, now)

    def draw_crystal_attack(
        self,
        surface: pygame.Surface,
        effect,
        source: tuple[int, int],
        target: tuple[int, int],
        now: float,
    ) -> None:
        duration = max(0.001, effect.ends_at - effect.started_at)
        progress = max(
            0.0,
            min(1.0, (now - effect.started_at) / duration),
        )
        color = (
            self.palette.blue
            if effect.crystal_id == CRYSTAL_BLUE
            else self.palette.red
        )
        travel = min(1.0, progress / 0.68)
        point = self._lerp(source, target, travel)
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        pygame.draw.line(overlay, (*color, 80), source, point, 10)
        pygame.draw.line(overlay, (255, 255, 255, 210), source, point, 3)
        pygame.draw.circle(overlay, (*color, 220), point, 10)
        pygame.draw.circle(overlay, (255, 255, 255, 245), point, 4)
        if progress >= 0.62:
            radius = 12 + int((progress - 0.62) * 90)
            pygame.draw.circle(
                overlay,
                (255, 244, 214, 210),
                target,
                radius,
                4,
            )
        surface.blit(overlay, (0, 0))
        if progress >= 0.55:
            draw_text(
                surface,
                self.fonts.body,
                f"-{effect.damage}",
                (target[0], target[1] - 72 - int(progress * 20)),
                self.palette.red_soft,
                "midbottom",
            )

    def _skill_one(
        self,
        surface: pygame.Surface,
        source: tuple[int, int],
        target: tuple[int, int],
        color: tuple[int, int, int],
        progress: float,
        damage: int,
    ) -> None:
        travel = min(1.0, progress / 0.72)
        point = self._lerp(source, target, travel)
        angle = math.atan2(target[1] - source[1], target[0] - source[0])
        tail = (
            int(point[0] - math.cos(angle) * 42),
            int(point[1] - math.sin(angle) * 42),
        )
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        pygame.draw.line(overlay, (*color, 110), tail, point, 9)
        pygame.draw.line(overlay, (255, 250, 210, 245), tail, point, 3)
        head_left = (
            int(point[0] - math.cos(angle - 0.55) * 15),
            int(point[1] - math.sin(angle - 0.55) * 15),
        )
        head_right = (
            int(point[0] - math.cos(angle + 0.55) * 15),
            int(point[1] - math.sin(angle + 0.55) * 15),
        )
        pygame.draw.polygon(
            overlay,
            (255, 247, 190, 245),
            [point, head_left, head_right],
        )
        if progress >= 0.70:
            radius = 10 + int((progress - 0.70) * 80)
            pygame.draw.circle(
                overlay,
                (255, 255, 235, 210),
                target,
                radius,
                3,
            )
        surface.blit(overlay, (0, 0))
        if progress >= 0.68:
            draw_text(
                surface,
                self.fonts.small,
                f"-{damage}",
                (target[0], target[1] - 66),
                self.palette.red_soft,
                "midbottom",
            )

    def _skill_two(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
        color: tuple[int, int, int],
        progress: float,
        healing: int,
    ) -> None:
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        pulse = math.sin(progress * math.pi)
        radius_x = 48 + int(34 * pulse)
        radius_y = 20 + int(15 * pulse)
        rect = pygame.Rect(
            center[0] - radius_x,
            center[1] - radius_y + 25,
            radius_x * 2,
            radius_y * 2,
        )
        pygame.draw.ellipse(overlay, (80, 255, 150, 70), rect)
        pygame.draw.ellipse(overlay, (255, 224, 100, 220), rect, 4)
        for index in range(9):
            phase = (progress * 1.7 + index / 9.0) % 1.0
            x = center[0] + int(math.sin(index * 2.4) * (20 + index * 2))
            y = center[1] + 18 - int(phase * 105)
            pygame.draw.circle(
                overlay,
                (130, 255, 170, int(230 * (1.0 - phase))),
                (x, y),
                3 + index % 3,
            )
        surface.blit(overlay, (0, 0))
        draw_text(
            surface,
            self.fonts.body,
            f"+{healing}",
            (center[0], center[1] - 78 - int(progress * 16)),
            (130, 255, 170),
            "midbottom",
        )

    def _skill_three(
        self,
        surface: pygame.Surface,
        source: tuple[int, int],
        target: tuple[int, int],
        color: tuple[int, int, int],
        progress: float,
        damage: int,
    ) -> None:
        overlay = pygame.Surface(surface.get_size(), pygame.SRCALPHA)
        beam_end = self._lerp(source, target, min(1.0, progress / 0.40))
        pygame.draw.line(overlay, (*color, 80), source, beam_end, 30)
        pygame.draw.line(
            overlay,
            (255, 255, 255, 225),
            source,
            beam_end,
            8,
        )
        if progress >= 0.34:
            blast = min(1.0, (progress - 0.34) / 0.66)
            for offset, alpha in ((0, 220), (22, 150), (44, 90)):
                pygame.draw.circle(
                    overlay,
                    (*color, alpha),
                    target,
                    int(24 + blast * 115 + offset),
                    7,
                )
            pygame.draw.circle(
                overlay,
                (255, 250, 220, 100),
                target,
                int(18 + blast * 75),
            )
        surface.blit(overlay, (0, 0))
        if progress >= 0.52:
            draw_text(
                surface,
                self.fonts.heading,
                f"-{damage}",
                (target[0], target[1] - 88),
                self.palette.red_soft,
                "midbottom",
            )

    def _hit(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
        remaining: float,
    ) -> None:
        for index in range(7):
            angle = index * 0.9 + remaining * 18.0
            radius = 24 + index * 3
            point = (
                int(center[0] + math.cos(angle) * radius),
                int(center[1] + math.sin(angle) * radius * 0.55),
            )
            pygame.draw.circle(surface, (255, 246, 220), point, 3)

    def _respawn(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
        color: tuple[int, int, int],
        remaining: float,
    ) -> None:
        glow = pygame.Surface((150, 82), pygame.SRCALPHA)
        alpha = int(120 * min(1.0, remaining / 1.2))
        pygame.draw.ellipse(glow, (*color, alpha), (10, 18, 130, 44), 4)
        surface.blit(glow, (center[0] - 75, center[1] - 34))

    def _switch(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
        now: float,
    ) -> None:
        radius = 45 + int(math.sin(now * 8.0) * 4)
        pygame.draw.circle(surface, self.palette.gold, center, radius, 3)

    def _offline(
        self,
        surface: pygame.Surface,
        center: tuple[int, int],
    ) -> None:
        pygame.draw.circle(surface, self.palette.gray, center, 48, 2)
        draw_text(
            surface,
            self.fonts.tiny,
            "离线",
            (center[0], center[1] + 52),
            self.palette.gray,
            "midtop",
        )

    @staticmethod
    def _lerp(
        source: tuple[int, int],
        target: tuple[int, int],
        progress: float,
    ) -> tuple[int, int]:
        return (
            int(source[0] + (target[0] - source[0]) * progress),
            int(source[1] + (target[1] - source[1]) * progress),
        )
