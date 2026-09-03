from __future__ import annotations

import pygame

from arena_asset_loader import ArenaAssetLoader
from arena_map_renderer import ArenaMapRenderer
from game_model import GameModel
from ui_theme import FontSet, Palette


class MapView:
    """Compatibility wrapper for the ArenaofValor map renderer."""

    def __init__(self, palette: Palette, fonts: FontSet, assets: ArenaAssetLoader) -> None:
        self.renderer = ArenaMapRenderer(palette, fonts, assets)

    def draw(self, surface: pygame.Surface, rect: pygame.Rect, model: GameModel) -> None:
        self.renderer.draw(surface, rect, model)
