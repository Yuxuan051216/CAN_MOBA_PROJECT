TARGET := observer_stm32f103
BUILD_DIR := build

TOOLCHAIN ?= E:/MounRiver_Studio/toolchain/arm-none-eabi-gcc/bin
CC := $(TOOLCHAIN)/arm-none-eabi-gcc.exe
AS := $(CC)
OBJCOPY := $(TOOLCHAIN)/arm-none-eabi-objcopy.exe
SIZE := $(TOOLCHAIN)/arm-none-eabi-size.exe

C_SOURCES := \
Core/Src/main.c \
Core/Src/stm32f1xx_it.c \
Core/Src/can_moba_protocol.c \
Core/Src/can_receiver.c \
Core/Src/observer_controller.c \
Core/Src/event_queue.c \
Core/Src/dfplayer.c \
Core/Src/voice_queue.c \
Core/Src/rgb_effect.c \
Core/Src/buzzer.c \
Core/Src/status_display.c \
Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c

ASM_SOURCES := \
Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s

INCLUDES := \
-ICore/Inc \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/STM32F1xx_HAL_Driver/Inc

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
COMMON_FLAGS := $(CPU_FLAGS) -Os -g3 -Wall -Wextra \
-ffunction-sections -fdata-sections -fno-common \
-DSTM32F103xB $(INCLUDES)
CFLAGS := $(COMMON_FLAGS) -std=gnu11 -MMD -MP
ASFLAGS := $(COMMON_FLAGS) -x assembler-with-cpp
LDFLAGS := $(CPU_FLAGS) -TSTM32F103C8TX_FLASH.ld \
-nostartfiles -Wl,--gc-sections \
-Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
-Wl,--print-memory-usage

VPATH := $(sort $(dir $(C_SOURCES) $(ASM_SOURCES)))
C_OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
ASM_OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)
DEPS := $(C_OBJECTS:.o=.d)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).hex \
     $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) \
	-Wl,--start-group -lc -lm -Wl,--end-group -lgcc -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

clean:
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"

-include $(DEPS)
