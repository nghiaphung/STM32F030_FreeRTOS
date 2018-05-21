/*
 * @filename: serial.h
 * @date: 7/5/2018
 */
#ifndef SERIAL_H
#define SERIAL_H

/**
 * @brief Serial error flag
 */
typedef enum
{
	SERIAL_ERR_NONE      = 0,//!< SERIAL_ERR_NONE
	SERIAL_ERR_OVERRUN   = 1,//!< SERIAL_ERR_OVERRUN
	SERIAL_ERR_FRAME     = 2,//!< SERIAL_ERR_FRAME
	SERIAL_ERR_IDLE_LINE = 3,//!< SERIAL_ERR_IDLE_LINE
	SERIAL_ERR_PARITY    = 4 //!< SERIAL_ERR_PARITY
}serial_error_t;

/**!
 * RX callback function prototype
 */
typedef void (*serial_callback_t)(uint8_t error, uint8_t byte);

/**!
 * Serial configuration structure
 */
typedef struct
{
	uint32_t          baudrate;
	serial_callback_t callback;
}serial_t;

void Serial_Init (serial_t* serial);
void Serial_SendByte(uint8_t byte);
void Serial_Send(char* data, uint16_t length);

#endif /* SERIAL_H */
