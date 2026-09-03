from __future__ import annotations

from dataclasses import dataclass, field
import time

from game_rules import (
    SKILL_EFFECT_DURATIONS,
    WORLD_X_BOUNDS,
    WORLD_Y_BOUNDS,
)


def clamp(value: float, minimum: float, maximum: float) -> float:
    return max(minimum, min(maximum, value))


@dataclass(slots=True)
class HeroEntity:
    """地图中的英雄状态，坐标使用 0~1 的归一化地图坐标。"""

    role_label: str
    side: str
    spawn_x: float
    spawn_y: float
    team_color: tuple[int, int, int]
    x: float = 0.0
    y: float = 0.0
    alive: bool = True
    hp: int = 100
    max_hp: int = 100
    score: int = 0
    status: str = "待命"
    cooldowns: list[float] = field(default_factory=lambda: [0.0] * 4)
    respawn_at: float = 0.0
    hit_flash_until: float = 0.0
    action_flash_until: float = 0.0
    switch_flash_until: float = 0.0
    respawn_flash_until: float = 0.0
    active_skill_id: int = 0
    active_skill_started_at: float = 0.0
    active_skill_until: float = 0.0
    active_skill_source_node: int = 0
    active_skill_target_node: int = 0
    active_skill_value: int = 0

    def __post_init__(self) -> None:
        if self.x == 0.0:
            self.x = self.spawn_x
        if self.y == 0.0:
            self.y = self.spawn_y

    @property
    def hp_ratio(self) -> float:
        if self.max_hp <= 0:
            return 0.0
        return clamp(self.hp / self.max_hp, 0.0, 1.0)

    def set_hp(self, hp: int, now: float | None = None) -> None:
        now = time.monotonic() if now is None else now
        new_hp = max(0, min(self.max_hp, int(hp)))
        if not self.alive and self.respawn_at and new_hp > 0:
            # 总线可能在死亡事件后立即广播满血，地图仍保留完整死亡动画。
            return
        if 0 < new_hp < self.hp:
            self.hit_flash_until = now + 0.28
        self.hp = new_hp
        if self.hp <= 0 and self.alive:
            self.mark_dead(3.0, now)

    def mark_action(self, now: float | None = None) -> None:
        now = time.monotonic() if now is None else now
        self.action_flash_until = now + 0.35

    def start_skill(
        self,
        skill_id: int,
        source_node: int,
        target_node: int,
        value: int,
        now: float | None = None,
    ) -> None:
        now = time.monotonic() if now is None else now
        duration = SKILL_EFFECT_DURATIONS.get(int(skill_id), 0.8)
        self.active_skill_id = int(skill_id)
        self.active_skill_started_at = now
        self.active_skill_until = now + duration
        self.active_skill_source_node = int(source_node)
        self.active_skill_target_node = int(target_node)
        self.active_skill_value = int(value)
        self.action_flash_until = now + min(0.45, duration)

    def clear_skill(self) -> None:
        self.active_skill_id = 0
        self.active_skill_started_at = 0.0
        self.active_skill_until = 0.0
        self.active_skill_source_node = 0
        self.active_skill_target_node = 0
        self.active_skill_value = 0

    def mark_switch(self, now: float | None = None) -> None:
        now = time.monotonic() if now is None else now
        self.switch_flash_until = now + 1.5

    def mark_dead(
        self,
        respawn_seconds: float,
        now: float | None = None,
        status: str = "已阵亡",
    ) -> None:
        now = time.monotonic() if now is None else now
        self.alive = False
        self.hp = 0
        self.respawn_at = now + max(0.8, respawn_seconds)
        self.status = status

    def mark_respawn(self, now: float | None = None) -> None:
        now = time.monotonic() if now is None else now
        self.alive = True
        self.hp = self.max_hp
        self.x = self.spawn_x
        self.y = self.spawn_y
        self.respawn_at = 0.0
        self.respawn_flash_until = now + 1.2
        self.status = "战斗中"

    def reset(self) -> None:
        self.x = self.spawn_x
        self.y = self.spawn_y
        self.alive = True
        self.hp = self.max_hp
        self.score = 0
        self.status = "待命"
        self.respawn_at = 0.0
        self.hit_flash_until = 0.0
        self.action_flash_until = 0.0
        self.switch_flash_until = 0.0
        self.respawn_flash_until = 0.0
        self.clear_skill()
        for index in range(len(self.cooldowns)):
            self.cooldowns[index] = 0.0

    def update(self, now: float | None = None) -> None:
        now = time.monotonic() if now is None else now
        if not self.alive and self.respawn_at and now >= self.respawn_at:
            self.mark_respawn(now)
        if self.active_skill_id and now >= self.active_skill_until:
            self.clear_skill()

    def move(
        self,
        x_dir: float,
        y_dir: float,
        x_distance: float,
        y_distance: float | None = None,
    ) -> None:
        if not self.alive:
            return
        if y_distance is None:
            y_distance = x_distance
        self.x = clamp(
            self.x + x_dir * x_distance,
            WORLD_X_BOUNDS[0],
            WORLD_X_BOUNDS[1],
        )
        self.y = clamp(
            self.y + y_dir * y_distance,
            WORLD_Y_BOUNDS[0],
            WORLD_Y_BOUNDS[1],
        )

    def cooldown_remaining(self, skill_id: int, now: float | None = None) -> float:
        now = time.monotonic() if now is None else now
        return max(0.0, self.cooldowns[skill_id] - now)

    def respawn_remaining(self, now: float | None = None) -> float:
        now = time.monotonic() if now is None else now
        return max(0.0, self.respawn_at - now)

    def transient_status(self, now: float | None = None) -> str:
        now = time.monotonic() if now is None else now
        if not self.alive:
            return f"{self.status} {self.respawn_remaining(now):.1f}s"
        if now < self.switch_flash_until:
            return "控制节点切换"
        if now < self.respawn_flash_until:
            return "已复活"
        if now < self.hit_flash_until:
            return "受到攻击"
        if now < self.action_flash_until:
            return "技能释放"
        return self.status


@dataclass(slots=True)
class CrystalAttackEffect:
    crystal_id: int
    target_node: int
    damage: int
    hit_seq: int
    term: int
    started_at: float
    ends_at: float
