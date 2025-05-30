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


extern void load_idt(IdtDescriptor* addr);
extern void software_interrupt();
extern void interrupt_init();
extern void call_software_interrupt();

extern void isr_exception_0();
extern void isr_exception_1();
extern void isr_exception_2();
extern void isr_exception_3();
extern void isr_exception_4();
extern void isr_exception_5();
extern void isr_exception_6();
extern void isr_exception_7();
extern void isr_exception_8();
extern void isr_exception_9();
extern void isr_exception_10();
extern void isr_exception_11();
extern void isr_exception_12();
extern void isr_exception_13();
extern void isr_exception_14();
extern void isr_exception_15();
extern void isr_exception_16();
extern void isr_exception_17();
extern void isr_exception_18();
extern void isr_exception_19();
extern void isr_exception_20();
extern void isr_exception_21();
extern void isr_exception_22();
extern void isr_exception_23();
extern void isr_exception_24();
extern void isr_exception_25();
extern void isr_exception_26();
extern void isr_exception_27();
extern void isr_exception_28();
extern void isr_exception_29();
extern void isr_exception_30();
extern void isr_exception_31();

#endif