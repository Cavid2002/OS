#include "../include/interrupt.h"


void PIC_send_eoi(uint8_t irq)
{
    if(irq >= 8)
    {
        out_byte(PIC2_CMD_PORT, PIC_EOI);
    }
    out_byte(PIC1_CMD_PORT, PIC_EOI);
}

void PIC_disable()
{
    out_byte(PIC1_CMD_PORT, PIC_DISABLE);
    out_byte(PIC2_CMD_PORT, PIC_DISABLE);
}

void PIC_remap(uint8_t offset1, uint8_t offset20)
{
    
}