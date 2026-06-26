PROJECT := ros_robot_controller
BUILD_DIR := build

CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size
GDB := arm-none-eabi-gdb
STM32FLASH := stm32flash
UART_PORT ?= $(firstword $(wildcard /dev/cu.usbserial-* /dev/cu.wchusbserial-*))
UART_BOOT_SEQ ?= -i '-rts,dtr,-dtr:rts,dtr,-dtr'

CPU_FLAGS := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS := $(CPU_FLAGS) -std=c11 -Og -g3 -Wall -Wextra -Werror \
	-Iinclude \
	-Ithird_party/FreeRTOS-Kernel/include \
	-Ithird_party/FreeRTOS-Kernel/portable/GCC/ARM_CM4F \
	-ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(CPU_FLAGS) -x assembler-with-cpp -g3
LDFLAGS := $(CPU_FLAGS) -TSTM32F407VETx_FLASH.ld \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map \
	-nostdlib -nostartfiles

C_SOURCES := \
	src/main.c \
	src/app/motor_demo_app.c \
	src/bsp/board.c \
	src/drivers/chassis/mecanum.c \
	src/drivers/encoder/encoder.c \
	src/drivers/led/led.c \
	src/drivers/motor/motor.c \
	src/drivers/uart/uart.c
ASM_SOURCES := startup_stm32f407xx.s
OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SOURCES)) \
	$(patsubst %.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean flash flash-uart erase openocd debug

all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).bin

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) STM32F407VETx_FLASH.ld
	@mkdir -p $(@D)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	$(CC) $(ASFLAGS) -c $< -o $@

flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "program $(BUILD_DIR)/$(PROJECT).elf verify reset exit"

flash-uart: all
	@test -n "$(UART_PORT)" || (echo "No USB serial port found" && exit 1)
	$(STM32FLASH) $(UART_BOOT_SEQ) -b 115200 -m 8e1 -w $(BUILD_DIR)/$(PROJECT).bin \
		-v -g 0x08000000 $(UART_PORT)

erase:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
		-c "init; reset halt; stm32f4x mass_erase 0; reset; exit"

openocd:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

debug: all
	$(GDB) $(BUILD_DIR)/$(PROJECT).elf \
		-ex "target extended-remote localhost:3333" \
		-ex "monitor reset halt"

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
