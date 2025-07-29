#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>

typedef struct
{
    uint8_t attrib;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_stop[3];
    uint32_t lba_start;
    uint32_t sector_num;
} __attribute__((packed)) mbr_partition_table_entry;

typedef struct 
{
    uint32_t partition_table_addr;
    uint8_t boot_disk_num;
    uint32_t mmap_addr;
} boot_data;


extern int read_drive_num();

#endif 
