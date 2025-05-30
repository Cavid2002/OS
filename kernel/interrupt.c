#include "../include/interrupt.h"
#include "../include/VGA.h"
#include "../include/PIC.h"


static IdtEntry interruptTable[256];
static IdtDescriptor Idescriptor;


void idt_init()
{
    Idescriptor.addr = (uint32_t)interruptTable;
    Idescriptor.size = sizeof(interruptTable) - 1;
    interruptTable[0].flags = 0;
    interruptTable[0].offset_high = 0;
    interruptTable[0].offset_low = 0;
    interruptTable[0].reserved = 0;
    interruptTable[0].selector = 0;
    load_idt(&Idescriptor);
}

void idt_entry_create(void (func)(), uint8_t num, uint8_t type)
{
    uint32_t addr = (uint32_t)func;
    interruptTable[num].offset_low = (uint16_t)addr & 0xFFFF;
    interruptTable[num].offset_high = (uint16_t)(addr >> 16);
    interruptTable[num].flags = 0x80 | type;
    interruptTable[num].selector = 0x0008;
}


void idt_init_exceptions()
{
    idt_entry_create(isr_exception_0, 0, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_1, 1, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_2, 2, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_3, 3, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_4, 4, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_5, 5, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_6, 6, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_7, 7, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_8, 8, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_9, 9, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_10, 10, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_11, 11, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_12, 12, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_13, 13, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_14, 14, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_15, 15, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_16, 16, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_17, 17, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_18, 18, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_19, 19, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_20, 20, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_21, 21, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_22, 22, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_23, 23, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_24, 24, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_25, 25, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_26, 26, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_27, 27, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_28, 28, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_29, 29, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_30, 30, GATE_TYPE_TRAP);
    idt_entry_create(isr_exception_31, 31, GATE_TYPE_TRAP);
}


void software_interrupt_routine()
{
    terminal_puts("INTERRUPT!!");
}

void interrupt_init()
{
    PIC_remap(0x20, 0x28);
    idt_init_exceptions();
    idt_entry_create(software_interrupt, 0x40, GATE_TYPE_INT);
    idt_init();
}
