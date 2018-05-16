/*
 * @filename: events.c
 * @date: 16/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "stm32f0xx.h"
#include "queue.h"
#include "task.h"
#include "button.h"
#include "serial.h"
#include "events.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define SERIAL_QUEUE_SIZE      16
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
button_event_status_t button_t =
{
		.status = false
};

static xQueueHandle xSysEventQueue = NULL;
static xQueueHandle xSerialQueue   = NULL;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
static void vSERIAL_CmdService(void *pvParam);
/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
void vBUTTON_EventHandler(button_event_t event)
{
	/* do nothing, for debouncing */
	if(button_t.status == true)
		return;
	/* Set blocking flag */
	button_t.status = true;
	sys_events_t xSysEvent;
	xSysEvent.event = SYS_EVENT_BUTTON;
	/* Send to system even queue */
	xQueueSend(xSysEventQueue,(void*)&xSysEvent, 0);
	taskYIELD();
}

void vSERIAL_EventHandler(uint8_t error, uint8_t byte)
{
    if (error == SERIAL_ERR_NONE)
    {
        xQueueSend(xSerialQueue,(void*)&byte,0);
        /* Yield for higher priority task */
        taskYIELD();
    }
}
/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
static void vSERIAL_CmdService(void *pvParam)
{
    /* Create serial Queue */
    xSerialQueue = xQueueCreate(SERIAL_QUEUE_SIZE, sizeof(uint8_t));
    if (xSerialQueue == NULL) while (1);

}
