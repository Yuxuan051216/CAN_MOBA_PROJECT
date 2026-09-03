#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "ch32v30x.h"

#define NODE_ID_PC              0U
#define NODE_ID_BOARD_A         1U
#define NODE_ID_BOARD_B         2U
#define NODE_COUNT              3U

#include "build_variant.h"

/*
 * 普通 Build 使用 build_variant.h 中的默认节点。
 * 仍允许通过编译参数 -DNODE_ID=2 覆盖，批量构建优先使用项目脚本。
 */
#ifndef NODE_ID
#define NODE_ID                 CAN_MOBA_BUILD_NODE_ID
#endif

#if (NODE_ID != NODE_ID_BOARD_A) && (NODE_ID != NODE_ID_BOARD_B)
#error "NODE_ID must be 1 (Board A) or 2 (Board B)"
#endif

#define CAN1_USE_REMAP_PB8_PB9  1
#define CAN_BAUDRATE_500K       1
#define CAN_DEBUG_PRINT         1

#define USE_JOYSTICK            1

#define GAME_INIT_HP            100U
#define EMBEDDED_INIT_HP        100U
#define PC_INIT_HP              100U
#define WIN_SCORE               3U

#define SKILL1_DAMAGE           10U
#define SKILL2_HEAL             15U
#define SKILL3_DAMAGE           30U
#define SKILL1_RANGE_MAP_PIXELS 700
#define SKILL3_RANGE_MAP_PIXELS 1400

#define SKILL1_CD_MS            1000UL
#define SKILL2_CD_MS            5000UL
#define SKILL3_CD_MS            10000UL

#define PC_RESPAWN_SECONDS      3U
#define DEATH_COOLDOWN_MS       10000UL
#define DEATH_LED_ON_MS         5000UL

#define HEARTBEAT_PERIOD_MS     500UL
#define MASTER_TIMEOUT_MS       1500UL
#define NODE_ONLINE_TIMEOUT_MS  1600UL
#define MASTER_CLAIM_WINDOW_MS  120UL
#define MASTER_CLAIM_RETRY_MS   300UL
#define GLOBAL_STATE_PERIOD_MS  100UL
#define POSITION_STATE_PERIOD_MS 100UL
#define PLAYER_INPUT_PERIOD_MS  50UL
#define MOVE_INPUT_REFRESH_MS   250UL
#define MASTER_TICK_PERIOD_MS   50UL
#define KEY_DEBOUNCE_MS         20UL
#define CAN_RX_BUDGET_PER_LOOP  8U

/* Master authoritative world coordinates use normalized units * 1000. */
#define WORLD_COORD_SCALE       1000
#define PC_SPAWN_X              290
#define PC_SPAWN_Y              160
#define EMBEDDED_SPAWN_X        710
#define EMBEDDED_SPAWN_Y        (-160)
#define WORLD_X_MIN             120
#define WORLD_X_MAX             880
#define WORLD_Y_MIN             (-620)
#define WORLD_Y_MAX             620
#define WORLD_MAP_X_PIXELS      3880
#define WORLD_MAP_Y_PIXELS      330
#define MOVE_STEP_X_PER_TICK    5
#define MOVE_STEP_Y_PER_TICK    59

#define CRYSTAL_BLUE            1U
#define CRYSTAL_RED             2U
#define BLUE_CRYSTAL_X          193
#define BLUE_CRYSTAL_Y          0
#define RED_CRYSTAL_X           807
#define RED_CRYSTAL_Y           0
#define CRYSTAL_ATTACK_RANGE_MAP_PIXELS 234
#define CRYSTAL_ATTACK_MS       1000UL
#define CRYSTAL_DAMAGE          10U

#define KEY1_PORT               GPIOB
#define KEY1_PIN                GPIO_Pin_0
#define KEY2_PORT               GPIOB
#define KEY2_PIN                GPIO_Pin_1
#define KEY3_PORT               GPIOB
#define KEY3_PIN                GPIO_Pin_2

#define JOYSTICK_X_PORT         GPIOA
#define JOYSTICK_X_PIN          GPIO_Pin_0
#define JOYSTICK_X_CHANNEL      ADC_Channel_0
#define JOYSTICK_Y_PORT         GPIOA
#define JOYSTICK_Y_PIN          GPIO_Pin_1
#define JOYSTICK_Y_CHANNEL      ADC_Channel_1
#define JOYSTICK_LOW_THRESHOLD  1300U
#define JOYSTICK_HIGH_THRESHOLD 2800U

/* 默认 LED 使用 PC0/PC1，低电平点亮，可按核心板原理图修改。 */
#define LED_RUN_PORT            GPIOC
#define LED_RUN_PIN             GPIO_Pin_0
#define LED_ROLE_PORT           GPIOC
#define LED_ROLE_PIN            GPIO_Pin_1
#define LED_ACTIVE_LOW          1

#define LCD1602_I2C_ADDRESS     0x27U
#define LCD1602_SCL_PORT        GPIOC
#define LCD1602_SCL_PIN         GPIO_Pin_10
#define LCD1602_SDA_PORT        GPIOC
#define LCD1602_SDA_PIN         GPIO_Pin_11

#define PC_DEATH_LED_PORT       GPIOC
#define PC_DEATH_LED_PIN        GPIO_Pin_2
#define BOARD_DEATH_LED_PORT    GPIOC
#define BOARD_DEATH_LED_PIN     GPIO_Pin_3

#if NODE_ID == NODE_ID_BOARD_A
#define FIRMWARE_VARIANT_NAME   "BOARD_A"
#else
#define FIRMWARE_VARIANT_NAME   "BOARD_B"
#endif

#endif
