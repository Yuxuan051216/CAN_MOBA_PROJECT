from __future__ import annotations

import unittest

from protocol import (
    CAN_ID_CRYSTAL_ATTACK,
    CAN_ID_HEARTBEAT_BASE,
    CAN_ID_MASTER_CLAIM_BASE,
    CAN_ID_MOVE_INPUT_BASE,
    CAN_ID_POSITION_STATE,
    CAN_ID_READY_BASE,
    CAN_ID_SKILL_INPUT_BASE,
    CAN_ID_SKILL_RESULT_BASE,
    GameControl,
    SkillResult,
    pack_crystal_attack,
    pack_game_ctrl,
    pack_global_state,
    pack_heartbeat,
    pack_master_claim,
    pack_move_input,
    pack_position_state,
    pack_death_event,
    pack_ready,
    pack_role_switch,
    pack_skill_input,
    pack_skill_result,
    unpack_frame,
)


class ProtocolTests(unittest.TestCase):
    def test_heartbeat_layout(self) -> None:
        frame = pack_heartbeat(1, 1, 1, 100, 1, 0, heartbeat_seq=7)
        self.assertEqual(frame.can_id, CAN_ID_HEARTBEAT_BASE + 1)
        self.assertEqual(frame.data, bytes([1, 1, 1, 100, 1, 0, 0, 7]))
        parsed = unpack_frame(frame)
        self.assertEqual(parsed["type"], "heartbeat")
        self.assertEqual(parsed["node_id"], 1)
        self.assertEqual(parsed["heartbeat_seq"], 7)

    def test_skill_layout(self) -> None:
        frame = pack_skill_input(0, 3, 1, 9)
        self.assertEqual(frame.can_id, CAN_ID_SKILL_INPUT_BASE)
        self.assertEqual(frame.data, bytes([0, 3, 1, 0, 0, 9, 0, 0]))
        self.assertEqual(unpack_frame(frame)["skill_id"], 3)

    def test_game_control_layout(self) -> None:
        frame = pack_game_ctrl(int(GameControl.START))
        self.assertEqual(frame.data, bytes([1, 0, 0, 0, 0, 0, 0, 0]))

    def test_skill_result_layout(self) -> None:
        frame = pack_skill_result(
            master_node=1,
            source_node=0,
            skill_id=3,
            result=int(SkillResult.ACCEPTED),
            input_seq=9,
            pc_hp=100,
            embedded_hp=70,
            term=2,
        )
        self.assertEqual(frame.can_id, CAN_ID_SKILL_RESULT_BASE + 1)
        parsed = unpack_frame(frame)
        self.assertEqual(parsed["type"], "skill_result")
        self.assertEqual(parsed["source_node"], 0)
        self.assertEqual(parsed["input_seq"], 9)
        self.assertEqual(parsed["embedded_hp"], 70)

    def test_crystal_attack_layout(self) -> None:
        frame = pack_crystal_attack(
            crystal_id=1,
            target_node=2,
            damage=10,
            hit_seq=17,
            pc_hp=100,
            embedded_hp=90,
            term=4,
        )
        self.assertEqual(frame.can_id, CAN_ID_CRYSTAL_ATTACK)
        self.assertEqual(
            frame.data,
            bytes([1, 2, 10, 17, 100, 90, 4, 0]),
        )
        parsed = unpack_frame(frame)
        self.assertEqual(parsed["type"], "crystal_attack")
        self.assertEqual(parsed["damage"], 10)
        self.assertEqual(parsed["hit_seq"], 17)

    def test_move_input_layout(self) -> None:
        frame = pack_move_input(2, -1, 1, 55)
        self.assertEqual(frame.can_id, CAN_ID_MOVE_INPUT_BASE + 2)
        self.assertEqual(frame.data, bytes([2, 0, 2, 55, 0, 0, 0, 0]))

    def test_global_state_layout(self) -> None:
        frame = pack_global_state(3, 2, 1, 90, 80, 4, 5, 1)
        self.assertEqual(
            frame.data,
            bytes([3, 2, 1, 90, 80, 4, 5, 1]),
        )

    def test_position_state_signed_little_endian_layout(self) -> None:
        frame = pack_position_state(290, -160, -32768, 32767)
        self.assertEqual(frame.can_id, CAN_ID_POSITION_STATE)
        self.assertEqual(
            frame.data,
            bytes([0x22, 0x01, 0x60, 0xFF, 0x00, 0x80, 0xFF, 0x7F]),
        )
        parsed = unpack_frame(frame)
        self.assertEqual(parsed["pc_x"], 290)
        self.assertEqual(parsed["pc_y"], -160)
        self.assertEqual(parsed["embedded_x"], -32768)
        self.assertEqual(parsed["embedded_y"], 32767)

    def test_death_role_ready_and_claim_layouts(self) -> None:
        self.assertEqual(
            pack_death_event(2, 0, 10, 4).data,
            bytes([2, 0, 10, 4, 0, 0, 0, 0]),
        )
        self.assertEqual(
            pack_role_switch(5, 2, 1, 1, 2, 10).data,
            bytes([5, 2, 1, 1, 2, 10, 0, 0]),
        )
        ready = pack_ready(1, 5)
        self.assertEqual(ready.can_id, CAN_ID_READY_BASE + 1)
        self.assertEqual(
            ready.data,
            bytes([1, 1, 5, 0, 0, 0, 0, 0]),
        )
        claim = pack_master_claim(2, 6, 2, 3)
        self.assertEqual(
            claim.can_id,
            CAN_ID_MASTER_CLAIM_BASE + 2,
        )
        self.assertEqual(
            claim.data,
            bytes([2, 6, 2, 3, 0, 0, 0, 0]),
        )


if __name__ == "__main__":
    unittest.main()
