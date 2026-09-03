from __future__ import annotations

import json
from pathlib import Path
from typing import Any

import pygame


class AssetManager:
    """Load hero artwork once and cache scaled frames."""

    ACTIONS = ("idle", "move", "attack", "hit", "death", "respawn")

    def __init__(self) -> None:
        self.root = Path(__file__).resolve().parent / "assets"
        self._hero_frames: dict[str, dict[str, list[pygame.Surface]]] = {}
        self._scaled: dict[tuple[int, int], pygame.Surface] = {}
        self._warned: set[str] = set()

    def hero_frames(self, hero_key: str) -> dict[str, list[pygame.Surface]]:
        if hero_key not in self._hero_frames:
            self._hero_frames[hero_key] = self._load_hero(hero_key)
        return self._hero_frames[hero_key]

    def scaled(self, image: pygame.Surface, size: tuple[int, int]) -> pygame.Surface:
        key = (id(image), size[0] * 10000 + size[1])
        if key not in self._scaled:
            self._scaled[key] = pygame.transform.smoothscale(image, size)
        return self._scaled[key]

    def _warn_once(self, key: str, message: str) -> None:
        if key in self._warned:
            return
        self._warned.add(key)
        print(f"[CAN MOBA][assets] {message}")

    def _load_hero(self, hero_key: str) -> dict[str, list[pygame.Surface]]:
        hero_dir = self.root / "heroes" / hero_key
        frames: dict[str, list[pygame.Surface]] = {}
        try:
            frames.update(self._load_sequences(hero_dir))
            frames.update(self._load_spritesheet(hero_dir, hero_key))
            single = self._load_single(hero_dir, hero_key)
            if single is not None:
                for action in self.ACTIONS:
                    frames.setdefault(action, [single])
        except (pygame.error, OSError, json.JSONDecodeError) as exc:
            self._warn_once(hero_key, f"{hero_key} 素材加载失败，使用 fallback：{exc}")
        if not frames:
            self._warn_once(
                hero_key,
                f"缺少 assets/heroes/{hero_key}/ 资源，已使用程序化 fallback",
            )
            fallback = self._fallback_hero(hero_key)
            frames = {action: [fallback] for action in self.ACTIONS}
        for action in self.ACTIONS:
            frames.setdefault(action, frames.get("idle", [self._fallback_hero(hero_key)]))
        return frames

    def _load_sequences(self, hero_dir: Path) -> dict[str, list[pygame.Surface]]:
        frames: dict[str, list[pygame.Surface]] = {}
        for action in self.ACTIONS:
            action_dir = hero_dir / action
            if not action_dir.exists():
                continue
            images = [
                pygame.image.load(str(path)).convert_alpha()
                for path in sorted(action_dir.glob("*.png"))
            ]
            if images:
                frames[action] = images
        return frames

    def _load_spritesheet(
        self, hero_dir: Path, hero_key: str
    ) -> dict[str, list[pygame.Surface]]:
        sheet_path = hero_dir / f"{hero_key}_spritesheet.png"
        meta_path = hero_dir / f"{hero_key}_spritesheet.json"
        if not sheet_path.exists() or not meta_path.exists():
            return {}
        sheet = pygame.image.load(str(sheet_path)).convert_alpha()
        data: dict[str, Any] = json.loads(meta_path.read_text(encoding="utf-8"))
        frames: dict[str, list[pygame.Surface]] = {}
        for action, rects in data.get("animations", data).items():
            if action not in self.ACTIONS:
                continue
            action_frames = []
            for rect in rects:
                x, y, w, h = [int(v) for v in rect]
                frame = pygame.Surface((w, h), pygame.SRCALPHA)
                frame.blit(sheet, (0, 0), pygame.Rect(x, y, w, h))
                action_frames.append(frame)
            if action_frames:
                frames[action] = action_frames
        return frames

    def _load_single(self, hero_dir: Path, hero_key: str) -> pygame.Surface | None:
        for name in (f"{hero_key}.png", "hero.png", "idle.png"):
            path = hero_dir / name
            if path.exists():
                return pygame.image.load(str(path)).convert_alpha()
        return None

    def _fallback_hero(self, hero_key: str) -> pygame.Surface:
        surf = pygame.Surface((120, 132), pygame.SRCALPHA)
        if hero_key == "maodie":
            body = (174, 147, 124)
            accent = (98, 70, 55)
            pygame.draw.ellipse(surf, body, (25, 34, 70, 78))
            pygame.draw.polygon(surf, body, [(35, 42), (47, 18), (58, 46)])
            pygame.draw.polygon(surf, body, [(75, 42), (87, 18), (96, 48)])
            pygame.draw.circle(surf, (36, 32, 30), (48, 67), 5)
            pygame.draw.circle(surf, (36, 32, 30), (74, 67), 5)
            pygame.draw.circle(surf, (242, 210, 192), (61, 83), 12)
            pygame.draw.line(surf, accent, (61, 82), (61, 91), 2)
            pygame.draw.arc(surf, accent, (51, 82, 20, 16), 0, 3.14, 2)
            pygame.draw.line(surf, accent, (30, 78), (10, 70), 2)
            pygame.draw.line(surf, accent, (90, 78), (111, 70), 2)
        else:
            body = (250, 214, 78)
            accent = (245, 139, 60)
            pygame.draw.ellipse(surf, body, (26, 28, 68, 84))
            pygame.draw.ellipse(surf, (255, 236, 138), (38, 52, 44, 42))
            pygame.draw.circle(surf, (45, 36, 25), (49, 61), 5)
            pygame.draw.circle(surf, (45, 36, 25), (72, 61), 5)
            pygame.draw.arc(surf, (45, 36, 25), (51, 68, 20, 13), 0, 3.14, 2)
            pygame.draw.circle(surf, accent, (34, 78), 8)
            pygame.draw.circle(surf, accent, (87, 78), 8)
            pygame.draw.polygon(surf, (255, 229, 111), [(44, 32), (60, 8), (76, 32)])
        pygame.draw.ellipse(surf, (0, 0, 0, 40), (22, 104, 78, 16))
        return surf
