from __future__ import annotations

import os
from pathlib import Path

os.environ.setdefault("SDL_VIDEODRIVER", "dummy")
os.environ.setdefault("CAN_MOBA_BACKEND", "mock")

import pygame

import config
from game_model import GameModel
from game_ui import GameUI


def main() -> None:
    """Render a headless preview of the ArenaofValor battle UI."""

    pygame.init()
    model = GameModel()
    ui = GameUI(model)
    ui.draw()
    output = Path(__file__).resolve().parent.parent / "artifacts" / "arena_ui_preview.png"
    output.parent.mkdir(parents=True, exist_ok=True)
    pygame.image.save(ui.screen, str(output))
    ui.close()
    print(output)


if __name__ == "__main__":
    main()
