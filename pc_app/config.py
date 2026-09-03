from __future__ import annotations

import os

CAN_BACKEND = os.getenv("CAN_MOBA_BACKEND", "mock")
DEMO_AI = os.getenv("CAN_MOBA_DEMO_AI", "0") == "1"
CONSOLE_CAN_LOG = os.getenv(
    "CAN_MOBA_CONSOLE_LOG",
    "0" if CAN_BACKEND.lower() == "mock" else "1",
) == "1"

# python-can 参数。PEAK PCAN-USB 常用这组默认值，可通过环境变量修改。
PYTHON_CAN_INTERFACE = os.getenv("CAN_MOBA_INTERFACE", "pcan")
PYTHON_CAN_CHANNEL = os.getenv("CAN_MOBA_CHANNEL", "PCAN_USBBUS1")
PYTHON_CAN_BITRATE = int(os.getenv("CAN_MOBA_BITRATE", "500000"))

WINDOW_WIDTH = 1200
WINDOW_HEIGHT = 760
FPS = 60
NODE_TIMEOUT_SECONDS = 1.6
PC_HEARTBEAT_SECONDS = 0.5
MOVE_INPUT_REFRESH_SECONDS = 0.25

GAME_TITLE = "CAN MOBA 单路对战地图"
