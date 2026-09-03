from __future__ import annotations

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class MapSourceTests(unittest.TestCase):
    def test_renderer_does_not_load_or_draw_towers(self) -> None:
        renderer = (ROOT / "arena_map_renderer.py").read_text(
            encoding="utf-8"
        )
        loader = (ROOT / "arena_asset_loader.py").read_text(
            encoding="utf-8"
        )
        self.assertNotIn("blue_tower", renderer)
        self.assertNotIn("red_tower", renderer)
        self.assertNotIn("blue_tower", loader)
        self.assertNotIn("red_tower", loader)
        self.assertIn("blueShuiJin.png", renderer)
        self.assertIn("redShuiJin.png", renderer)


if __name__ == "__main__":
    unittest.main()
