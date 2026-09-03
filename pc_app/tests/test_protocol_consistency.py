from __future__ import annotations

from pathlib import Path
import re
import unittest

import config
import game_rules
import protocol


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL_H = (
    ROOT / "firmware_ch32v307" / "App" / "game_protocol.h"
).read_text(encoding="utf-8")
CONFIG_H = (
    ROOT / "firmware_ch32v307" / "App" / "app_config.h"
).read_text(encoding="utf-8")
GAME_TYPES_H = (
    ROOT / "firmware_ch32v307" / "App" / "game_types.h"
).read_text(encoding="utf-8")
PROTOCOL_C = (
    ROOT / "firmware_ch32v307" / "App" / "game_protocol.c"
).read_text(encoding="utf-8")
GAME_MASTER_C = (
    ROOT / "firmware_ch32v307" / "App" / "game_master.c"
).read_text(encoding="utf-8")


def c_hex(name: str) -> int:
    match = re.search(
        rf"#define\s+{name}\s+(0x[0-9A-Fa-f]+)U?",
        PROTOCOL_H,
    )
    if match is None:
        raise AssertionError(f"missing C macro {name}")
    return int(match.group(1), 16)


def c_int(name: str) -> int:
    match = re.search(
        rf"#define\s+{name}\s+\(?(-?\d+)\)?(?:U|UL)?",
        CONFIG_H,
    )
    if match is None:
        raise AssertionError(f"missing C macro {name}")
    return int(match.group(1))


def c_enum(name: str) -> int:
    match = re.search(rf"\b{name}\s*=\s*(\d+)", GAME_TYPES_H)
    if match is None:
        raise AssertionError(f"missing C enum {name}")
    return int(match.group(1))


def c_function(name: str, next_name: str) -> str:
    start = PROTOCOL_C.index(f"void {name}")
    end = PROTOCOL_C.index(f"void {next_name}", start)
    return PROTOCOL_C[start:end]


class ProtocolConsistencyTests(unittest.TestCase):
    def test_all_can_ids_match_c_header(self) -> None:
        names = (
            "CAN_ID_ROLE_SWITCH",
            "CAN_ID_GAME_CTRL",
            "CAN_ID_MASTER_CLAIM_BASE",
            "CAN_ID_DEATH_EVENT",
            "CAN_ID_GLOBAL_STATE",
            "CAN_ID_POSITION_STATE",
            "CAN_ID_CRYSTAL_ATTACK",
            "CAN_ID_HERO_STATE_BASE",
            "CAN_ID_MOVE_INPUT_BASE",
            "CAN_ID_SKILL_INPUT_BASE",
            "CAN_ID_SKILL_RESULT_BASE",
            "CAN_ID_READY_BASE",
            "CAN_ID_HEARTBEAT_BASE",
        )
        for name in names:
            self.assertEqual(getattr(protocol, name), c_hex(name), name)

    def test_shared_crystal_and_movement_rules_match(self) -> None:
        self.assertEqual(c_int("CRYSTAL_DAMAGE"), game_rules.CRYSTAL_DAMAGE)
        self.assertEqual(c_int("WIN_SCORE"), game_rules.WIN_SCORE)
        self.assertEqual(
            c_int("SKILL1_RANGE_MAP_PIXELS"),
            game_rules.SKILL1_RANGE_MAP_PIXELS,
        )
        self.assertEqual(
            c_int("SKILL3_RANGE_MAP_PIXELS"),
            game_rules.SKILL3_RANGE_MAP_PIXELS,
        )
        self.assertEqual(
            c_int("POSITION_STATE_PERIOD_MS"),
            round(game_rules.POSITION_STATE_PERIOD_SECONDS * 1000),
        )
        self.assertEqual(
            c_int("CRYSTAL_ATTACK_RANGE_MAP_PIXELS") * 2,
            game_rules.CRYSTAL_ATTACK_RANGE_MAP_PIXELS,
        )
        self.assertEqual(
            c_int("CRYSTAL_ATTACK_MS"),
            round(
                game_rules.CRYSTAL_ATTACK_INTERVAL_SECONDS * 1000
            ),
        )
        self.assertEqual(
            c_int("MOVE_INPUT_REFRESH_MS"),
            round(config.MOVE_INPUT_REFRESH_SECONDS * 1000),
        )
        self.assertEqual(
            c_int("MOVE_STEP_X_PER_TICK"),
            round(
                game_rules.MOVE_STEP_X_PER_TICK
                * game_rules.WORLD_COORD_SCALE
            ),
        )
        self.assertEqual(
            c_int("MOVE_STEP_Y_PER_TICK"),
            round(
                game_rules.MOVE_STEP_Y_PER_TICK
                * game_rules.WORLD_COORD_SCALE
            ),
        )
        self.assertEqual(
            c_int("WORLD_MAP_X_PIXELS"),
            game_rules.WORLD_MAP_X_PIXELS,
        )
        self.assertEqual(
            c_int("WORLD_MAP_Y_PIXELS"),
            game_rules.WORLD_MAP_Y_PIXELS,
        )
        self.assertEqual(
            c_int("BLUE_CRYSTAL_X"),
            round(
                game_rules.BLUE_CRYSTAL_POSITION[0]
                * game_rules.WORLD_COORD_SCALE
            ),
        )
        self.assertEqual(
            c_int("RED_CRYSTAL_X"),
            round(
                game_rules.RED_CRYSTAL_POSITION[0]
                * game_rules.WORLD_COORD_SCALE
            ),
        )
        for c_name, python_value in (
            ("PC_SPAWN_X", game_rules.PC_SPAWN[0]),
            ("PC_SPAWN_Y", game_rules.PC_SPAWN[1]),
            ("EMBEDDED_SPAWN_X", game_rules.EMBEDDED_SPAWN[0]),
            ("EMBEDDED_SPAWN_Y", game_rules.EMBEDDED_SPAWN[1]),
            ("WORLD_X_MIN", game_rules.WORLD_X_BOUNDS[0]),
            ("WORLD_X_MAX", game_rules.WORLD_X_BOUNDS[1]),
            ("WORLD_Y_MIN", game_rules.WORLD_Y_BOUNDS[0]),
            ("WORLD_Y_MAX", game_rules.WORLD_Y_BOUNDS[1]),
        ):
            self.assertEqual(
                c_int(c_name),
                round(python_value * game_rules.WORLD_COORD_SCALE),
                c_name,
            )

    def test_all_protocol_enums_match_c(self) -> None:
        groups = (
            (
                protocol.NodeRole,
                {
                    "NONE": "ROLE_NONE",
                    "MASTER": "ROLE_MASTER",
                    "PLAYER": "ROLE_PLAYER",
                    "COOLDOWN": "ROLE_COOLDOWN",
                    "BACKUP": "ROLE_BACKUP",
                    "MASTER_PLAYER": "ROLE_MASTER_PLAYER",
                },
            ),
            (
                protocol.HeroState,
                {
                    "DEAD": "HERO_DEAD",
                    "ALIVE": "HERO_ALIVE",
                    "COOLDOWN": "HERO_COOLDOWN",
                    "READY": "HERO_READY",
                },
            ),
            (
                protocol.GameState,
                {
                    "IDLE": "GAME_IDLE",
                    "RUNNING": "GAME_RUNNING",
                    "PAUSED": "GAME_PAUSED",
                    "OVER": "GAME_OVER",
                },
            ),
            (
                protocol.SkillId,
                {
                    "NONE": "SKILL_NONE",
                    "SKILL_1": "SKILL_1",
                    "SKILL_2": "SKILL_2",
                    "SKILL_3": "SKILL_3",
                },
            ),
            (
                protocol.SkillResult,
                {
                    "ACCEPTED": "SKILL_RESULT_ACCEPTED",
                    "GAME_NOT_RUNNING": (
                        "SKILL_RESULT_GAME_NOT_RUNNING"
                    ),
                    "COOLDOWN": "SKILL_RESULT_COOLDOWN",
                    "NOT_CURRENT_PLAYER": (
                        "SKILL_RESULT_NOT_CURRENT_PLAYER"
                    ),
                    "OUT_OF_RANGE": "SKILL_RESULT_OUT_OF_RANGE",
                },
            ),
            (
                protocol.SwitchReason,
                {
                    "PLAYER_DEATH": "SWITCH_BY_PLAYER_DEATH",
                    "MASTER_TIMEOUT": "SWITCH_BY_MASTER_TIMEOUT",
                    "MANUAL_RESET": "SWITCH_BY_MANUAL_RESET",
                    "NODE_RECOVERY": "SWITCH_BY_NODE_RECOVERY",
                },
            ),
            (
                protocol.GameControl,
                {
                    "START": "GAME_CTRL_START",
                    "PAUSE": "GAME_CTRL_PAUSE",
                    "RESET": "GAME_CTRL_RESET",
                },
            ),
        )
        for enum_type, names in groups:
            for python_name, c_name in names.items():
                self.assertEqual(
                    int(getattr(enum_type, python_name)),
                    c_enum(c_name),
                    c_name,
                )

    def test_crystal_frame_field_order_matches_c_sender(self) -> None:
        sender = c_function(
            "Protocol_SendCrystalAttack",
            "Protocol_SendDeathEvent",
        )
        assignments = [
            "data[0] = crystal_id;",
            "data[1] = target_node;",
            "data[2] = damage;",
            "data[3] = hit_seq;",
            "data[4] = g_game_master.pc_hp;",
            "data[5] = g_game_master.embedded_hp;",
            "data[6] = g_node_role.current_term;",
            "data[7] = 0U;",
        ]
        positions = [sender.index(item) for item in assignments]
        self.assertEqual(positions, sorted(positions))

    def test_all_c_sender_field_orders(self) -> None:
        cases = (
            (
                "Protocol_SendHeartbeat",
                "Protocol_SendSkillInput",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = (uint8_t)NodeRole_GetHeartbeatRole();",
                    "data[2] = (uint8_t)g_node_role.self_hero_state;",
                    "data[3] = GameMaster_GetEmbeddedHp();",
                    "data[4] = g_node_role.current_term;",
                    "data[5] = g_node_role.cooldown_remaining_s;",
                    "data[6] = USE_JOYSTICK ?",
                    "data[7] = heartbeat_seq++;",
                ),
            ),
            (
                "Protocol_SendSkillInput",
                "Protocol_SendSkillResult",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = skill_id;",
                    "data[2] = target_id;",
                    "data[3] = 0U;",
                    "data[4] = 0U;",
                    "data[5] = sequence;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendSkillResult",
                "Protocol_SendMoveInput",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = source_node;",
                    "data[2] = skill_id;",
                    "data[3] = result;",
                    "data[4] = source_input_seq;",
                    "data[5] = g_game_master.pc_hp;",
                    "data[6] = g_game_master.embedded_hp;",
                    "data[7] = g_node_role.current_term;",
                ),
            ),
            (
                "Protocol_SendMoveInput",
                "Protocol_SendGlobalState",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = x_dir;",
                    "data[2] = y_dir;",
                    "data[3] = sequence;",
                    "data[4] = 0U;",
                    "data[5] = 0U;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendGlobalState",
                "Protocol_SendCrystalAttack",
                (
                    "data[0] = g_node_role.current_term;",
                    "data[1] = g_node_role.current_master;",
                    "data[2] = g_node_role.current_player;",
                    "data[3] = g_game_master.pc_hp;",
                    "data[4] = g_game_master.embedded_hp;",
                    "data[5] = g_game_master.pc_score;",
                    "data[6] = g_game_master.embedded_score;",
                    "data[7] = g_game_master.game_state;",
                ),
            ),
            (
                "Protocol_SendDeathEvent",
                "Protocol_SendRoleSwitch",
                (
                    "data[0] = dead_node;",
                    "data[1] = killer_node;",
                    "data[2] = cooldown_s;",
                    "data[3] = g_node_role.current_term;",
                    "data[4] = 0U;",
                    "data[5] = 0U;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendRoleSwitch",
                "Protocol_SendReady",
                (
                    "data[0] = g_node_role.current_term;",
                    "data[1] = new_master;",
                    "data[2] = new_player;",
                    "data[3] = reason;",
                    "data[4] = target_node;",
                    "data[5] = cooldown_s;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendReady",
                "Protocol_SendMasterClaim",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = 1U;",
                    "data[2] = g_node_role.current_term;",
                    "data[3] = 0U;",
                    "data[4] = 0U;",
                    "data[5] = 0U;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendMasterClaim",
                "Protocol_SendHeroState",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = claim_term;",
                    "data[2] = NODE_ID;",
                    "data[3] = (uint8_t)g_node_role.self_hero_state;",
                    "data[4] = 0U;",
                    "data[5] = 0U;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
            (
                "Protocol_SendHeroState",
                "Protocol_HandleRxFrame",
                (
                    "data[0] = NODE_ID;",
                    "data[1] = GameMaster_GetEmbeddedHp();",
                    "data[2] = (uint8_t)g_node_role.self_hero_state;",
                    "data[3] = (uint8_t)NodeRole_GetHeartbeatRole();",
                    "data[4] = g_node_role.current_term;",
                    "data[5] = g_node_role.cooldown_remaining_s;",
                    "data[6] = 0U;",
                    "data[7] = 0U;",
                ),
            ),
        )
        for function_name, next_name, assignments in cases:
            sender = c_function(function_name, next_name)
            positions = [sender.index(item) for item in assignments]
            self.assertEqual(
                positions,
                sorted(positions),
                function_name,
            )

    def test_position_state_uses_explicit_little_endian_helpers(self) -> None:
        sender = c_function(
            "Protocol_SendPositionState",
            "Protocol_SendCrystalAttack",
        )
        for expression in (
            "Protocol_WriteInt16LE(&data[0], g_game_master.pc_x);",
            "Protocol_WriteInt16LE(&data[2], g_game_master.pc_y);",
            "Protocol_WriteInt16LE(&data[4], g_game_master.embedded_x);",
            "Protocol_WriteInt16LE(&data[6], g_game_master.embedded_y);",
        ):
            self.assertIn(expression, sender)
        self.assertNotIn("(int16_t *)", PROTOCOL_C)

    def test_non_master_saves_position_and_does_not_integrate(self) -> None:
        self.assertIn(
            "void GameMaster_OnPositionState(const CanFrame_t *frame)",
            GAME_MASTER_C,
        )
        self.assertIn(
            "if(!NodeRole_IsMaster())",
            GAME_MASTER_C,
        )
        self.assertIn(
            "Protocol_ReadInt16LE(&frame->data[6])",
            GAME_MASTER_C,
        )


if __name__ == "__main__":
    unittest.main()
