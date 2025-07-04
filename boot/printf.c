#include "../include/printf.h"

void dec_to_str(uint8_t* buff, uint32_t* size, int value)
{
    uint32_t s = 0, i = 0;
    uint8_t temp;
    int is_negative = 0;

    if (value == 0)
    {
        buff[0] = '0';
        *size = 1;
        return;
    }

    if (value < 0)
    {
        is_negative = 1;
        value = -value;
    }

    for(s = 0; value != 0; s++)
    {
        buff[s] = value % 10 + '0';
        value /= 10;
    }

    if (is_negative)
    {
        buff[s++] = '-';
    }

    // Reverse the string
    for(i = 0; i < s / 2; i++) {
        temp = buff[i];
        buff[i] = buff[s - i - 1];
        buff[s - i - 1] = temp;
    }

    *size = s;
}


void hex_to_str(uint8_t* buff, uint32_t* size, int value)
{
    int s = 0, i = 0, temp;

    if (value == 0) {
        buff[0] = '0';
        *size = 1;
        return;
    }

    for(s = 0; value != 0; s++)
    {
        temp = value % 16;
        if(temp >= 10) buff[s] = temp - 10 + 'A'; 
        else buff[s] = temp + '0';

        value /= 16;
    }

    for(i = 0; i < s / 2; i++)
    {
        buff[i] = buff[i] + buff[s - i - 1];
        buff[s - i - 1] = buff[i] - buff[s - i - 1];
        buff[i] = buff[i] - buff[s - i - 1]; 
    }

    *size = s;
}


