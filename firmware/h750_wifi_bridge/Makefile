TARGET := h750_wifi_bridge
BUILD_DIR := build

PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPU := -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
DEFS := -DUSE_HAL_DRIVER -DSTM32H743xx -DAPPLICATION_ADDRESS=0x08000000

INCLUDES := \
  -IUser \
  -ILibraries/CMSIS/Include \
  -ILibraries/CMSIS/Device/ST/STM32H7xx/Include \
  -ILibraries/STM32H7xx_HAL_Driver/Inc \
  -ILibraries/STM32H7xx_HAL_Driver/Inc/Legacy

CFLAGS := $(CPU) $(DEFS) $(INCLUDES) -Og -g3 -Wall -ffunction-sections -fdata-sections
ASFLAGS := $(CPU) $(DEFS) $(INCLUDES) -x assembler-with-cpp
LDFLAGS := $(CPU) -TSTM32H750_FLASH.ld -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map --specs=nosys.specs --specs=nano.specs

SOURCES_C := \
  Libraries/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
  Libraries/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c \
  User/Common/Common.c \
  User/ESP8266/bsp_esp8266.c \
  User/Test/test.c \
  User/delay/core_delay.c \
  User/led/bsp_led.c \
  User/main.c \
  User/mpu/bsp_mpu.c \
  User/stm32h7xx_it.c \
  User/syscalls.c \
  User/systick/bsp_SysTick.c \
  User/usart/bsp_usart.c

SOURCES_S := Libraries/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s

OBJECTS := \
  $(addprefix $(BUILD_DIR)/,$(SOURCES_C:.c=.o)) \
  $(addprefix $(BUILD_DIR)/,$(SOURCES_S:.s=.o))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	$(SIZE) $(BUILD_DIR)/$(TARGET).elf

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) STM32H750_FLASH.ld
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
