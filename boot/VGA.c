#include "../include/VGA.h"
#include "../include/printf.h"

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

void terminal_clean()
{
    for(uint16_t i = 0; i < HEIGHT; i++)
    {
        for(uint16_t j = 0; j < WIDTH; j++)
        {
            addr[i * WIDTH + j] = entry(0x0f, ' ');
        }
    }
    POS_X = 0;
    POS_Y = 0;
}

void terminal_init()
{
    POS_X = 0;
    POS_Y = 0;
    terminal_clean();
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


void terminal_parse_format(uint8_t format_specifier, uint32_t** arg_addr)
{
    uint8_t arr[50];
    uint32_t size = 0;
    switch (format_specifier)
    {
    case 'd': case 'i':
        dec32_to_str(arr, &size, *(*arg_addr));
        *arg_addr = *arg_addr + 1;
        break;
    case 'x': case 'X':
        hex32_to_str(arr, &size, *(*arg_addr));
        *arg_addr = *arg_addr + 1;
        break;
    case 'c':
        terminal_putchar((uint8_t)(*(*arg_addr) & 0xFF));
        *arg_addr = *arg_addr + 1;
        return;
        break;
    case 'l': 
        *arg_addr = *arg_addr + 2;
        break;
    default:
        break;
    }

    for(int i = 0; i < size; i++)
    {
        terminal_putchar(arr[i]);
    }
}

void terminal_printf(const char* format, ...)
{
    uint32_t* argp = (uint32_t*)(&format + 1);

    for(int i = 0; format[i] != '\0'; i++)
    {
        if(format[i] == '%')
        {
            terminal_parse_format(format[++i], &argp);
            continue;
        }

        if(format[i] == '\n')
        {
            POS_Y = (POS_Y >= HEIGHT) ? 0 : POS_Y + 1;
            POS_X = 0;
            continue;
        }

        if(format[i] == '\t')
        {
            POS_X += 4;
            if(POS_X >= WIDTH)
            {
                POS_Y++;
                POS_X = POS_X - WIDTH;
            }
            continue;
        }

        terminal_putchar(format[i]);
    }
}