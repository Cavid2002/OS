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
    uint32_t disk_num;
} boot_data;



#endif 
