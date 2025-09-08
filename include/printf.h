#ifndef PRINTF_H
#define PRINTF_H

#include <stdint.h>


void hex32_to_str(uint8_t* buff, uint32_t* size, int value);

void dec32_to_str(uint8_t* buff, uint32_t* size, int value);


#endif