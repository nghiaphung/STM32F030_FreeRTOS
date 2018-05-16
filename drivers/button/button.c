/*
 * @filename: button.c
 * @date: 14/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include <stddef.h>
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "button.h"
#include "events.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define BUTTON_GPIO_PORT     GPIOC
#define BUTTON_GPIO_PIN      GPIO_Pin_0
#define BUTTON_EXTI_LINE     EXTI_Line0
#define BUTTON_IRQ           EXTI0_1_IRQn
#define BUTTON_EXTI_PORT_SRC EXTI_PortSourceGPIOC
#define BUTTON_EXTI_PIN_SRC  EXTI_PinSource0
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
button_callback_t button_callback = vBUTTON_EventHandler;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/

/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
/*
 * @brief Initialize Button driver
 * @param none
 * @return none
 */
void Button_Init (void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	/* Enable GPIOC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    /* Data initialization for GPIO */
	GPIO_InitStruct.GPIO_Pin   = BUTTON_GPIO_PIN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);
    /* Configure external interrupt for GPIO */
    EXTI_InitStruct.EXTI_Line    = BUTTON_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    SYSCFG_EXTILineConfig(BUTTON_EXTI_PORT_SRC, BUTTON_EXTI_PIN_SRC);
    /* Configure NVIC for indicated vectors */
    NVIC_InitStruct.NVIC_IRQChannelPriority  = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd  = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannel = BUTTON_IRQ;
    NVIC_Init(&NVIC_InitStruct);
}

/*
 * @brief Get Button status
 * @param none
 * @return 1: button is pressed, 0: button is not pressed
 *
 */
button_status_t Button_Get(void)
{
	return (button_status_t)GPIO_ReadInputDataBit(BUTTON_GPIO_PORT, \
			BUTTON_GPIO_PIN);
}

/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
void EXTI0_1_IRQHandler(void)
{
	if (SET == EXTI_GetITStatus(BUTTON_EXTI_LINE))
	{
		if(SET == GPIO_ReadInputDataBit(BUTTON_GPIO_PORT, BUTTON_GPIO_PIN))
			button_callback(BUTTON_RISING_EDGE);
		else
			button_callback(BUTTON_FALLING_EDGE);
        /* Clear interrupt flag */
        EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);
	}
}
/******************************************************************************/
/**!                             END OF FILE                                  */
/******************************************************************************/
