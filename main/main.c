/*
 * @filename: main.c
 * @date: 9/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stm32f0xx.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "led.h"
#include "serial.h"
#include "meter.h"
#include "button.h"
#include "eeprom.h"
#include "adc.h"
#include "lora.h"
#include "rtc.h"
#include "timer.h"
#include "events.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/

/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
xTaskHandle xTask2Handle;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
void vTask1(void* pvParam);
void vTask2(void* pvParam);
void prvHardwareSetup(void);
/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/

int main(int argc, char* argv[])
{
    /* To avoid compiler error/warning */
    int temp = argc + (uint32_t)argv; temp++;

    prvHardwareSetup();
	portBASE_TYPE task_result;
	task_result = xTaskCreate(vTask2, "Task 2", 240, NULL, 2, &xTask2Handle);
	if (task_result != pdPASS)
	{
		Serial_Send("Failed 2", 8);
	}
	task_result = xTaskCreate(vTask1, "Task 1", 240, NULL, 1, NULL);
	if (task_result != pdPASS)
	{
		Serial_Send("Failed 1", 8);
	}
	vTaskStartScheduler();

	while(1);
	return 1;
}

void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	assert_param(1);
}

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	assert_param(1);
}
/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
void prvHardwareSetup (void)
{
	serial_t serial =
	{
			.baudrate = 115200,
			.callback = vSERIAL_EventHandler
	};
	button_t button =
	{
			.callback = vBUTTON_EventHandler
	};
	SystemInit();
	Led_Init();
	Serial_Init(&serial);
	Button_Init(&button);
	SysTimer_Init();
}

void vTask1(void* pvParam)
{
	int uxPriority;
	uxPriority = uxTaskPriorityGet(NULL);
	for(;;)
	{
		printf("%s : Free heap size %d bytes\r\n", __func__, xPortGetFreeHeapSize());
		printf("raise task 2 priority");
		vTaskPrioritySet(xTask2Handle, (uxPriority + 1));
	}
}

void vTask2(void* pvParam)
{
	int uxPriority;
	uxPriority = uxTaskPriorityGet(NULL);
	for(;;)
	{
		printf("Task 2 is running\n");
		printf("lower task 2 priority");
		vTaskPrioritySet(NULL, (uxPriority - 2));
	}
}

