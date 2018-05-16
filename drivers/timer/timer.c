/*
 * @filename: timer.c
 * @date: 14/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include "stm32f0xx.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_misc.h"
#include "timer.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/
typedef struct {
	uint32_t Interval;
	uint32_t Timeout;
	uint8_t Started;
	void (*callback)(void*);
	void* Param;
} SWTimer_t;
/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define SYS_TIMER_HW_UNIT           ( TIM16 )
#define SYS_TIMER_CLK               ( 1000000UL )
#define SYS_TIMER_IRQ_VECTOR        ( TIM16_IRQn )
#define SW_TIMER_MAX_NUM            ( 16 )
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
static volatile uint32_t _SystemTimerMsCounter = 0;
static volatile uint8_t _SystemSWTimersNum = 0;
static SWTimer_t _SWTimers[SW_TIMER_MAX_NUM] = { { 0 } };
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
static uint32_t _getSystemMsTimer(void);
/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
void SysTimer_Init(void)
{
	uint16_t TIM_Prescaler = 0;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct = { 0 };
	NVIC_InitTypeDef NVIC_InitStruct = { 0 };
	RCC_ClocksTypeDef RCC_ClocksStatus = { 0 };

	/* Get clock status */
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	/* Calculate pre-scaler */
	TIM_Prescaler = (uint16_t)(RCC_ClocksStatus.PCLK_Frequency / SYS_TIMER_CLK);
	/* Set timer's parameters */
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Prescaler = TIM_Prescaler;
	TIM_TimeBaseInitStruct.TIM_Period = 1000;  // 1000us = 1ms
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0; // There is no meaningful
	TIM_TimeBaseInit(SYS_TIMER_HW_UNIT, &TIM_TimeBaseInitStruct);
	TIM_ITConfig(SYS_TIMER_HW_UNIT, TIM_IT_Update, ENABLE);
	TIM_Cmd(SYS_TIMER_HW_UNIT, ENABLE);
	/* Initialize NVIC's IRQ of used HW timer unit */
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPriority = 5;
	NVIC_InitStruct.NVIC_IRQChannel = SYS_TIMER_IRQ_VECTOR;
	NVIC_Init(&NVIC_InitStruct);
	/* Reset all variable to default */
	_SystemTimerMsCounter = 0;
	_SystemSWTimersNum = 0;
}

int CreateSWTimer(uint32_t interval, void (*Callback)(void*), void* param)
{
	int i;
	if ((interval == 0) || (NULL == Callback))
		return -1;
	for (i = 0; i < SW_TIMER_MAX_NUM; i++)
	{
		if (_SWTimers[i].callback == NULL)  // check for available SWTimer
		{
			/* setting param of sw timer */
			_SWTimers[i].Interval = interval;
			_SWTimers[i].Timeout = 0;
			_SWTimers[i].Started = 0;
			_SWTimers[i].callback = Callback;
			_SWTimers[i].Param = param;
			return i;
		}
	}
	return -1;
}

int RunSWTimer(int id)
{
	if ((id > SW_TIMER_MAX_NUM) || (id < 0))
		return -1;
	if (_SWTimers[id].callback != NULL) {
		_SWTimers[id].Timeout = _getSystemMsTimer();
		_SWTimers[id].Timeout += _SWTimers[id].Interval; // Set timeout for timer
		_SWTimers[id].Started = 1;
	}
	return 0;
}

void HaltSWTimer(int id)
{
	_SWTimers[id].Started = 0;
}

int DeleteSWTimer(int id)
{
	if ((id > SW_TIMER_MAX_NUM) || (id < 0))
		return -1;
	if (_SWTimers[id].callback != NULL)
	{
		_SWTimers[id].callback = NULL;
		_SWTimers[id].Param = NULL;
		_SWTimers[id].Started = 0;
	}
	return 0;
}

/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
uint32_t _getSystemMsTimer(void)
{
	return _SystemTimerMsCounter;
}
