/*
 * @filename: eeprom.c
 * @date: 13/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
#include <stdio.h>
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_rcc.h"
#include "eeprom.h"
/******************************************************************************/
/**!                            LOCAL TYPEDEF                                 */
/******************************************************************************/

/******************************************************************************/
/**!                            LOCAL SYMBOLS                                 */
/******************************************************************************/
#define EEPROM_I2C_SCL_PORT      GPIOB
#define EEPROM_I2C_SDA_PORT      GPIOB
#define EEPROM_I2C_SCL_PIN       GPIO_Pin_13
#define EEPROM_I2C_SDA_PIN       GPIO_Pin_14
#define EEPROM_GPIO_AF_FUNC      GPIO_AF_5
#define EEPROM_SCL_PIN_SRC       GPIO_PinSource13
#define EEPROM_SDA_PIN_SRC       GPIO_PinSource14

#define EEPROM_HW                   I2C2
#define EEPROM_SLAVE_ADDR           ((uint8_t)0xA0)
#define EEPROM_CLKC_FREQ_HZ         100000UL
#define EEPROM_WRITABLE_BYTES_LIMIT 32
#define EEPROM_PAGE_SIZE_IN_BYTES   256
#define EEPROM_PAGES_NUM            32
#define EEPRPOM_READ_CMD            0x01
#define EEPRPOM_WRITE_CMD           0x00

#define I2C_TIMING                  0x2000090E //100kHz, 48MHz clock
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/

/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/

/******************************************************************************/
/**!                        EXPORTED FUNCTIONS                                */
/******************************************************************************/
/*
 * @brief Initialize EEPROM driver
 * @param none
 * @return none
 */
void Eeprom_Init (void)
{
    I2C_InitTypeDef  I2C_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_UP;
    /* Initialize PB13 SCL */
    GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_SCL_PIN;
    GPIO_Init(EEPROM_I2C_SCL_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(EEPROM_I2C_SCL_PORT, EEPROM_SCL_PIN_SRC, EEPROM_GPIO_AF_FUNC);
    /* Initialize PB14 SDA */
    GPIO_InitStruct.GPIO_Pin = EEPROM_I2C_SDA_PIN;
    GPIO_Init(EEPROM_I2C_SDA_PORT, &GPIO_InitStruct);
    GPIO_PinAFConfig(EEPROM_I2C_SDA_PORT, EEPROM_SDA_PIN_SRC, EEPROM_GPIO_AF_FUNC);
    /*  Initialize I2C module */
    I2C_InitStruct.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStruct.I2C_DigitalFilter       = 0x00;
    I2C_InitStruct.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStruct.I2C_Timing              = I2C_TIMING;
    I2C_InitStruct.I2C_AnalogFilter        = I2C_AnalogFilter_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(EEPROM_HW, &I2C_InitStruct);
    I2C_Cmd(EEPROM_HW, ENABLE);
	I2C_10BitAddressingModeCmd(EEPROM_HW, DISABLE);

}
/*
 * @brief Write data to EEPROM
 * @param: uint16_t addr: start address to write
 *         uint8_t* data: data to write
 * 		   uint8_t: len: number of data in bytes
 * @return 0
 * TODO: write multi pages, add timeout
 */
int Eeprom_Write(uint16_t addr, uint8_t* data, uint16_t len)
{
	int i;
	/* Disable Auto reload mode */
	I2C_ReloadCmd(EEPROM_HW, DISABLE);
	/* Disable Auto end mode */
	I2C_AutoEndCmd(EEPROM_HW, DISABLE);
	/* Disable 10 bit address mode, use 7 bit mode */
	I2C_10BitAddressingModeCmd(EEPROM_HW, DISABLE);
	uint8_t start_byte = EEPROM_SLAVE_ADDR;
	I2C_SlaveAddressConfig(EEPROM_HW, (uint16_t)start_byte);
	/* Set number of byte to send, 2 byte addr and len-byte data */
	I2C_NumberOfBytesConfig(EEPROM_HW, 2 + len);
	/* Set direction as transmitter (write cmd) */
	I2C_MasterRequestConfig(EEPROM_HW, I2C_Direction_Transmitter);
	/* Generate start condition */
	/* After start cond, I2C module auto send slave addr and
	   direction bit */
	I2C_GenerateSTART(EEPROM_HW, ENABLE);

    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));
	/* Send 2 bytes eeprom address */
    I2C_SendData(EEPROM_HW, (uint8_t)(addr >> 8));
    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));

    I2C_SendData(EEPROM_HW, (uint8_t)(addr & 0xFF));
    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));
    /* Send data byte */
    for (i = 0; i < len; i++)
    {
    	I2C_SendData(EEPROM_HW, data[i]);
    	while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));
    }
    I2C_GenerateSTOP(EEPROM_HW, ENABLE);
    return 0;
}

/*
 * @brief Read data from EEPROM
 * @param: uint16_t addr: start address to read
 *         uint8_t* data: data to read
 * 		   uint8_t: len: number of data in bytes
 * @return 0
 * TODO: read multi pages, add timeout
 */
int Eeprom_Read(uint16_t addr, uint8_t* data, uint16_t len)
{
	int i;
	/* Disable Auto reload mode */
	I2C_ReloadCmd(EEPROM_HW, DISABLE);
	/* Disable Auto end mode */
	I2C_AutoEndCmd(EEPROM_HW, DISABLE);
	/* Disable 10 bit address mode, use 7 bit mode */
	I2C_10BitAddressingModeCmd(EEPROM_HW, DISABLE);
	uint8_t start_byte = EEPROM_SLAVE_ADDR;
	I2C_SlaveAddressConfig(EEPROM_HW, (uint16_t)start_byte);
	/* Set number of byte to send (2 byte address) */
	I2C_NumberOfBytesConfig(EEPROM_HW, 2);
	/* Set direction as transmitter (write cmd) */
	I2C_MasterRequestConfig(EEPROM_HW, I2C_Direction_Transmitter);
	I2C_GenerateSTART(EEPROM_HW, ENABLE);

    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));
    I2C_SendData(EEPROM_HW, (uint8_t)(addr >> 8));
    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));
    I2C_SendData(EEPROM_HW, (uint8_t)(addr & 0xFF));
    while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE));

	start_byte = EEPROM_SLAVE_ADDR;
	I2C_SlaveAddressConfig(EEPROM_HW, (uint16_t)start_byte);
	/* Set as read mode */
	I2C_MasterRequestConfig(EEPROM_HW, I2C_Direction_Receiver);
	I2C_NumberOfBytesConfig(EEPROM_HW, len);
    I2C_GenerateSTART(EEPROM_HW, ENABLE);

    for (i = 0; i< len; i++)
    {
    	while (SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_RXNE));
    	data[i] = I2C_ReceiveData(EEPROM_HW);
    }
    I2C_GenerateSTOP(EEPROM_HW, ENABLE);
    return 0;
}

/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
