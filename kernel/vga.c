#include "../include/vga.h"


static uint16_t* addr = (uint16_t*)0xb8000;
static const uint16_t WIDTH = 80;
static const uint16_t HEIGHT = 25; 
static uint16_t POS_X;
static uint16_t POS_Y;

uint16_t entry(uint8_t color, uint8_t c)
{
    return (uint16_t)c | (uint16_t)(color << 8); 
}

uint32_t strlen(const char* str)
{
    int len = 0;
    while(str[len])
    {
        len++;
    }
    return len;
}

void terminal_init()
{
    POS_X = 0;
    POS_Y = 0;
    for(uint16_t i = 0; i < HEIGHT; i++)
    {
        for(uint16_t j = 0; j < WIDTH; j++)
        {
            addr[i * WIDTH + j] = entry(0x0f, ' ');
        }
    }
}


void terminal_putchar(uint8_t c)
{
    addr[POS_Y * WIDTH + POS_X] = entry(0x0f, c);
    if(++POS_X == WIDTH)
    {
        POS_X = 0;
        if(++POS_Y == HEIGHT)
        {
            POS_Y = 0;
        }
    }
}


void terminal_write(const char* str, uint32_t len)
{
    for(int i = 0; i < len; i++)
    {
        terminal_putchar(str[i]);
    }   
}

void terminal_puts(const char* msg)
{
    terminal_write(msg, strlen(msg));
}