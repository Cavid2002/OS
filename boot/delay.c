
#include "../include/delay.h"
#include "../include/portio.h"

void io_wait()
{
    out_byte(0x80, 0);
}


void delay_in_ns(uint32_t ns)
{
    ns = ns * INSTR_PER_NANOSEC * 10;
    while(ns)
    {
        ns--;
    }
}

void delay_in_us(uint32_t us)
{
    us = us * INSTR_PER_NANOSEC * 100;
    while(us)
    {
        us--;
    }
}

void delay_in_ms(uint32_t ms)
{
    ms = ms * INSTR_PER_NANOSEC * 10000;
    while(ms)
    {
        ms--;
    }
}