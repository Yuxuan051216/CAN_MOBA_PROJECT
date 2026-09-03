from __future__ import annotations

# Shared game-rule values. Matching integer definitions live in
# firmware_ch32v307/App/app_config.h and are checked by the tests.
WORLD_COORD_SCALE = 1000
WORLD_MAP_X_PIXELS = 3880
WORLD_MAP_Y_PIXELS = 330

PC_SPAWN = (0.290, 0.160)
EMBEDDED_SPAWN = (0.710, -0.160)
WORLD_X_BOUNDS = (0.120, 0.880)
WORLD_Y_BOUNDS = (-0.620, 0.620)
MASTER_TICK_SECONDS = 0.050
POSITION_STATE_PERIOD_SECONDS = 0.100
POSITION_PREDICTION_TIMEOUT_SECONDS = 0.250
MOVE_STEP_X_PER_TICK = 0.005
MOVE_STEP_Y_PER_TICK = 0.059
MOVE_SPEED_X_PER_SECOND = MOVE_STEP_X_PER_TICK / MASTER_TICK_SECONDS
MOVE_SPEED_Y_PER_SECOND = MOVE_STEP_Y_PER_TICK / MASTER_TICK_SECONDS

CRYSTAL_BLUE = 1
CRYSTAL_RED = 2
BLUE_CRYSTAL_POSITION = (0.193, 0.0)
RED_CRYSTAL_POSITION = (0.807, 0.0)
CRYSTAL_ATTACK_RANGE_MAP_PIXELS = 234 * 2
CRYSTAL_ATTACK_INTERVAL_SECONDS = 1.0
CRYSTAL_DAMAGE = 10
WIN_SCORE = 3

SKILL1_RANGE_MAP_PIXELS = 700
SKILL3_RANGE_MAP_PIXELS = 1400


def distance_map_pixels(
    first: tuple[float, float] | list[float],
    second: tuple[float, float] | list[float],
) -> float:
    dx = (first[0] - second[0]) * WORLD_MAP_X_PIXELS
    dy = (first[1] - second[1]) * WORLD_MAP_Y_PIXELS
    return (dx * dx + dy * dy) ** 0.5


def skill_in_range(
    skill_id: int,
    source: tuple[float, float] | list[float],
    target: tuple[float, float] | list[float],
) -> bool:
    if int(skill_id) == 2:
        return True
    limit = (
        SKILL1_RANGE_MAP_PIXELS
        if int(skill_id) == 1
        else SKILL3_RANGE_MAP_PIXELS
    )
    return distance_map_pixels(source, target) <= limit

SKILL_EFFECT_DURATIONS = {
    1: 0.80,
    2: 1.05,
    3: 1.15,
}
CRYSTAL_EFFECT_DURATION = 0.75
