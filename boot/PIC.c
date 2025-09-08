#include "../include/PIC.h"
#include "../include/portio.h"
#include "../include/delay.h"

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

uint16_t PIC_read_irr()
{
    uint16_t res = 0x0000;
    out_byte(PIC1_CMD_PORT, PIC_READ_IRR);
    out_byte(PIC2_CMD_PORT, PIC_READ_IRR);
    res |= (uint16_t)in_byte(PIC1_CMD_PORT);
    res |= (uint16_t)in_byte(PIC2_CMD_PORT) << 8;
    return res;
}

uint16_t PIC_read_isr()
{
    uint16_t res = 0x0000;
    out_byte(PIC1_CMD_PORT, PIC_READ_ISR);
    out_byte(PIC2_CMD_PORT, PIC_READ_ISR);
    res |= (uint16_t)in_byte(PIC1_CMD_PORT);
    res |= (uint16_t)in_byte(PIC2_CMD_PORT) << 8;
    return res;
}


void PIC_set_mask(uint16_t line)
{
    uint16_t addr;
    uint8_t mask;
    if(line < 8)
    {
        addr = PIC1_DATA_PORT;
    }
    else
    {
        addr = PIC2_DATA_PORT;
        line = line - 8;
    }
    
    mask = in_byte(addr) | 1 << line;
    return out_byte(addr, mask);
}


void PIC_unmask(uint16_t line)
{
    uint16_t addr;
    uint8_t mask;
    if(line < 8)
    {
        addr = PIC1_DATA_PORT;
    }
    else
    {
        addr = PIC2_DATA_PORT;
        line = line - 8;
    }
    
    mask = in_byte(addr) & ~(1 << line);
    return out_byte(addr, mask);
}


void PIC_remap(uint8_t offset1, uint8_t offset2)
{
    out_byte(PIC1_CMD_PORT, 0x11);
    io_wait();
    out_byte(PIC2_CMD_PORT, 0x11);
    io_wait();

    out_byte(PIC1_DATA_PORT, offset1);
    io_wait();
    out_byte(PIC2_DATA_PORT, offset2);
    io_wait();

    
    out_byte(PIC1_DATA_PORT, 0x04);
    io_wait();
    out_byte(PIC2_DATA_PORT, 0x02);
    io_wait();

    
    out_byte(PIC1_DATA_PORT, 0x01);
    io_wait();
    out_byte(PIC2_DATA_PORT, 0x01);
    io_wait();

    out_byte(PIC1_CMD_PORT, 0x00);
    out_byte(PIC2_CMD_PORT, 0x00);

}