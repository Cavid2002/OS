#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

#define GATE_TYPE_TRAP 0x0E
#define GATE_TYPE_INT 0x0F


typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
}__attribute__((packed)) IdtEntry;

typedef struct
{
    uint16_t size;
    uint32_t addr;
}__attribute__((packed)) IdtDescriptor;


void load_idt(IdtDescriptor* addr);

#endif