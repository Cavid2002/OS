#ifndef PIC_H
#define PIC_H

#include <stdint.h>
#include "portio.h"

#define PIC1_CMD_PORT   0x20
#define PIC2_CMD_PORT   0xA0
#define PIC1_DATA_PORT  0x21
#define PIC2_DATA_PORT  0xA1


#define PIC_EOI         0x20
#define PIC_DISABLE     0xFF

#define ICW1_ICW4	    0x01
#define ICW1_SINGLE	    0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	    0x08
#define ICW1_INIT	    0x10

#define ICW4_8086	    0x01	
#define ICW4_AUTO	    0x02	
#define ICW4_BUF_SLAVE	0x08	
#define ICW4_BUF_MASTER	0x0C	
#define ICW4_SFNM	    0x10

#define PIC_READ_IRR    0x0a    
#define PIC_READ_ISR    0x0b  


void PIC_send_eoi(uint8_t irq);
void PIC_disable();
uint16_t PIC_read_irr();
uint16_t PIC_read_isr();
void PIC_set_mask(uint16_t line);
void PIC_unmask(uint16_t line);
void PIC_remap(uint8_t offset1, uint8_t offset2);


#endif