from __future__ import annotations

from collections import deque
from datetime import datetime

import config
from protocol import CanFrame


def format_frame(frame: CanFrame, direction: str = "RX") -> str:
    timestamp = datetime.fromtimestamp(frame.timestamp).strftime("%H:%M:%S.%f")[:-3]
    payload = " ".join(f"{byte:02X}" for byte in frame.data)
    return f"{timestamp} {direction} ID=0x{frame.can_id:03X} [{payload}]"


class CanLog:
    def __init__(self, max_entries: int = 200) -> None:
        self._entries: deque[str] = deque(maxlen=max_entries)

    def add_frame(self, frame: CanFrame, direction: str = "RX") -> None:
        line = format_frame(frame, direction)
        self._entries.append(line)
        if config.CONSOLE_CAN_LOG:
            print(line, flush=True)

    def add_text(self, text: str) -> None:
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        line = f"{timestamp} {text}"
        self._entries.append(line)
        if config.CONSOLE_CAN_LOG:
            print(line, flush=True)

    def recent(self, count: int = 20) -> list[str]:
        return list(self._entries)[-count:]
