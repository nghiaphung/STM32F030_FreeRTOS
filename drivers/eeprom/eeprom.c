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

#define I2C_HW                   I2C2
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
//    I2C_InitStruct.I2C_Ack                 = I2C_Ack_Enable;
//    I2C_InitStruct.I2C_DigitalFilter       = 0x0F;
//    I2C_InitStruct.I2C_Mode                = I2C_Mode_I2C;
//    I2C_InitStruct.I2C_Timing              = _i2cTimingCal(EEPROM_CLKC_FREQ_HZ);
//    I2C_InitStruct.I2C_AnalogFilter        = I2C_AnalogFilter_Enable;
//    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
//    I2C_Init(I2C_HW, &I2C_InitStruct);
//    I2C_Cmd(I2C_HW, ENABLE);

}

int Eeprom_Write(uint16_t addr, uint8_t* data, uint16_t len)
{

}
