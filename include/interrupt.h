#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>
#include "portio.h"

#define PIC1_CMD_PORT   0x20
#define PIC2_CMD_PORT   0xA0
#define PIC1_DATA_PORT  0x21
#define PIC2_DATA_PORT  0xA1


#define PIC_EOI         0x20
#define PIC_DISABLE     0xFF

#define ICW1_ICW4	    0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	    0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	    0x10		/* Initialization - required! */

#define ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	    0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	    0x10		/* Special fully nested (not) */


typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
}__attribute__((packed)) idt_entry;

typedef struct
{
    uint16_t size;
    uint32_t addr;
}__attribute__((packed)) idt_descriptor;


void load_idt(idt_descriptor* addr);

#endif