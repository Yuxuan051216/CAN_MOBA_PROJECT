from __future__ import annotations

from can_driver import create_can_bus
from game_model import GameModel
from game_ui import GameUI
from pc_player import PcPlayer


def main() -> None:
    bus = create_can_bus()
    model = GameModel()
    mode_message = model.mode_notice
    print(f"[CAN MOBA] {mode_message}")
    model.log.add_text(mode_message)
    ui = GameUI(model)
    pc_player = PcPlayer(bus, model)
    running = True

    try:
        while running:
            running = pc_player.handle_events(ui.poll_events())
            pc_player.update()

            while True:
                frame = bus.recv(timeout=0.0)
                if frame is None:
                    break
                model.handle_frame(frame)

            model.update_timeouts()
            model.update_visuals()
            ui.draw()
            ui.tick()
    finally:
        bus.close()
        ui.close()


if __name__ == "__main__":
    main()
