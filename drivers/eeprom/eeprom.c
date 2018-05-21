/*
 * @filename: eeprom.c
 * @date: 13/5/2018
 */
/******************************************************************************/
/**!                               INCLUDE                                    */
/******************************************************************************/
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
#define EEPROM_CLKC_FREQ_HZ         400000UL
#define EEPROM_WRITABLE_BYTES_LIMIT 32
#define EEPROM_PAGE_SIZE_IN_BYTES   256
#define EEPROM_PAGES_NUM            32
#define EEPRPOM_READ_CMD            0x01
#define EEPRPOM_WRITE_CMD           0x00
/******************************************************************************/
/**!                         EXPORTED VARIABLES                               */
/******************************************************************************/

/******************************************************************************/
/**!                          LOCAL VARIABLES                                 */
/******************************************************************************/
static uint32_t I2CFrequency = 0;
/******************************************************************************/
/**!                    LOCAL FUNCTIONS PROTOTYPES                            */
/******************************************************************************/
static uint32_t _i2cTimingCal (uint32_t i2c_clk);
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
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
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
    I2C_InitStruct.I2C_DigitalFilter       = 0x0F;
    I2C_InitStruct.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStruct.I2C_Timing              = _i2cTimingCal(EEPROM_CLKC_FREQ_HZ);
    I2C_InitStruct.I2C_AnalogFilter        = I2C_AnalogFilter_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(EEPROM_HW, &I2C_InitStruct);
    I2C_Cmd(EEPROM_HW, ENABLE);

}

int Eeprom_Write(uint16_t addr, uint8_t* data, uint16_t len)
{
    /* Validate EEPROM memory space */
    if ((addr + len) > (EEPROM_PAGES_NUM * EEPROM_PAGE_SIZE_IN_BYTES))
        return -1;
	/* Timeout 2 cycle of I2C clock * 12 bits */
	uint32_t timeout = 2 * 12 * (I2CFrequency/EEPROM_CLKC_FREQ_HZ);
	uint32_t count   = 0;

    uint16_t start_byte = 0x0000;
    uint16_t i       = 0;
    uint16_t current_addr = addr;
    uint16_t length = 0;
    /* Disable Auto reload mode */
    I2C_ReloadCmd(EEPROM_HW, DISABLE);
    /* Enable Auto end mode */
    I2C_AutoEndCmd(EEPROM_HW, ENABLE);
    /* Set slave address and flow control bit */
    start_byte = EEPROM_SLAVE_ADDR | EEPRPOM_WRITE_CMD;
    /* Start sending */
    while (current_addr < addr + len)
    {
        /* Indicate start byte */
    	start_byte |= (uint8_t)((current_addr >> 8) & 0xFF);
        /* Set slave address */
        I2C_SlaveAddressConfig(EEPROM_HW, start_byte);
        /* Check remaining data bytes */
        if (((len + addr) - current_addr) > EEPROM_WRITABLE_BYTES_LIMIT)
            length = EEPROM_WRITABLE_BYTES_LIMIT;
        else
            length = (len + addr) - current_addr;
        /* Set transaction to send 1 byte memory address + number of bytes data
         * and force clear TC bit */
        I2C_NumberOfBytesConfig(EEPROM_HW, 1 + length);
        /* Set start condition */
        I2C_GenerateSTART(EEPROM_HW, ENABLE);
        /* Check NACK */
        if (SET == I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_NACKF))
        {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Wait TX empty flag */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE)) \
                                              && (count--));
        /* Check timeout condition */
        if (0 == count) {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Send word address */
        I2C_SendData(EEPROM_HW, current_addr & 0xFF);
        /* Check TX empty flag to make sure HW buffer is ready to load data */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE)) \
                                              && (count--));
        if (0 == count)
        {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Send data */
        for (i = 0; i < length; i++)
        {
            I2C_SendData(EEPROM_HW, data[current_addr - addr + i]);
            /* Check TX empty flag */
            count = timeout;
            while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE)) \
                                                  && (count--));
            if (0 == count){
                /* Generate stop condition to terminate transaction on slave */
                I2C_GenerateSTOP(EEPROM_HW, ENABLE);
                return -1;
            }
        }
        /* Wait for TX transfer complete */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TC)) \
                                              && (count--));
        if (0 == count) {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Check NACK to avoid disconnect during transmitting */
        if (SET == I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_NACKF)) {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Update current_addr */
        current_addr += length;
    }
    return 0;

}


int Eeprom_Read(uint16_t addr, uint8_t* data, uint16_t len)
{
	if ((addr + len) > (EEPROM_PAGES_NUM * EEPROM_PAGE_SIZE_IN_BYTES))
		return -1;
	/* Timeout 4 cycle of I2C clock * 12 bits */
	uint32_t timeout = 4 * 12 * (I2CFrequency/EEPROM_CLKC_FREQ_HZ);
	uint32_t count   = 0;

    uint16_t start_byte = 0x0000;
    uint16_t i       = 0;
    uint16_t current_addr   = addr;
    uint16_t length = 0;
    /* Disable Auto reload mode */
    I2C_ReloadCmd(EEPROM_HW, DISABLE);
    /* Enable Auto end mode */
    I2C_AutoEndCmd(EEPROM_HW, ENABLE);
    /* Start sending */
    while (current_addr < addr + len) {
        /* Set slave address and flow control bit as write */
    	start_byte = EEPROM_SLAVE_ADDR | EEPRPOM_WRITE_CMD;
        /* Indicate start byte */
    	start_byte |= (uint8_t)((current_addr >> 8) & 0xFF);
        /* Set slave address */
        I2C_SlaveAddressConfig(EEPROM_HW, start_byte);
        /* Check remaining data bytes */
        if (current_addr < EEPROM_PAGE_SIZE_IN_BYTES)
        	length = EEPROM_PAGE_SIZE_IN_BYTES - current_addr;
        else
        	length = (len + addr) - current_addr;
        /* Set transaction to send 1 byte memory address + number of bytes data
         * to be read and force clear TC bit */
        I2C_NumberOfBytesConfig(EEPROM_HW, 1 + length);
        /* Set start condition */
        I2C_GenerateSTART(EEPROM_HW, ENABLE);
        /* Check NACK */
        if (SET == I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_NACKF)) {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Wait TX empty flag */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE))  && (count--));
        /* Check timeout condition */
        if (0 == count){
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Send word address */
        I2C_SendData(EEPROM_HW, current_addr & 0xFF);
        /* Check TX empty flag to make sure HW buffer is ready to load data */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TXE)) \
                                              && (count--));
        if (0 == count)
        {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Set slave address and flow control bit as read */
        start_byte = EEPROM_SLAVE_ADDR | EEPRPOM_READ_CMD;
        /* Indicate start byte */
        start_byte |= (uint8_t)((current_addr >> 8) & 0xFF);
        /* Set slave address */
        I2C_SlaveAddressConfig(EEPROM_HW, start_byte);
        /* Set start condition */
        I2C_GenerateSTART(EEPROM_HW, ENABLE);
        /* Reading data */
        for (i = 0; i < length; i++)
        {
            /* Check RX not empty flag */
        	count = timeout;
            while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_RXNE)) \
                                                     && (count--));
            if (0 == count) {
                /* Generate stop condition to terminate transaction on slave */
                I2C_GenerateSTOP(EEPROM_HW, ENABLE);
                return -1;
            }
            /* Get data from HW buffer */
            data[current_addr - addr + i] = I2C_ReceiveData(EEPROM_HW);
        }
        /* Wait for transmitting complete */
        count = timeout;
        while ((SET != I2C_GetFlagStatus(EEPROM_HW, I2C_FLAG_TC)) \
                                              && (count--));
        if (0 == count) {
            /* Generate stop condition to terminate transaction on slave */
            I2C_GenerateSTOP(EEPROM_HW, ENABLE);
            return -1;
        }
        /* Update current_addr */
        current_addr += length;
    }
    return 0;
}
/******************************************************************************/
/**!                           LOCAL FUNCTIONS                                */
/******************************************************************************/
static uint32_t _i2cTimingCal (uint32_t i2c_clk)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;

    uint32_t PRESC = 0;
    uint32_t SCLDEL = 0;
    uint32_t SDADEL = 0;
    uint32_t SCLH = 0;
    uint32_t SCLL = 0;

    /* Get data address */
    uint32_t I2C_TIMINGR_RegVal = 0;

    /* Get clock status */
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    /* Save I2C frequency */
    I2CFrequency = RCC_ClocksStatus.PCLK_Frequency;
    /* Check peripheral clock of I2C */
    switch (RCC_ClocksStatus.PCLK_Frequency)
    {
    case 48000000UL: // Full speed
        if (10000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x0B)<<28;
            SCLDEL = ((uint32_t)0x04)<<20;
            SDADEL = ((uint32_t)0x02)<<16;
            SCLH   = ((uint32_t)0xC3)<< 8;
            SCLL   = ((uint32_t)0xC7)<< 0;
        }
        else if (100000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x0B)<<28;
            SCLDEL = ((uint32_t)0x04)<<20;
            SDADEL = ((uint32_t)0x02)<<16;
            SCLH   = ((uint32_t)0x0F)<< 8;
            SCLL   = ((uint32_t)0x13)<< 0;
        }
        else if (400000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x05)<<28;
            SCLDEL = ((uint32_t)0x03)<<20;
            SDADEL = ((uint32_t)0x03)<<16;
            SCLH   = ((uint32_t)0x03)<< 8;
            SCLL   = ((uint32_t)0x09)<< 0;
        }
        break;
    case 8000000UL: //HSI
        /* TODO: for future usage. Refer STM32F0 reference manual */
        if (10000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x01)<<28;
            SCLDEL = ((uint32_t)0x04)<<20;
            SDADEL = ((uint32_t)0x02)<<16;
            SCLH   = ((uint32_t)0xC3)<< 8;
            SCLL   = ((uint32_t)0xC7)<< 0;
        }
        else if (100000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x01)<<28;
            SCLDEL = ((uint32_t)0x04)<<20;
            SDADEL = ((uint32_t)0x02)<<16;
            SCLH   = ((uint32_t)0x0F)<< 8;
            SCLL   = ((uint32_t)0x13)<< 0;
        }
        else if (400000UL == i2c_clk)
        {
            PRESC  = ((uint32_t)0x00)<<28;
            SCLDEL = ((uint32_t)0x03)<<20;
            SDADEL = ((uint32_t)0x01)<<16;
            SCLH   = ((uint32_t)0x03)<< 8;
            SCLL   = ((uint32_t)0x09)<< 0;
        }
    }
    /* Get register configuration data */
    I2C_TIMINGR_RegVal = PRESC + SCLDEL + SDADEL + SCLH + SCLL;

    return I2C_TIMINGR_RegVal;
}
