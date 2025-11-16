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
    fs_data.single_indirect_block_size = fs_data.block_size * (fs_data.block_size / 4);
    fs_data.double_indirect_block_size = fs_data.single_indirect_block_size * (fs_data.block_size / 4);
    fs_data.triple_indirect_block_size = fs_data.double_indirect_block_size * (fs_data.block_size / 4);
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

int read_direct_block(uint32_t fp, uint32_t block_num, 
                    uint8_t* buff, uint32_t size)
{
    uint32_t offset = fp % fs_data.block_size;
    uint8_t block_buff[fs_data.block_size];
    
    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.lba = block_to_lba(block_num, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR] read_direct_block");
        return -1;
    }

    uint32_t available_bytes = fs_data.block_size - offset;
    uint32_t transfer_size = (size < available_bytes) ? size : available_bytes;

    memcpy(buff, block_buff + offset, transfer_size);

    return transfer_size;
}

int read_single_indirect_block(uint32_t fp, 
    uint32_t single_block, 
    uint32_t block_offset, 
    uint8_t* buff, uint32_t size)
{
    uint32_t sum = 0, ret = 0;
    disk_packet_lba28 pack;
    uint32_t block_arr[fs_data.block_size / 4];

    pack.buff = block_arr;
    pack.lba = block_to_lba(single_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_single_indirect_block\n");
        return -1;
    }

    for(int i = block_offset; i < fs_data.block_size / 4; i++)
    {
        if(size == 0) return sum;
        ret = read_direct_block(fp, block_arr[i], buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
    }

    return sum;
}

int read_double_indirect_block(uint32_t fp, 
                                uint32_t double_block, 
                                uint32_t block_offset, 
                                uint8_t* buff, uint32_t size)
{
    uint32_t sum = 0, ret = 0;
    disk_packet_lba28 pack;
    uint32_t block_arr[fs_data.block_size / 4];
    uint32_t single_block_offset = (fp % fs_data.single_indirect_block_size) / fs_data.block_size;

    pack.buff = block_arr;
    pack.lba = block_to_lba(double_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_double_indirect_block\n");
        return -1;
    }

    for(int i = block_offset; i < fs_data.block_size / 4; i++)
    {
        if(size == 0) return sum;
        ret = read_single_indirect_block(fp, block_arr[i], single_block_offset ,buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
        single_block_offset = 0;
    }

    return sum;
}

int read_triple_indirect_block(uint32_t fp, uint32_t triple_block,
                                uint32_t block_offset, uint8_t* buff, uint32_t size)
{
    uint32_t sum = 0, ret = 0;
    disk_packet_lba28 pack;
    uint32_t block_arr[fs_data.block_size / 4];
    uint32_t double_block_offset = (fp % fs_data.double_indirect_block_size) / fs_data.single_indirect_block_size;


    pack.buff = block_arr;
    pack.lba = block_to_lba(triple_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_triple_indirect_block\n");
        return -1;
    }

    for(int i = block_offset; i < fs_data.block_size / 4; i++)
    {
        if(size == 0) return sum;
        ret = read_double_indirect_block(fp, block_arr[i], double_block_offset,buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
        double_block_offset = 0;
    }

    return sum;
}


int read_file(file_descriptor* fd, uint8_t* buff, uint32_t size)
{
    uint32_t block_offset = fd->file_pointer / fs_data.block_size;
    uint32_t ret = 0, sum = 0;
    if(fd->file_pointer + size >= fd->in.size)
    {
        size = fd->in.size - fd->file_pointer;
    }

    for(int i = block_offset; i < 12; i++)
    {
        if(size == 0) return sum;
        ret = read_direct_block(fd->file_pointer, fd->in.direct[i], buff, size);
        buff += ret;
        size -= ret;
        fd->file_pointer += ret;
        sum += ret;
    }

    ret = read_single_indirect_block(fd->file_pointer, fd->in.single_direct, 
                                        block_offset - 12, buff, size);
    buff += ret;
    size -= ret;
    fd->file_pointer += ret;
    sum += ret;
    
    if(size == 0) return sum;
    block_offset = fd->file_pointer / fs_data.single_indirect_block_size;
    ret = read_double_indirect_block(fd->file_pointer, fd->in.double_indirect,
                                    block_offset, buff, size);
    buff += ret;
    size -= ret;
    fd->file_pointer += ret;
    sum += ret;

    if(size == 0) return sum;
    
    block_offset = fd->file_pointer / fs_data.double_indirect_block_size;
    ret = read_triple_indirect_block(fd, fd->in.triple_indirect, 
                                    block_offset, buff, size);
    buff += ret;
    size -= ret;
    fd->file_pointer += ret;
    sum += ret;


    return sum;
}



void create_ext2()
{
    
}




