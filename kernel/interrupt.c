#include "../include/interrupt.h"

static IdtEntry interruptTable[256];
static IdtDescriptor Idescriptor;


void IdtDescriptorInit()
{
    Idescriptor.addr = (uint32_t)interruptTable;
    Idescriptor.size = 255;
}

void IdtEntryCreate(void (func)(), uint8_t num, uint8_t type)
{
    uint32_t addr = (uint32_t)func;
    interruptTable[num].offset_low = (uint16_t)addr;
    interruptTable[num].offset_high = (uint16_t)(addr >> 16);
    interruptTable[num].flags = 0x80 | type;
    interruptTable[num].selector = 0x0008;
}