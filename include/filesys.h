#ifndef FILESYS_H
#define FILESYS_H

#include <stdint.h>

#define MBR_TABLE_SIZE      4
#define EXT_MAGIC_NUMBER    0x83


typedef struct
{
    uint8_t attrib;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_stop[3];
    uint32_t lba_start;
    uint32_t sector_num;
} __attribute__((packed)) mbr_partition_table_entry;



#endif