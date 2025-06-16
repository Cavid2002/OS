#include "../include/printf.h"

void dec_to_str(uint8_t* buff, uint32_t* size, int value)
{
    uint32_t s = 0, i = 0;
    uint8_t temp;
    for(s = 0; value != 0; s++)
    {
        buff[s] = value % 10 + '0';
        value /= 10;
    }
    
    for(i = 0; i < s / 2; i++)
    {
        buff[i] = buff[i] + buff[s - i - 1];
        buff[s - i - 1] = buff[i] - buff[s - i - 1];
        buff[i] = buff[i] - buff[s - i - 1]; 
    }

    *size = s;
}

void hex_to_str(uint8_t* buff, uint32_t* size, int value)
{
    int s = 0, i = 0, temp;
    for(s = 0; value != 0; s++)
    {
        temp = value % 16;
        if(temp >= 10) buff[s] = temp + 'A';
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

