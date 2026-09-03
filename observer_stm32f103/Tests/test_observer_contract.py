from __future__ import annotations

from pathlib import Path
import re
import unittest


OBSERVER = Path(__file__).resolve().parents[1]
ROOT = OBSERVER.parent


class ObserverContractTests(unittest.TestCase):
    def test_protocol_ids_match_authoritative_header(self) -> None:
        source = (
            ROOT / "firmware_ch32v307" / "App" / "game_protocol.h"
        ).read_text(encoding="utf-8")
        observer = (
            OBSERVER / "Core" / "Inc" / "can_moba_protocol.h"
        ).read_text(encoding="utf-8")

        for name in (
            "CAN_ID_ROLE_SWITCH",
            "CAN_ID_GAME_CTRL",
            "CAN_ID_DEATH_EVENT",
            "CAN_ID_GLOBAL_STATE",
            "CAN_ID_POSITION_STATE",
            "CAN_ID_CRYSTAL_ATTACK",
            "CAN_ID_SKILL_RESULT_BASE",
        ):
            expected = re.search(
                rf"#define\s+{name}\s+(0x[0-9A-Fa-f]+)U?",
                source,
            )
            actual = re.search(
                rf"#define\s+{name}\s+(0x[0-9A-Fa-f]+)U?",
                observer,
            )
            self.assertIsNotNone(expected, name)
            self.assertIsNotNone(actual, name)
            self.assertEqual(expected.group(1), actual.group(1), name)

    def test_existing_node_count_is_unchanged(self) -> None:
        app_config = (
            ROOT / "firmware_ch32v307" / "App" / "app_config.h"
        ).read_text(encoding="utf-8")
        self.assertIn("#define NODE_COUNT              3U", app_config)
        self.assertNotIn("DEVICE_ID_OBSERVER", app_config)

    def test_observer_has_no_application_can_transmit_api(self) -> None:
        text = "\n".join(
            path.read_text(encoding="utf-8", errors="ignore")
            for path in (OBSERVER / "Core").rglob("*")
            if path.is_file()
        )
        for token in (
            "HAL_CAN_AddTxMessage",
            "Protocol_SendMasterClaim",
            "Protocol_SendHeartbeat",
            "Protocol_SendRoleSwitch",
            "Protocol_SendGlobalState",
            "CAN_TxHeaderTypeDef",
        ):
            self.assertNotIn(token, text)

    def test_observer_does_not_contain_role_or_game_engines(self) -> None:
        text = "\n".join(
            path.read_text(encoding="utf-8", errors="ignore")
            for path in OBSERVER.rglob("*")
            if path.is_file()
            and "build" not in path.parts
            and "Tests" not in path.parts
        )
        for token in (
            "NodeRole_",
            "GameMaster_",
            "GamePlayer_",
            "Referee" + "Controller",
            "Game" + " Referee",
            "Judge" + " Node",
        ):
            self.assertNotIn(token, text)


if __name__ == "__main__":
    unittest.main()
