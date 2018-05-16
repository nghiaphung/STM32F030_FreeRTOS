/*
 * @filename serial.c
 * @date 7/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_misc.h"
#include "serial.h"
#include "events.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define SERIAL_HW                 USART1
#define SERIAL_TX_PORT            GPIOA
#define SERIAL_RX_PORT            GPIOA
#define SERIAL_TX_PIN             GPIO_Pin_9
#define SERIAL_RX_PIN             GPIO_Pin_10
#define SERIAL_GPIO_AF_FUNC       GPIO_AF_1
#define SERIAL_TX_PIN_SRC         GPIO_PinSource9
#define SERIAL_RX_PIN_SRC         GPIO_PinSource10

#define SERIAL_BAUDRATE           115200

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
serial_callback_t serial_callback = vSERIAL_EventHandler;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
int _write(int file, char* ptr, int len);
static void _serial_SendByte(uint8_t byte);
/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
/*
 * @brief Initialize Serial driver
 * @param none
 * @return none
 */
void Serial_Init (void)
{
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef  GPIO_InitStruct;
	NVIC_InitTypeDef  NVIC_InitStruct;

	/* Enable GPIOA, USART1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
	/* PA9 Tx pin */
	GPIO_InitStruct.GPIO_Pin = SERIAL_TX_PIN;
	GPIO_PinAFConfig(SERIAL_TX_PORT, SERIAL_TX_PIN_SRC, SERIAL_GPIO_AF_FUNC);
	GPIO_Init(SERIAL_TX_PORT, &GPIO_InitStruct);
	/* PA10 Rx pin */
	GPIO_InitStruct.GPIO_Pin = SERIAL_RX_PIN;
	GPIO_PinAFConfig(SERIAL_RX_PORT, SERIAL_RX_PIN_SRC, SERIAL_GPIO_AF_FUNC);
	GPIO_Init(SERIAL_RX_PORT, &GPIO_InitStruct);

	/* Initialize USART1*/
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_BaudRate = SERIAL_BAUDRATE;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(SERIAL_HW, &USART_InitStruct);

	USART_ITConfig(SERIAL_HW, USART_IT_RXNE, ENABLE);
	USART_ITConfig(SERIAL_HW, USART_IT_ERR, ENABLE);
	USART_ITConfig(SERIAL_HW, USART_IT_PE, ENABLE);

	USART_Cmd(SERIAL_HW, ENABLE);

	/* Configure interrupt */
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
}



/**
 * @brief Send number of data bytes via serial port
 * @param[in]   data	: data package
 * @param[in]   length	: number of data bytes
 * @return      none
 */
void Serial_Send(char* data, uint16_t length)
{
	//<! Sending data
	_write(0, (char*)data, length);
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
    Serial_SendByte(ch);
    return ch;
}
/******************************************************************************/
/**!                          LOCAL FUNCTIONS                                 */
/******************************************************************************/
/*
 * @brief Send 1 byte via Serial
 * @param[in] byte want to be sent
 */
static void _serial_SendByte(uint8_t byte)
{
    while (SET != USART_GetFlagStatus(SERIAL_HW, USART_FLAG_TXE));
    USART_SendData(SERIAL_HW, byte);
}

/**
 * @brief To re-direct printf() function
 * @param[in]   file: one byte data
 * @param[in]   ptr : data address
 * @param[out]  len : data's length
 */
int _write(int file, char* ptr, int len)
{
    file++;
    int i = 0;
    for (i = 0; i < len; i++)
    	_serial_SendByte(ptr[i]);
    return 0;
}

/**
 * @brief UART1 Interrupt Service Routine
 * @param   none
 * @return      none
 */
void USART1_IRQHandler(void)
{
	//<! Get interrupt flag
	if (SET == USART_GetITStatus(SERIAL_HW, USART_IT_RXNE))
	{
		//<! Get data
		uint8_t byte = (uint8_t)USART_ReceiveData(SERIAL_HW) & 0xFF;
		serial_callback(SERIAL_ERR_NONE, byte);
	}
	//<! Get overrun interrupt flag
	else if (SET == USART_GetITStatus(SERIAL_HW, USART_IT_ORE))
	{
		serial_callback(SERIAL_ERR_OVERRUN, 0);
		USART_ClearITPendingBit(SERIAL_HW, USART_IT_ORE);
	}
	//<! Get framing error interrupt flag
	else if (SET == USART_GetITStatus(SERIAL_HW, USART_IT_FE))
	{
		serial_callback(SERIAL_ERR_FRAME, 0);
        USART_ClearITPendingBit(SERIAL_HW, USART_IT_FE);
	}
	//<! Get Idle line error interrupt flag
	else if (SET == USART_GetITStatus(SERIAL_HW, USART_IT_IDLE))
	{
		serial_callback(SERIAL_ERR_IDLE_LINE, 0);
        USART_ClearITPendingBit(SERIAL_HW, USART_IT_IDLE);
	}
    //<! Get parity error interrupt flag
    else if (SET == USART_GetITStatus(SERIAL_HW, USART_IT_PE))
    {
    	serial_callback(SERIAL_ERR_PARITY, 0);
        USART_ClearITPendingBit(SERIAL_HW, USART_IT_PE);
    }
}
