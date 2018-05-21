/*
 * @filename: eeprom.h
 * @date: 13/5/2018
 */
#ifndef EEPROM_H
#define EEPROM_H

void Eeprom_Init (void);
int Eeprom_Write(uint16_t addr, uint8_t* data, uint16_t len);
int Eeprom_Read(uint16_t addr, uint8_t* data, uint16_t len);

#endif /* EEPROM_H */
