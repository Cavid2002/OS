#include "../include/keyboard.h"
#include "../include/portio.h"
#include "../include/interrupt.h"


void io_wait()
{
    out_byte(0x80, 0);
}

void PS2_init()
{
    out_byte(PS2_CMD, 0xAD);
    out_byte(PS2_CMD, 0xA7);
    in_byte(PS2_DATA);
    
}

//TODO