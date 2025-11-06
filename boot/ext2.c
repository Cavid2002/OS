#include "../include/MBR.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"
#include "../include/ext2.h"

static uint8_t mbr_buff[512];
static mbr_partition_table_entry* mbr_table;
static mbr_partition_table_entry root_part;     
static super_block s_block;
static uint16_t block_size;
static ext2_fs_data fs_data;
static block_group_descriptor bg_table[10000];

void memncpy(char* src, char* dst, uint32_t size)
{
    for(int i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
}

int read_mbr()
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
            return i;
        }
    }

    terminal_printf("EXT2 partition is not found!\n");
    return -1;
}


int read_superblock(uint8_t part_id)
{
    disk_packet_lba28 pack;
    pack.lba = mbr_table[part_id].lba_start + OFFSET_SUPERBLOCK; 
    pack.sector_count = 2;
    pack.buff = &s_block;

    if(atapio_read_lba28(&pack) != 1024)
    {
        terminal_printf("read_superblock error\n");
        return -1;
    }

    fs_data.block_size = 1024 << s_block.block_size;
    fs_data.block_n_sectors = fs_data.block_size >> 9;
    fs_data.block_group_count = s_block.total_block_count / s_block.block_group_size;
    return 0;
}

int read_block_group_descriptor(uint8_t part_id)
{
    disk_packet_lba28 pack;
    pack.lba = mbr_table[part_id].lba_start + OFFSET_BG_DESC;
    pack.sector_count = fs_data.block_group_count * sizeof(block_group_descriptor) / SECTOR_SIZE;
    pack.buff = bg_table;

    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("read block descriptor error\n");
        return -1;
    }

    return 0;
}


uint32_t block_to_lba(uint32_t block_num, uint32_t block_size)
{
    uint32_t res = block_num * (block_size / SECTOR_SIZE); 
    return res + mbr_table[1].lba_start;
}

int read_inode(uint32_t inode_num, inode* in)
{
    inode* temp;
    disk_packet_lba28 pack;
    uint32_t block_group = (inode_num - 1) / s_block.block_group_inode_count;
    uint32_t block_inode = (inode_num - 1) % s_block.block_group_inode_count;
    
    uint32_t inode_block = bg_table[block_group].inode_table_addr + 
                           (block_inode * INODE_SIZE) / fs_data.block_size;
    
    uint32_t offset = (block_inode * INODE_SIZE) % fs_data.block_size;
    
    pack.lba = block_to_lba(inode_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("read_inode error\n");
        return -1;
    }
    
    *in = *(inode*)(pack.buff + offset);
    
    return 0;
}

int read_direct_block(uint32_t file_pointer,
    uint32_t* block_array, 
    uint8_t* buff, 
    uint32_t size)
{
    uint32_t block_num = file_pointer / fs_data.block_size;
    uint32_t block_offset = file_pointer % fs_data.block_size;
    uint32_t block_count = (size + block_offset + fs_data.block_size - 1) / fs_data.block_size;
    uint32_t block_array_size = fs_data.block_size / 4;
    
    disk_packet_lba28 pack;
    uint32_t copy_size, sum = 0;
    uint8_t block_buff[fs_data.block_size];
    for(int i = block_num; i < block_num + block_count; i++)
    {
        if(i >= block_array_size || size == 0) return sum;
        
        pack.lba = block_to_lba(block_array[i], fs_data.block_size);
        pack.buff = block_buff;
        pack.sector_count = fs_data.block_n_sectors;
        
        
        if(atapio_read_lba28(&pack) != pack.sector_count << 9)
        {
            terminal_printf("[ERROR]read_direct_block\n");
            return -1;
        }

        copy_size = fs_data.block_size - block_offset;
        if(copy_size > size) copy_size = size;

        memncpy(block_buff + block_offset, buff, copy_size);
        sum += copy_size;
        buff += copy_size;
        size -= copy_size;
        block_offset = 0;
    }
    return sum;
}

int read_single_indirect_block(uint32_t file_pointer, uint32_t single_block, uint8_t* buff, uint32_t size)
{
    disk_packet_lba28 pack;
    uint32_t ret;
    uint32_t block_buff[fs_data.block_size / 4];

    pack.buff = block_buff;
    pack.lba = block_to_lba(single_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_single_indirect_block\n");
        return -1;
    }

    return read_direct_block(file_pointer, block_buff, buff, size);
}


int read_file(file_descriptor* fd, uint8_t* buff, uint32_t size)
{
    

    return 0;
}



void create_ext2()
{
    
}




