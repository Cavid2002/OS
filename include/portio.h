#ifndef PORTIO_H
#define PORTIO_H

#include <stdint.h>

uint8_t in_byte(uint16_t addr);
uint8_t in_word(uint16_t addr);

void out_byte(uint16_t addr, uint8_t val);
void out_word(uint16_t addr, uint16_t val);


#endif