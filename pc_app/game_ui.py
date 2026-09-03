from __future__ import annotations

import pygame

import config
from arena_asset_loader import ArenaAssetLoader
from game_model import GameModel
from hud import Hud
from log_panel import LogPanel
from map_view import MapView
from ui_theme import Palette, create_fonts


class GameUI:
    """Compose the reference ArenaofValor battle scene."""

    def __init__(self, model: GameModel) -> None:
        pygame.init()
        if config.CAN_BACKEND.lower() == "mock":
            mode = "MOCK DEMO AI" if config.DEMO_AI else "MOCK"
        else:
            mode = "REAL CAN"
        pygame.display.set_caption(f"{config.GAME_TITLE} [{mode}]")
        self.screen = pygame.display.set_mode(
            (config.WINDOW_WIDTH, config.WINDOW_HEIGHT),
            pygame.RESIZABLE,
        )
        self.clock = pygame.time.Clock()
        self.model = model
        self.palette = Palette()
        self.fonts = create_fonts()
        self.assets = ArenaAssetLoader()
        self.map_view = MapView(self.palette, self.fonts, self.assets)
        self.hud = Hud(self.palette, self.fonts, self.assets)
        self.log_panel = LogPanel(self.palette, self.fonts)
        self.show_logs = False
        self.map_rect = self._layout(self.screen.get_size())

    def _layout(self, size: tuple[int, int]) -> pygame.Rect:
        width, height = size
        return pygame.Rect(0, 0, width, height)

    def draw(self) -> None:
        self.screen.fill(self.palette.background)
        self.map_rect = self._layout(self.screen.get_size())
        self.map_view.draw(self.screen, self.map_rect, self.model)
        self.hud.draw(self.screen, self.map_rect, self.model)
        self.log_panel.draw(self.screen, self.hud.log_rect(self.map_rect), self.model, self.show_logs)
        pygame.display.flip()

    def poll_events(self):
        events = pygame.event.get()
        for event in events:
            if event.type == pygame.VIDEORESIZE:
                self.screen = pygame.display.set_mode(event.size, pygame.RESIZABLE)
                self.map_rect = self._layout(event.size)
            elif event.type == pygame.KEYDOWN and event.key == pygame.K_F3:
                self.show_logs = not self.show_logs
        return events

    def tick(self) -> None:
        self.clock.tick(config.FPS)

    def close(self) -> None:
        pygame.quit()
