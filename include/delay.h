#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#define INSTR_PER_NANOSEC 2

void delay_in_ms(uint32_t ms);
void delay_in_ns(uint32_t ns);
void delay_in_us(uint32_t us);


#endif