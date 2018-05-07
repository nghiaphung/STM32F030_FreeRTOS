#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f0xx.h"
#include "led.h"
void vApplicationStackOverflowHook (void)
{

}

int main(void)
{

	uint8_t i;
	SystemInit();

	while(1);
	return 1;
}
