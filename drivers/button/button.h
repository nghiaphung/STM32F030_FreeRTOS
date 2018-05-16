/*
 * @filename: button.h
 * @date: 14/5/2018
 */
#ifndef BUTTON_H
#define BUTTON_H


typedef enum
{
	BUTTON_INACTIVE = 0,
	BUTTON_ACTIVE =1
}button_status_t;

typedef void (*button_callback_t)(button_event_t event);

void Button_Init (void);
button_status_t Button_Get(void);

#endif
