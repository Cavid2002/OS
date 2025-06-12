#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_LIST_ADDR 0x0700


typedef struct
{
    uint32_t base_low;
    uint32_t base_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
    uint32_t ACPI;
} __attribute__((packed)) memory_list_entry;


#endif