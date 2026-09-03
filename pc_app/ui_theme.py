from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import pygame


@dataclass(frozen=True, slots=True)
class Palette:
    background: tuple[int, int, int] = (6, 11, 18)
    map_dark: tuple[int, int, int] = (13, 30, 25)
    map_green: tuple[int, int, int] = (32, 82, 50)
    grass_light: tuple[int, int, int] = (50, 112, 62)
    lane_dark: tuple[int, int, int] = (64, 58, 51)
    lane_light: tuple[int, int, int] = (120, 112, 92)
    water: tuple[int, int, int] = (29, 91, 105)
    panel: tuple[int, int, int] = (15, 22, 34)
    border: tuple[int, int, int] = (82, 112, 135)
    text: tuple[int, int, int] = (241, 246, 250)
    muted: tuple[int, int, int] = (151, 166, 180)
    blue: tuple[int, int, int] = (58, 153, 255)
    blue_soft: tuple[int, int, int] = (103, 206, 255)
    red: tuple[int, int, int] = (238, 71, 82)
    red_soft: tuple[int, int, int] = (255, 132, 108)
    green: tuple[int, int, int] = (75, 219, 139)
    gold: tuple[int, int, int] = (245, 192, 76)
    gray: tuple[int, int, int] = (103, 113, 124)
    shadow: tuple[int, int, int] = (3, 6, 10)


@dataclass(slots=True)
class FontSet:
    tiny: pygame.font.Font
    small: pygame.font.Font
    body: pygame.font.Font
    heading: pygame.font.Font
    title: pygame.font.Font


def load_font(size: int) -> pygame.font.Font:
    """Load a font with Chinese glyph coverage when available."""

    candidates = [
        Path("C:/Windows/Fonts/msyh.ttc"),
        Path("C:/Windows/Fonts/msyhbd.ttc"),
        Path("C:/Windows/Fonts/simhei.ttf"),
        Path("C:/Windows/Fonts/simsun.ttc"),
    ]
    for path in candidates:
        if path.exists():
            return pygame.font.Font(str(path), size)
    return pygame.font.SysFont("Microsoft YaHei,SimHei,Arial", size)


def create_fonts() -> FontSet:
    return FontSet(
        tiny=load_font(12),
        small=load_font(14),
        body=load_font(17),
        heading=load_font(21),
        title=load_font(30),
    )


def clamp01(value: float) -> float:
    return max(0.0, min(1.0, value))


def draw_text(
    surface: pygame.Surface,
    font: pygame.font.Font,
    text: object,
    position: tuple[int, int],
    color: tuple[int, int, int],
    anchor: str = "topleft",
) -> pygame.Rect:
    image = font.render(str(text), True, color)
    rect = image.get_rect()
    setattr(rect, anchor, position)
    surface.blit(image, rect)
    return rect


def draw_glass_panel(
    surface: pygame.Surface,
    rect: pygame.Rect,
    palette: Palette,
    alpha: int = 210,
    radius: int = 8,
    border_color: tuple[int, int, int] | None = None,
) -> None:
    """Draw a translucent HUD panel."""

    panel = pygame.Surface(rect.size, pygame.SRCALPHA)
    pygame.draw.rect(
        panel,
        (*palette.panel, alpha),
        panel.get_rect(),
        border_radius=radius,
    )
    pygame.draw.rect(
        panel,
        (*(border_color or palette.border), min(255, alpha + 30)),
        panel.get_rect(),
        width=1,
        border_radius=radius,
    )
    surface.blit(panel, rect)


def hp_color(ratio: float, palette: Palette) -> tuple[int, int, int]:
    if ratio > 0.45:
        return palette.green
    if ratio > 0.22:
        return palette.gold
    return palette.red


def draw_hp_bar(
    surface: pygame.Surface,
    rect: pygame.Rect,
    ratio: float,
    palette: Palette,
    team_color: tuple[int, int, int] | None = None,
    delayed_ratio: float | None = None,
    offline: bool = False,
) -> None:
    ratio = clamp01(ratio)
    delayed_ratio = ratio if delayed_ratio is None else clamp01(delayed_ratio)
    frame = palette.gray if offline else (team_color or palette.border)
    pygame.draw.rect(surface, (18, 23, 28), rect, border_radius=4)
    inner = rect.inflate(-2, -2)
    if delayed_ratio > ratio:
        delay = inner.copy()
        delay.width = int(inner.width * delayed_ratio)
        pygame.draw.rect(surface, (220, 65, 60), delay, border_radius=3)
    fill = inner.copy()
    fill.width = int(inner.width * ratio)
    if fill.width > 0:
        color = palette.gray if offline else hp_color(ratio, palette)
        pygame.draw.rect(surface, color, fill, border_radius=3)
    pygame.draw.rect(surface, frame, rect, width=1, border_radius=4)


def fit_text(text: str, font: pygame.font.Font, max_width: int) -> str:
    if font.size(text)[0] <= max_width:
        return text
    suffix = "..."
    while text and font.size(text + suffix)[0] > max_width:
        text = text[:-1]
    return text + suffix
