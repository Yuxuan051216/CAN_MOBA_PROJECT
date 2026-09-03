from __future__ import annotations

import os
from pathlib import Path
import time

os.environ.setdefault("SDL_VIDEODRIVER", "dummy")

import pygame

from game_model import GameModel
from game_ui import GameUI
from protocol import GameState, NODE_ID_BOARD_A, NODE_ID_BOARD_B


def main() -> None:
    model = GameModel()
    model.node_online[NODE_ID_BOARD_A] = True
    model.node_online[NODE_ID_BOARD_B] = True
    model.on_global_state(
        {
            "term": 1,
            "master": NODE_ID_BOARD_A,
            "player": NODE_ID_BOARD_B,
            "pc_hp": 100,
            "embedded_hp": 100,
            "pc_score": 1,
            "embedded_score": 1,
            "game_state": int(GameState.RUNNING),
        }
    )
    model.on_skill_result(
        {
            "master_node": NODE_ID_BOARD_A,
            "source_node": 0,
            "skill_id": 3,
            "result": 1,
            "input_seq": 7,
            "pc_hp": 100,
            "embedded_hp": 70,
            "term": 1,
        }
    )
    model.on_crystal_attack(
        {
            "crystal_id": 1,
            "target_node": NODE_ID_BOARD_B,
            "damage": 10,
            "hit_seq": 9,
            "pc_hp": 100,
            "embedded_hp": 60,
            "term": 1,
        }
    )

    now = time.monotonic()
    model.pc_hero.active_skill_started_at = now - 0.55
    model.pc_hero.active_skill_until = now + 0.60
    assert model.crystal_attack is not None
    model.crystal_attack.started_at = now - 0.40
    model.crystal_attack.ends_at = now + 0.35

    ui = GameUI(model)
    try:
        ui.draw()
        output = (
            Path(__file__).resolve().parents[1]
            / "artifacts"
            / "arena_crystal_effects.png"
        )
        output.parent.mkdir(parents=True, exist_ok=True)
        pygame.image.save(ui.screen, str(output))
        print(output)
    finally:
        ui.close()


if __name__ == "__main__":
    main()
