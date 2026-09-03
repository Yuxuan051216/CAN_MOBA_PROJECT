from __future__ import annotations

import re
from pathlib import Path

import pygame


DIRECTIONS = (
    "down",
    "downLeft",
    "left",
    "upLeft",
    "up",
    "upRight",
    "right",
    "downRight",
)


class ArenaAssetLoader:
    """Load and cache ArenaofValor visual resources."""

    def __init__(self) -> None:
        self.root = Path(__file__).resolve().parent / "assets" / "reference_arena"
        self._images: dict[Path, pygame.Surface] = {}
        self._scaled: dict[tuple[int, int, int], pygame.Surface] = {}
        self._animations: dict[str, dict[str, dict[str, list[pygame.Surface]]]] = {}
        self._validate_root()

    def _validate_root(self) -> None:
        required = [
            self.root / "LICENSE",
            self.root / "SOURCE.txt",
            self.root / "map" / "battle_map.png",
            self.root / "buildings" / "blueShuiJin.png",
            self.root / "buildings" / "redShuiJin.png",
            self.root / "heroes" / "HouYi",
            self.root / "heroes" / "YaSe",
        ]
        missing = [str(path) for path in required if not path.exists()]
        if missing:
            raise FileNotFoundError(
                "ArenaofValor reference resources are missing:\n" + "\n".join(missing)
            )

    def image(self, relative: str) -> pygame.Surface:
        path = self.root / relative
        if path not in self._images:
            self._images[path] = pygame.image.load(str(path)).convert_alpha()
        return self._images[path]

    def scaled(self, image: pygame.Surface, size: tuple[int, int]) -> pygame.Surface:
        key = (id(image), size[0], size[1])
        if key not in self._scaled:
            self._scaled[key] = pygame.transform.smoothscale(image, size)
        return self._scaled[key]

    def hero_animations(self, hero_name: str) -> dict[str, dict[str, list[pygame.Surface]]]:
        if hero_name not in self._animations:
            self._animations[hero_name] = self._load_hero(hero_name)
        return self._animations[hero_name]

    def skill_icon(self, hero_name: str, skill_id: int) -> pygame.Surface:
        return self.image(f"heroes/{hero_name}/{hero_name}Skill{skill_id}.png")

    def portrait(self, hero_name: str) -> pygame.Surface:
        if hero_name == "HouYi":
            return self.image("ui/HouYiLogo.png")
        return self.image("ui/YaSeleft.png")

    def _load_hero(self, hero_name: str) -> dict[str, dict[str, list[pygame.Surface]]]:
        hero_dir = self.root / "heroes" / hero_name
        result: dict[str, dict[str, list[pygame.Surface]]] = {
            "move": {},
            "attack": {},
        }
        for direction in DIRECTIONS:
            result["move"][direction] = self._load_sequence(
                hero_dir,
                rf"^{re.escape(hero_name)}{re.escape(direction)}(\d+)\.png$",
            )
            result["attack"][direction] = self._load_sequence(
                hero_dir,
                rf"^{re.escape(hero_name)}Attack{self._attack_token(direction)}(\d+)\.png$",
            )
        missing = [
            f"{state}/{direction}"
            for state, groups in result.items()
            for direction, frames in groups.items()
            if not frames
        ]
        if missing:
            raise FileNotFoundError(f"{hero_name} animation frames missing: {', '.join(missing)}")
        return result

    def _load_sequence(self, directory: Path, pattern: str) -> list[pygame.Surface]:
        regex = re.compile(pattern, re.IGNORECASE)
        matched: list[tuple[int, Path]] = []
        for path in directory.glob("*.png"):
            match = regex.match(path.name)
            if match:
                matched.append((int(match.group(1)), path))
        return [
            pygame.image.load(str(path)).convert_alpha()
            for _, path in sorted(matched, key=lambda item: item[0])
        ]

    @staticmethod
    def _attack_token(direction: str) -> str:
        return {
            "down": "Down",
            "downLeft": "DownLeft",
            "left": "Left",
            "upLeft": "UpLeft",
            "up": "Up",
            "upRight": "UpRight",
            "right": "Right",
            "downRight": "DownRight",
        }[direction]
