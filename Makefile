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

LDSCRIPT = ld/stm32_flash.ld
LDFLAGS	 = --gc-sections,-T$(LDSCRIPT),-no-startup,-nostdlib,-lnosys
OCFLAGS  = -Obinary
ODFLAGS  = -S
OUTPUT_DIR = bin
TARGET = $(OUTPUT_DIR)/main

INCLUDE = -I./core/include \
		-I./STM32F0xx_StdPeriph_Driver/inc \
		-I./FreeRTOS/include \
		-I./FreeRTOS/portable/GCC/ARM_CM0

SRCS    = ./core/stm32f0xx_it.c \
		./core/system_stm32f0xx.c \
		./src/main.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_adc.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_exti.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_flash.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_gpio.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_i2c.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_misc.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_rcc.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_spi.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_syscfg.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_tim.c \
		./STM32F0xx_StdPeriph_Driver/src/stm32f0xx_usart.c \
		./FreeRTOS/portable/MemMang/heap_4.c \
		./FreeRTOS/tasks.c \
		./FreeRTOS/list.c \
		./FreeRTOS/portable/GCC/ARM_CM0/port.c \
		./FreeRTOS/queue.c \
		./FreeRTOS/timers.c

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

$(TARGET).elf: $(OBJS) ./core/startup_stm32f030.o
	@$(CC) -mcpu=cortex-m0 -mthumb -Wl,$(LDFLAGS),-o$(TARGET).elf,-Map,$(TARGET).map ./core/startup_stm32f030.o $(OBJS)

%.o: %.c
	@echo "  CC $<"
	@$(CC) $(INCLUDE) $(CFLAGS)  $< -o $*.o

%.o: %.S
	@echo "  CC $<"
	@$(CC) $(INCLUDE) $(CFLAGS)  $< -o $*.o
