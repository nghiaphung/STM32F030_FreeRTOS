/*
 * @filename: meter.c
 * @date: 7/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_rcc.h"
#include "timer.h"
#include "meter.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define METER_SPI_CS_PORT      GPIOA
#define METER_SPI_SCK_PORT     GPIOA
#define METER_SPI_MISO_PORT    GPIOA
#define METER_SPI_MOSI_PORT    GPIOA
#define METER_SYN_PORT         GPIOB
#define METER_EN_PORT          GPIOA
#define METER_SPI_CS_PIN       GPIO_Pin_4
#define METER_SPI_SCK_PIN      GPIO_Pin_5
#define METER_SPI_MISO_PIN     GPIO_Pin_6
#define METER_SPI_MOSI_PIN     GPIO_Pin_7
#define METER_SYN_PIN          GPIO_Pin_1
#define METER_EN_PIN           GPIO_Pin_11

#define METER_GPIO_AF_FUNC     GPIO_AF_0
#define METER_SCK_PIN_SRC      GPIO_PinSource5
#define METER_CS_PIN_SRC       GPIO_PinSource4
#define METER_MISO_PIN_SRC     GPIO_PinSource6
#define METER_MOSI_PIN_SRC     GPIO_PinSource7
#define METER_SYN_PIN_SRC      GPIO_PinSource1

#define METER_HW               SPI1

#define CRC_8                  0x07
#define METER_FRAME_LEN        5
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
uint8_t CRC_u8Checksum;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
void _crc8Calc(uint8_t Data);
uint8_t _calcCRC8(uint8_t *pBuf);
uint8_t _spi_Transfer(uint8_t pData);
void _meter_Enable (void);
void _meter_Disable (void);
void delay(void);
/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
/*
 * @brief Initialize Meter driver
 */
void Meter_Init (void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    SPI_InitTypeDef  SPI_InitStruct;

    /* Initialize PA4  CS */
    GPIO_InitStruct.GPIO_Pin   = METER_SPI_CS_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_Init(METER_SPI_CS_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(METER_SPI_CS_PORT, METER_CS_PIN_SRC, METER_GPIO_AF_FUNC);
    /* Initialize PA5  SCK */
    GPIO_InitStruct.GPIO_Pin   = METER_SPI_SCK_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(METER_SPI_SCK_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(METER_SPI_SCK_PORT, METER_SCK_PIN_SRC, METER_GPIO_AF_FUNC);
    /* Initialize PA6  MISO */
    GPIO_InitStruct.GPIO_Pin   = METER_SPI_MISO_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(METER_SPI_MISO_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(METER_SPI_MISO_PORT, METER_MISO_PIN_SRC, METER_GPIO_AF_FUNC);
    /* Initialize PA7  MOSI */
    GPIO_InitStruct.GPIO_Pin   = METER_SPI_MOSI_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(METER_SPI_MOSI_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(METER_SPI_MOSI_PORT, METER_MOSI_PIN_SRC, METER_GPIO_AF_FUNC);
    /* Initialize PB1 as SYN */
    GPIO_InitStruct.GPIO_Pin   = METER_SYN_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(METER_SYN_PORT, &GPIO_InitStruct);
    /* Initialize PA11 as EN */
    GPIO_InitStruct.GPIO_Pin   = METER_EN_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(METER_EN_PORT, &GPIO_InitStruct);

    /* Initialize SPI1 */
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(METER_HW, &SPI_InitStruct);
	SPI_Cmd(METER_HW, ENABLE);

	GPIO_SetBits(METER_SYN_PORT, METER_SYN_PIN);

}

uint32_t Meter_ReadRegister(uint8_t addr)
{
	uint8_t send_buff[5];
	uint8_t recv_buff[5];
	int i = 0;
	send_buff[0] = addr;
	send_buff[1] = 0x05;
	send_buff[2] = 0x80;
	send_buff[3] = 0x00;
	send_buff[4] = _calcCRC8(send_buff);
	_meter_Enable();
	delay();
	for (i = 0; i < METER_FRAME_LEN; i++)
	{
		recv_buff[i] = _spi_Transfer(send_buff[i]);
	}
	_meter_Disable();
	delay();
	delay();
	send_buff[0] = 0xFF;
	send_buff[1] = 0xFF;
	send_buff[2] = 0xAA;
	send_buff[3] = 0x55;
	send_buff[4] = _calcCRC8(send_buff);
	_meter_Enable();
	delay();
	for (i = 0; i < METER_FRAME_LEN; i++)
	{
		recv_buff[i] = _spi_Transfer(send_buff[i]);
	}
	_meter_Disable();
	return (recv_buff[3] << 24) + (recv_buff[2] << 16) + (recv_buff[1] << 8) \
			+ recv_buff[0];

}

/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/

void _meter_Enable(void)
{
	GPIO_ResetBits(METER_SPI_CS_PORT, METER_SPI_CS_PIN);
}

void _meter_Disable(void)
{
	GPIO_SetBits(METER_SPI_CS_PORT, METER_SPI_CS_PIN);
}


void _crc8Calc(uint8_t Data) {
	uint8_t loc_u8Idx;
	uint8_t loc_u8Temp;
	loc_u8Idx = 0;
	while (loc_u8Idx < 8) {
		loc_u8Temp = Data ^ CRC_u8Checksum;
		CRC_u8Checksum <<= 1;
		if (loc_u8Temp & 0x80) {
			CRC_u8Checksum ^= CRC_8;
		}
		Data <<= 1;
		loc_u8Idx++;
	}
}

uint8_t _calcCRC8(uint8_t *pBuf) {
	uint8_t i;
	CRC_u8Checksum = 0x00;
	for (i = 0; i < METER_FRAME_LEN - 1; i++) {
		_crc8Calc(pBuf[i]);
	}
	return CRC_u8Checksum;
}

uint8_t _spi_Transfer(uint8_t pData)
{
    // Write data to be transmitted to the SPI data register
	METER_HW->DR = pData;
	// Wait until transmit complete
	while (!(METER_HW->SR & (SPI_I2S_FLAG_TXE)));
	// Wait until receive complete
	//while (!(SPI1->SR & (SPI_I2S_FLAG_RXNE)));
	// Wait until SPI is not busy anymore
	while (METER_HW->SR & (SPI_I2S_FLAG_BSY));
	// Return received data from SPI data register
	return METER_HW->DR;

}

void delay(void)
{
	uint16_t count = 500;
	int i = 0;
	for (i = 0; i < count; i++);
}
/******************************************************************************/
/**!                                 END                                      */
/******************************************************************************/
