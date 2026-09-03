from __future__ import annotations

import pygame

from arena_asset_loader import ArenaAssetLoader
from arena_hud import ArenaHud, ArenaLayout
from game_model import GameModel
from ui_theme import FontSet, Palette


class Hud:
    """Compatibility wrapper for the ArenaofValor battle HUD."""

    def __init__(self, palette: Palette, fonts: FontSet, assets: ArenaAssetLoader) -> None:
        self.renderer = ArenaHud(palette, fonts, assets)
        self._layout: ArenaLayout | None = None

    def draw(self, surface: pygame.Surface, map_rect: pygame.Rect, model: GameModel) -> None:
        self._layout = self.renderer.layout(map_rect)
        self.renderer.draw(surface, self._layout, model)

    def log_rect(self, map_rect: pygame.Rect) -> pygame.Rect:
        layout = self._layout or self.renderer.layout(map_rect)
        return layout.log_rect
