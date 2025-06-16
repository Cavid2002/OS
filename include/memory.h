#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_LIST_ADDR 0x0700

typedef enum{
    MEM_USABLE = 1,
    MEM_RESERVED = 2,
    MEM_ACPI_REC = 3,
    MEM_ACPI_NVS = 4, 
    MEM_BAD = 5
} MEMTYPE;


typedef struct
{
    uint64_t base;
    uint64_t lenght;
    uint32_t type;
    uint32_t ACPI;
} __attribute__((packed)) mmap_list_entry;


typedef struct memory
{
    uint32_t mmap_list_size;
    mmap_list_entry* mmap_list;
} mmap_descriptor;

void sort_mmap();
void filter_mmap();

#endif