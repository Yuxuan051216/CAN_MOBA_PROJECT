from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum
import time

CAN_ID_ROLE_SWITCH = 0x001
CAN_ID_GAME_CTRL = 0x010
CAN_ID_MASTER_CLAIM_BASE = 0x020
CAN_ID_DEATH_EVENT = 0x080
CAN_ID_GLOBAL_STATE = 0x100
CAN_ID_POSITION_STATE = 0x110
CAN_ID_CRYSTAL_ATTACK = 0x120
CAN_ID_HERO_STATE_BASE = 0x180
CAN_ID_MOVE_INPUT_BASE = 0x200
CAN_ID_SKILL_INPUT_BASE = 0x300
CAN_ID_SKILL_RESULT_BASE = 0x380
CAN_ID_READY_BASE = 0x400
CAN_ID_HEARTBEAT_BASE = 0x700

HEARTBEAT_FLAG_JOYSTICK_ENABLED = 0x01

NODE_ID_PC = 0
NODE_ID_BOARD_A = 1
NODE_ID_BOARD_B = 2
NODE_COUNT = 3


class NodeRole(IntEnum):
    NONE = 0
    MASTER = 1
    PLAYER = 2
    COOLDOWN = 3
    BACKUP = 4
    MASTER_PLAYER = 5


class HeroState(IntEnum):
    DEAD = 0
    ALIVE = 1
    COOLDOWN = 2
    READY = 3


class GameState(IntEnum):
    IDLE = 0
    RUNNING = 1
    PAUSED = 2
    OVER = 3


class SkillId(IntEnum):
    NONE = 0
    SKILL_1 = 1
    SKILL_2 = 2
    SKILL_3 = 3


class SkillResult(IntEnum):
    ACCEPTED = 1
    GAME_NOT_RUNNING = 2
    COOLDOWN = 3
    NOT_CURRENT_PLAYER = 4
    OUT_OF_RANGE = 5


class SwitchReason(IntEnum):
    PLAYER_DEATH = 1
    MASTER_TIMEOUT = 2
    MANUAL_RESET = 3
    NODE_RECOVERY = 4


class GameControl(IntEnum):
    START = 1
    PAUSE = 2
    RESET = 3


class CrystalId(IntEnum):
    BLUE = 1
    RED = 2


SKILL_COOLDOWN_SECONDS = {
    SkillId.SKILL_1: 1.0,
    SkillId.SKILL_2: 5.0,
    SkillId.SKILL_3: 10.0,
}


@dataclass(slots=True)
class CanFrame:
    can_id: int
    data: bytes
    timestamp: float = 0.0

    def __post_init__(self) -> None:
        if not 0 <= self.can_id <= 0x7FF:
            raise ValueError("CAN ID must be an 11-bit standard identifier")
        if len(self.data) != 8:
            raise ValueError("MOBA protocol frames must always use DLC=8")
        if not self.timestamp:
            self.timestamp = time.time()


def _frame(can_id: int, values: list[int]) -> CanFrame:
    if len(values) != 8:
        raise ValueError("protocol payload must contain exactly 8 bytes")
    return CanFrame(can_id, bytes(value & 0xFF for value in values))


def _pack_i16_le(value: int) -> list[int]:
    if not -32768 <= int(value) <= 32767:
        raise ValueError("signed 16-bit value out of range")
    raw = int(value) & 0xFFFF
    return [raw & 0xFF, (raw >> 8) & 0xFF]


def _unpack_i16_le(data: bytes, offset: int) -> int:
    raw = data[offset] | (data[offset + 1] << 8)
    return raw - 0x10000 if raw & 0x8000 else raw


def pack_heartbeat(
    node_id: int,
    role: int,
    hero_state: int,
    hp: int,
    term: int,
    cooldown_s: int = 0,
    fault_flags: int = 0,
    heartbeat_seq: int = 0,
) -> CanFrame:
    return _frame(
        CAN_ID_HEARTBEAT_BASE + node_id,
        [
            node_id,
            role,
            hero_state,
            hp,
            term,
            cooldown_s,
            fault_flags,
            heartbeat_seq,
        ],
    )


def pack_skill_input(
    node_id: int,
    skill_id: int,
    target_id: int,
    input_seq: int = 0,
    x: int = 0,
    y: int = 0,
) -> CanFrame:
    return _frame(
        CAN_ID_SKILL_INPUT_BASE + node_id,
        [node_id, skill_id, target_id, x, y, input_seq, 0, 0],
    )


def pack_skill_result(
    master_node: int,
    source_node: int,
    skill_id: int,
    result: int,
    input_seq: int,
    pc_hp: int,
    embedded_hp: int,
    term: int,
) -> CanFrame:
    return _frame(
        CAN_ID_SKILL_RESULT_BASE + master_node,
        [
            master_node,
            source_node,
            skill_id,
            result,
            input_seq,
            pc_hp,
            embedded_hp,
            term,
        ],
    )


def pack_move_input(
    node_id: int,
    x_dir: int,
    y_dir: int,
    input_seq: int = 0,
) -> CanFrame:
    if x_dir not in (-1, 0, 1) or y_dir not in (-1, 0, 1):
        raise ValueError("directions must be -1, 0 or 1")
    return _frame(
        CAN_ID_MOVE_INPUT_BASE + node_id,
        [node_id, x_dir + 1, y_dir + 1, input_seq, 0, 0, 0, 0],
    )


def pack_game_ctrl(command: int) -> CanFrame:
    return _frame(CAN_ID_GAME_CTRL, [command, NODE_ID_PC, 0, 0, 0, 0, 0, 0])


def pack_global_state(
    term: int,
    master: int,
    player: int,
    pc_hp: int,
    embedded_hp: int,
    pc_score: int,
    embedded_score: int,
    game_state: int,
) -> CanFrame:
    return _frame(
        CAN_ID_GLOBAL_STATE,
        [
            term,
            master,
            player,
            pc_hp,
            embedded_hp,
            pc_score,
            embedded_score,
            game_state,
        ],
    )


def pack_position_state(
    pc_x: int,
    pc_y: int,
    embedded_x: int,
    embedded_y: int,
) -> CanFrame:
    return _frame(
        CAN_ID_POSITION_STATE,
        [
            *_pack_i16_le(pc_x),
            *_pack_i16_le(pc_y),
            *_pack_i16_le(embedded_x),
            *_pack_i16_le(embedded_y),
        ],
    )


def pack_crystal_attack(
    crystal_id: int,
    target_node: int,
    damage: int,
    hit_seq: int,
    pc_hp: int,
    embedded_hp: int,
    term: int,
) -> CanFrame:
    return _frame(
        CAN_ID_CRYSTAL_ATTACK,
        [
            crystal_id,
            target_node,
            damage,
            hit_seq,
            pc_hp,
            embedded_hp,
            term,
            0,
        ],
    )


def pack_death_event(
    dead_node: int,
    killer_node: int,
    cooldown_s: int,
    term: int,
) -> CanFrame:
    return _frame(
        CAN_ID_DEATH_EVENT,
        [dead_node, killer_node, cooldown_s, term, 0, 0, 0, 0],
    )


def pack_role_switch(
    term: int,
    new_master: int,
    new_player: int,
    reason: int,
    target_node: int,
    cooldown_s: int,
) -> CanFrame:
    return _frame(
        CAN_ID_ROLE_SWITCH,
        [
            term,
            new_master,
            new_player,
            reason,
            target_node,
            cooldown_s,
            0,
            0,
        ],
    )


def pack_ready(node_id: int, term: int) -> CanFrame:
    return _frame(CAN_ID_READY_BASE + node_id, [node_id, 1, term, 0, 0, 0, 0, 0])


def pack_master_claim(
    node_id: int,
    requested_term: int,
    priority: int,
    hero_state: int,
) -> CanFrame:
    return _frame(
        CAN_ID_MASTER_CLAIM_BASE + node_id,
        [node_id, requested_term, priority, hero_state, 0, 0, 0, 0],
    )


def unpack_frame(frame: CanFrame) -> dict[str, int | str]:
    data = frame.data
    can_id = frame.can_id

    if can_id == CAN_ID_ROLE_SWITCH:
        return {
            "type": "role_switch",
            "term": data[0],
            "master": data[1],
            "player": data[2],
            "reason": data[3],
            "target_node": data[4],
            "cooldown_s": data[5],
        }
    if can_id == CAN_ID_GAME_CTRL:
        return {"type": "game_ctrl", "command": data[0], "source": data[1]}
    if can_id == CAN_ID_DEATH_EVENT:
        return {
            "type": "death",
            "dead_node": data[0],
            "killer_node": data[1],
            "cooldown_s": data[2],
            "term": data[3],
        }
    if can_id == CAN_ID_GLOBAL_STATE:
        return {
            "type": "global_state",
            "term": data[0],
            "master": data[1],
            "player": data[2],
            "pc_hp": data[3],
            "embedded_hp": data[4],
            "pc_score": data[5],
            "embedded_score": data[6],
            "game_state": data[7],
        }
    if can_id == CAN_ID_POSITION_STATE:
        return {
            "type": "position_state",
            "pc_x": _unpack_i16_le(data, 0),
            "pc_y": _unpack_i16_le(data, 2),
            "embedded_x": _unpack_i16_le(data, 4),
            "embedded_y": _unpack_i16_le(data, 6),
        }
    if can_id == CAN_ID_CRYSTAL_ATTACK:
        return {
            "type": "crystal_attack",
            "crystal_id": data[0],
            "target_node": data[1],
            "damage": data[2],
            "hit_seq": data[3],
            "pc_hp": data[4],
            "embedded_hp": data[5],
            "term": data[6],
        }

    for base, name in (
        (CAN_ID_HEARTBEAT_BASE, "heartbeat"),
        (CAN_ID_MASTER_CLAIM_BASE, "master_claim"),
        (CAN_ID_HERO_STATE_BASE, "hero_state"),
        (CAN_ID_MOVE_INPUT_BASE, "move_input"),
        (CAN_ID_SKILL_INPUT_BASE, "skill_input"),
        (CAN_ID_SKILL_RESULT_BASE, "skill_result"),
        (CAN_ID_READY_BASE, "ready"),
    ):
        node_id = can_id - base
        if 0 <= node_id < NODE_COUNT and data[0] == node_id:
            if name == "heartbeat":
                return {
                    "type": name,
                    "node_id": node_id,
                    "role": data[1],
                    "hero_state": data[2],
                    "hp": data[3],
                    "term": data[4],
                    "cooldown_s": data[5],
                    "fault_flags": data[6],
                    "heartbeat_seq": data[7],
                }
            if name == "master_claim":
                return {
                    "type": name,
                    "node_id": node_id,
                    "term": data[1],
                    "priority": data[2],
                    "hero_state": data[3],
                }
            if name == "hero_state":
                return {
                    "type": name,
                    "node_id": node_id,
                    "hp": data[1],
                    "hero_state": data[2],
                    "role": data[3],
                    "term": data[4],
                    "cooldown_s": data[5],
                }
            if name == "move_input":
                return {
                    "type": name,
                    "node_id": node_id,
                    "x_dir": data[1] - 1,
                    "y_dir": data[2] - 1,
                    "input_seq": data[3],
                }
            if name == "skill_input":
                return {
                    "type": name,
                    "node_id": node_id,
                    "skill_id": data[1],
                    "target_id": data[2],
                    "x": data[3],
                    "y": data[4],
                    "input_seq": data[5],
                }
            if name == "skill_result":
                return {
                    "type": name,
                    "master_node": node_id,
                    "source_node": data[1],
                    "skill_id": data[2],
                    "result": data[3],
                    "input_seq": data[4],
                    "pc_hp": data[5],
                    "embedded_hp": data[6],
                    "term": data[7],
                }
            return {
                "type": name,
                "node_id": node_id,
                "ready": data[1],
                "term": data[2],
            }

    return {"type": "unknown", "can_id": can_id}
