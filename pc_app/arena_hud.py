from __future__ import annotations

from dataclasses import dataclass
import math
import time

import pygame

from arena_asset_loader import ArenaAssetLoader
from game_model import GameModel
from protocol import NODE_ID_BOARD_A, NODE_ID_BOARD_B, NODE_ID_PC, SKILL_COOLDOWN_SECONDS, SkillId
from ui_theme import FontSet, Palette, draw_glass_panel, draw_hp_bar, draw_text, fit_text


@dataclass(frozen=True, slots=True)
class ArenaLayout:
    viewport: pygame.Rect
    top_score_rect: pygame.Rect
    left_portrait_rect: pygame.Rect
    right_portrait_rect: pygame.Rect
    pc_skill_rects: tuple[pygame.Rect, pygame.Rect, pygame.Rect]
    embedded_skill_rects: tuple[pygame.Rect, pygame.Rect, pygame.Rect]
    network_status_rect: pygame.Rect
    log_rect: pygame.Rect

    @classmethod
    def for_viewport(cls, viewport: pygame.Rect) -> "ArenaLayout":
        skill_size = max(54, min(68, viewport.width // 15))
        gap = max(10, skill_size // 5)
        group_width = skill_size * 3 + gap * 2
        skill_y = viewport.bottom - skill_size - 32
        pc_x = viewport.left + 18
        embedded_x = viewport.right - group_width - 18

        def skill_group(start_x: int) -> tuple[pygame.Rect, pygame.Rect, pygame.Rect]:
            return tuple(
                pygame.Rect(
                    start_x + index * (skill_size + gap),
                    skill_y,
                    skill_size,
                    skill_size,
                )
                for index in range(3)
            )

        return cls(
            viewport=viewport,
            top_score_rect=pygame.Rect(viewport.centerx - 150, viewport.top + 10, 300, 42),
            left_portrait_rect=pygame.Rect(viewport.left + 12, viewport.top + 12, 210, 54),
            right_portrait_rect=pygame.Rect(viewport.right - 222, viewport.top + 12, 210, 54),
            pc_skill_rects=skill_group(pc_x),
            embedded_skill_rects=skill_group(embedded_x),
            network_status_rect=pygame.Rect(viewport.centerx - 180, viewport.bottom - 24, 360, 18),
            log_rect=pygame.Rect(viewport.left + 12, viewport.bottom - 104, 410, 68),
        )


class ArenaHud:
    """Reference-style battle HUD connected to existing GameModel state."""

    def __init__(self, palette: Palette, fonts: FontSet, assets: ArenaAssetLoader) -> None:
        self.palette = palette
        self.fonts = fonts
        self.assets = assets
        self.houyi_portrait = assets.portrait("HouYi")
        self.yase_portrait = assets.portrait("YaSe")
        self.pc_skill_icons = {
            1: assets.skill_icon("HouYi", 1),
            2: assets.skill_icon("HouYi", 2),
            3: assets.skill_icon("HouYi", 3),
        }
        self.embedded_skill_icons = {
            1: assets.skill_icon("YaSe", 1),
            2: assets.skill_icon("YaSe", 2),
            3: assets.skill_icon("YaSe", 3),
        }

    def layout(self, viewport: pygame.Rect) -> ArenaLayout:
        return ArenaLayout.for_viewport(viewport)

    def draw(self, surface: pygame.Surface, layout: ArenaLayout, model: GameModel) -> None:
        self._score(surface, layout.top_score_rect, model)
        self._portrait(surface, layout.left_portrait_rect, self.houyi_portrait, "后羿", model.pc_hero, self.palette.blue)
        self._portrait(surface, layout.right_portrait_rect, self.yase_portrait, "亚瑟", model.embedded_hero, self.palette.red, True)
        self._skills(
            surface,
            layout.pc_skill_rects,
            model,
            "pc",
            self.pc_skill_icons,
            ("J", "K", "L"),
            self.palette.blue,
            "PC",
        )
        self._skills(
            surface,
            layout.embedded_skill_rects,
            model,
            "embedded",
            self.embedded_skill_icons,
            ("PB0", "PB1", "PB2"),
            self.palette.red,
            "BOARD",
        )
        self._network(surface, layout.network_status_rect, model)

    def _score(self, surface: pygame.Surface, rect: pygame.Rect, model: GameModel) -> None:
        draw_glass_panel(surface, rect, self.palette, alpha=150, radius=6)
        draw_text(surface, self.fonts.heading, str(model.pc_score), (rect.x + 44, rect.y + 5), self.palette.blue, "midtop")
        draw_text(surface, self.fonts.small, model.outcome_text, (rect.centerx, rect.y + 4), self.palette.text, "midtop")
        draw_text(surface, self.fonts.heading, str(model.embedded_score), (rect.right - 44, rect.y + 5), self.palette.red, "midtop")
        info = f"{model.backend_label}  Master {model.master_label}  Term {model.term}"
        draw_text(surface, self.fonts.tiny, fit_text(info, self.fonts.tiny, rect.width - 18), (rect.centerx, rect.y + 25), self.palette.muted, "midtop")

    def _portrait(
        self,
        surface: pygame.Surface,
        rect: pygame.Rect,
        portrait: pygame.Surface,
        name: str,
        hero,
        color: tuple[int, int, int],
        right: bool = False,
    ) -> None:
        draw_glass_panel(surface, rect, self.palette, alpha=135, radius=6, border_color=color)
        icon = self.assets.scaled(portrait, (42, 42))
        icon_rect = icon.get_rect(midleft=(rect.x + 7, rect.centery))
        if right:
            icon_rect = icon.get_rect(midright=(rect.right - 7, rect.centery))
        surface.blit(icon, icon_rect)
        text_x = icon_rect.left - 8 if right else icon_rect.right + 8
        anchor = "topright" if right else "topleft"
        draw_text(surface, self.fonts.small, name, (text_x, rect.y + 7), color, anchor)
        hp = pygame.Rect(rect.x + 56, rect.y + 31, rect.width - 66, 8)
        if right:
            hp = pygame.Rect(rect.x + 10, rect.y + 31, rect.width - 66, 8)
        draw_hp_bar(surface, hp, hero.hp_ratio, self.palette, color)

    def _skills(
        self,
        surface: pygame.Surface,
        rects: tuple[pygame.Rect, pygame.Rect, pygame.Rect],
        model: GameModel,
        side: str,
        icons: dict[int, pygame.Surface],
        labels: tuple[str, str, str],
        color: tuple[int, int, int],
        title: str,
    ) -> None:
        title_x = (rects[0].left + rects[-1].right) // 2
        draw_text(
            surface,
            self.fonts.tiny,
            title,
            (title_x, rects[0].top - 18),
            color,
            "midtop",
        )
        for rect, skill_id in zip(rects, (1, 2, 3)):
            self._skill(
                surface,
                rect,
                model,
                skill_id,
                side,
                icons[skill_id],
                labels[skill_id - 1],
                color,
            )

    def _skill(
        self,
        surface: pygame.Surface,
        rect: pygame.Rect,
        model: GameModel,
        skill_id: int,
        side: str,
        skill_icon: pygame.Surface,
        label: str,
        color: tuple[int, int, int],
    ) -> None:
        center = rect.center
        radius = rect.width // 2
        icon = self.assets.scaled(skill_icon, (rect.width - 12, rect.height - 12))
        pygame.draw.circle(surface, (6, 10, 16, 185), center, radius)
        surface.blit(icon, icon.get_rect(center=center))
        pygame.draw.circle(surface, color, center, radius, 2)
        hero = model.pc_hero if side == "pc" else model.embedded_hero
        remaining = model.skill_cooldown_remaining(side, skill_id)
        pending = side == "pc" and model.skill_pending(skill_id)
        disabled = (
            not hero.alive
            or model.game_state_name != "RUNNING"
            or (side == "embedded" and not model.embedded_online)
        )
        if remaining > 0.0 or pending or disabled:
            cover = pygame.Surface(rect.size, pygame.SRCALPHA)
            pygame.draw.circle(cover, (0, 0, 0, 148), (radius, radius), radius)
            surface.blit(cover, rect)
        if remaining > 0.0:
            draw_text(surface, self.fonts.heading, f"{remaining:.1f}", center, self.palette.text, "center")
        elif pending:
            draw_text(surface, self.fonts.tiny, "CAN", center, self.palette.gold, "center")
        elif disabled:
            draw_text(surface, self.fonts.tiny, "锁定", center, self.palette.gray, "center")
        else:
            glow = 90 + int(math.sin(time.monotonic() * 6.0) * 45)
            pygame.draw.circle(surface, (*color, glow), center, radius + 3, 2)
        draw_text(
            surface,
            self.fonts.small,
            label,
            (rect.centerx, rect.bottom + 2),
            self.palette.text,
            "midtop",
        )

    def _network(self, surface: pygame.Surface, rect: pygame.Rect, model: GameModel) -> None:
        draw_glass_panel(surface, rect, self.palette, alpha=112, radius=4)
        def dot(node_id: int) -> str:
            return "●" if model.node_online[node_id] else "○"
        text = (
            f"PC {dot(NODE_ID_PC)}  A {dot(NODE_ID_BOARD_A)}  B {dot(NODE_ID_BOARD_B)}  "
            f"Master {model.master_label}  Player {model.controlled_board_label}  Term {model.term}"
        )
        draw_text(surface, self.fonts.tiny, text, (rect.x + 8, rect.centery), self.palette.text, "midleft")

    def _joystick(self, surface: pygame.Surface, viewport: pygame.Rect, model: GameModel) -> None:
        base = (viewport.left + 78, viewport.bottom - 86)
        pygame.draw.circle(surface, (0, 0, 0, 80), base, 44, 2)
        knob = base
        if getattr(model, "pc_move", None):
            dx, dy = model.pc_move
            knob = (base[0] + int(dx * 22), base[1] - int(dy * 22))
        pygame.draw.circle(surface, (220, 225, 235, 125), knob, 14)
