from __future__ import annotations

import pygame

from game_model import GameModel
from ui_theme import FontSet, Palette, draw_glass_panel, draw_text, fit_text


class LogPanel:
    """Small optional CAN log overlay."""

    def __init__(self, palette: Palette, fonts: FontSet) -> None:
        self.palette = palette
        self.fonts = fonts

    def draw(self, surface: pygame.Surface, rect: pygame.Rect, model: GameModel, visible: bool = False) -> None:
        if not visible:
            return
        draw_glass_panel(surface, rect, self.palette, alpha=118, radius=6, border_color=self.palette.border)
        draw_text(surface, self.fonts.tiny, "CAN", (rect.x + 8, rect.y + 5), self.palette.blue_soft)
        for index, line in enumerate(model.can_logs[-3:]):
            color = self.palette.blue_soft if " TX " in line else self.palette.muted
            if "死亡" in line or "OFFLINE" in line:
                color = self.palette.red_soft
            draw_text(
                surface,
                self.fonts.tiny,
                fit_text(line, self.fonts.tiny, rect.width - 16),
                (rect.x + 8, rect.y + 22 + index * 15),
                color,
            )
