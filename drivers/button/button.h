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

/**
 * Button events
 */
typedef enum
{
    BUTTON_RISING_EDGE  = 0x00,
    BUTTON_FALLING_EDGE = 0x01
}button_event_t;

typedef void (*button_callback_t)(button_event_t event);

/**
 * Button configuration structure
 */
typedef struct
{
    button_event_t    event;
    button_callback_t callback;
}button_t;



void Button_Init (button_t* button);
button_status_t Button_Get(void);

#endif
