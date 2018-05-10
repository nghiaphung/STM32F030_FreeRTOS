/*
 * @filename: serial.h
 * @date: 7/5/2018
 */
#ifndef SERIAL_H
#define SERIAL_H

void Serial_Init (void);
void Serial_SendByte(uint8_t byte);
void Serial_Send(char* data, uint16_t length);

#endif /* SERIAL_H */
