/*
 * @filename: events.h
 * @date: 16/5/2018
 */

#ifndef EVENTS_H
#define EVENTS_H

#include "button.h"
#include "serial.h"

/**
 * @brief button handle
 */
typedef struct{
    bool    status;
    uint8_t count;
    uint8_t level;
}button_event_status_t;

/**
 * @brief System event ID
 */
typedef enum{
    SYS_EVENT_BUTTON = 0x00,
    SYS_EVENT_IR_CMD = 0x01
}event_id_t;

void vBUTTON_EventHandler(button_event_t event);
void vSERIAL_EventHandler(uint8_t error, uint8_t byte);

#endif /* EVENTS_H */
