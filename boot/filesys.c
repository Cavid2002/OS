#include "../include/filesys.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"

static uint8_t mbr_buff[512];
static mbr_partition_table_entry* mbr_table;
static mbr_partition_table_entry root_part;     


int read_mbr(uint8_t* buff)
{
    disk_packet_lba28 pack;
    pack.buff = mbr_buff;
    pack.lba = 0;
    pack.sector_count = 1;

    if(atapio_read_lba28(&pack) != 512)
    {
        terminal_printf("MBR READ ERROR!\n");
        return -1;     
    }

    mbr_table = (mbr_partition_table_entry*)(mbr_buff + 0x1FE);
    return 0;
}

int write_mbr()
{
    disk_packet_lba28 pack;
    pack.buff = mbr_buff;
    pack.lba = 0;
    pack.sector_count = 1;

    if(atapio_write_lba28(&pack) != 512)
    {
        terminal_printf("MBR WRITE ERROR!\n");
        return -1;
    }
    return 0;
}


int locate_ext_partition()
{
    for(int i = 0; i < MBR_TABLE_SIZE; i++)
    {
        if(mbr_table[i].type == EXT_MAGIC_NUMBER)
        {
            root_part = mbr_table[i];
            return 0;
        }
    }

    terminal_printf("EXT2 partition is not found!\n");
    return -1;
}





