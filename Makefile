CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
AS = $(CROSS_COMPILE)as
OC = $(CROSS_COMPILE)objcopy
OD = $(CROSS_COMPILE)objdump
SZ = $(CROSS_COMPILE)size

CFLAGS = -c -fno-common \
		-ffunction-sections \
		-fdata-sections \
		-Os\
		-mcpu=cortex-m0 -Wall \
		-mthumb

LDSCRIPT = kernel/ld/stm32_flash.ld
LDFLAGS	 = --gc-sections,-T$(LDSCRIPT),-no-startup,-nostdlib,-lnosys
OCFLAGS  = -Obinary
ODFLAGS  = -S
OUTPUT_DIR = bin
TARGET = $(OUTPUT_DIR)/main

INCLUDE = -I./kernel/hal/CMSIS/Include \
		-I./kernel/hal \
		-I./kernel/hal/config \
		-I./kernel/hal/STM32F0xx_StdPeriph_Driver/inc \
		-I./kernel/hal/vectors \
		-I./kernel/FreeRTOS/include \
		-I./kernel/FreeRTOS/portable/GCC/ARM_CM0 \
		-I./kernel/FreeRTOS/config \
		-I./drivers/button \
		-I./drivers/led \
		-I./drivers/meter \
		-I./drivers/serial \
		-I./drivers/timer \
		-I./drivers/eeprom \
		-I./drivers/adc \
		-I./drivers/rtc \
		-I./drivers/button \
		-I./drivers/lora \
		-I./drivers/clock \
		-I./framework 


SRCS = ./kernel/hal/system_stm32f0xx.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_adc.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_exti.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_flash.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_gpio.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_i2c.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_misc.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_rcc.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_spi.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_syscfg.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_tim.c \
		./kernel/hal/STM32F0xx_StdPeriph_Driver/src/stm32f0xx_usart.c \
		./kernel/FreeRTOS/portable/MemMang/heap_4.c \
		./kernel/FreeRTOS/tasks.c \
		./kernel/FreeRTOS/list.c \
		./kernel/FreeRTOS/queue.c \
		./kernel/FreeRTOS/timers.c \
		./kernel/FreeRTOS/portable/GCC/ARM_CM0/port.c \
		./libs/newlib/_syscalls.c \
		./libs/newlib/assert.c \
		./libs/newlib/_sbrk.c \
		./libs/newlib/_exit.c \
		./drivers/led/led.c \
		./drivers/button/button.c \
		./drivers/meter/meter.c \
		./drivers/serial/serial.c \
		./drivers/eeprom/eeprom.c \
		./drivers/timer/timer.c \
		./drivers/clock/clock.c \
		./framework/events.c \
		./framework/sysctl.c \
		./main/main.c

OBJS = $(SRCS:.c=.o)
.PHONY : clean all

all: $(TARGET).bin  $(TARGET).list
	$(SZ) $(TARGET).elf

clean:
	-find . -name '*.o'   -exec rm {} \;
	-find . -name '*.elf' -exec rm {} \;
	-find . -name '*.lst' -exec rm {} \;
	-find . -name '*.out' -exec rm {} \;
	-find . -name '*.bin' -exec rm {} \;
	-find . -name '*.map' -exec rm {} \;

$(TARGET).list: $(TARGET).elf
	$(OD) $(ODFLAGS) $< > $(TARGET).lst

$(TARGET).bin: $(TARGET).elf
	$(OC) $(OCFLAGS) $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJS) ./kernel/hal/startup_stm32f030.o
	@$(CC) -mcpu=cortex-m0 -mthumb -Wl,$(LDFLAGS),-o$(TARGET).elf,-Map,$(TARGET).map ./kernel/hal/startup_stm32f030.o $(OBJS)

%.o: %.c
	@echo "  CC $<"
	@$(CC) $(INCLUDE) $(CFLAGS)  $< -o $*.o

%.o: %.S
	@echo "  CC $<"
	@$(CC) $(INCLUDE) $(CFLAGS)  $< -o $*.o
