#include "../include/MBR.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"
#include "../include/ext2.h"
#include "../include/string.h"
#include "../include/string.h"

static uint8_t mbr_buff[512];
static mbr_partition_table_entry* mbr_table;     
static super_block s_block;
static uint16_t block_size;
static ext2_meta_data meta_data;



int print_super_block()
{
    terminal_printf("[SUPER BLOCK DATA]\n");
    terminal_printf("Inode count: %d\n", s_block.total_inode_count);
    terminal_printf("Block count: %d\n", s_block.total_block_count);
    terminal_printf("Block size: %d\n", s_block.block_size);
    terminal_printf("Block group size: %d\n", s_block.block_group_size);
    terminal_printf("Block group inode: %d\n", s_block.block_group_inode_count);
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
    terminal_printf("MBR READ SUCCESS!\n");
    mbr_table = (mbr_partition_table_entry*)(mbr_buff + 0x1BE);
    for(int i = 0; i < 4; i++)
    {
        terminal_printf("PARTITION %d\n", i);
        terminal_printf("LBA: %d\n", mbr_table[i].lba_start);
        terminal_printf("Sector count: %d\n", mbr_table[i].sector_num);
        
    }
    return 0;
}

int clear_partition(uint8_t part_id)
{
    uint8_t buff[SECTOR_SIZE];
    memset(buff, 0, SECTOR_SIZE);
    uint32_t start = mbr_table[part_id].lba_start;
    uint32_t stop = start + mbr_table[part_id].sector_num;

    disk_packet_lba28 pack;
    pack.buff = buff;
    pack.sector_count = 1;
    for(uint32_t i = start; i < stop; i++)
    {
        pack.lba = i;
        if(atapio_write_lba28(&pack) != 512)
        {
            terminal_printf("[ERROR]clear_partition\n");
            return -1;
        }
    }
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


uint32_t block_to_lba(uint32_t block_num, uint32_t block_size)
{
    uint32_t res = block_num * (block_size / SECTOR_SIZE); 
    return res + mbr_table[1].lba_start;
}


int read_superblock(uint8_t part_id)
{
    disk_packet_lba28 pack;
    pack.lba = mbr_table[part_id].lba_start; 
    pack.sector_count = 2;
    pack.buff = &s_block;

    if(atapio_read_lba28(&pack) != 1024)
    {
        terminal_printf("read_superblock error\n");
        return -1;
    }

    meta_data.block_size = 1024 << s_block.block_size;
    meta_data.block_n_sectors = meta_data.block_size >> 9;
    meta_data.direct_block_size = 12;
    meta_data.single_indirect_block_size = meta_data.block_size >> 2;
    meta_data.double_indirect_block_size = 
        meta_data.single_indirect_block_size * meta_data.single_indirect_block_size;
    meta_data.triple_indirect_block_size = 
        meta_data.double_indirect_block_size * meta_data.double_indirect_block_size;
    print_super_block();
    return 0;
}

int read_block_group_descriptor(uint8_t part_id)
{
    
}




int read_inode(uint32_t inode_num, inode* in)
{
        
}

int write_inode(uint32_t inode_num, inode* in)
{
    
}


int allocate_block(uint32_t inode_num)
{
    
}


int allocate_inode(uint32_t inode_num)
{
    
}


int read_direct_block(file_descriptor* fd, 
    uint32_t block_num, 
    char* buff, 
    uint32_t size)
{
    char block_buff[BLOCK_SIZE];
    uint32_t internal_offset = fd->file_pointer % meta_data.block_size;
    uint32_t transfer_size = meta_data.block_size - internal_offset;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;
    pack.lba = block_to_lba(block_num, meta_data.block_size);
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_direct_block\n");
        return -1;
    }

    transfer_size = transfer_size > size ? size : transfer_size;
    memncpy(buff, block_buff + internal_offset, transfer_size);
    fd->file_pointer += transfer_size;
    return transfer_size; 
}

int write_direct_block(file_descriptor* fd, uint32_t block_num, 
    char* buff, uint32_t size)
{
    char block_buff[BLOCK_SIZE];
    memset(buff, 0, meta_data.block_size);
    uint32_t offset = fd->file_pointer % meta_data.block_size;
    uint32_t transfer_size = meta_data.block_size - offset;
    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;

    transfer_size = transfer_size > size ? size : transfer_size;

    if(offset != 0)
    {
        pack.lba = block_to_lba(block_num, meta_data.block_size);
        
        if(atapio_read_lba28(&pack) != pack.sector_count << 9)
        {
            terminal_printf("[ERROR]write_direct_block\n");
            return -1;
        }
    }
    
    
    memncpy(block_buff + offset, buff, transfer_size);
    pack.lba = block_to_lba(block_num, meta_data.block_size);
    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]write_direct_block\n");
        return -1;
    }
    fd->file_pointer += transfer_size;
    fd->in.size += transfer_size * (fd->file_pointer > fd->in.size);
    return transfer_size;
}

int write_single_indirect_block(
    file_descriptor* fd,
    uint32_t block_num,
    uint32_t block_offset,
    char* buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, meta_data.block_size);

    uint32_t ret = 0, sum = 0;
    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.lba = block_to_lba(block_num, meta_data.block_size);
    pack.sector_count = meta_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) == pack.sector_count << 9)
    {
        terminal_printf("[ERROR]write_single_indirect_block\n");
        return -1;
    }

    for(uint32_t i = block_offset; i < meta_data.single_indirect_block_size; i++)
    {
        if(size == 0) break;
        if(block_buff[i] == 0) block_buff[i] = allocate_block(fd->inode_num);
        ret = write_direct_block(fd, block_buff[i], buff, size);
        sum += ret;
        buff += ret;
        size -= ret;
    }


    atapio_write_lba28(&pack);
    return sum;
}



int read_single_indirect_block(
    file_descriptor* fd, 
    uint32_t block_num,
    uint32_t block_offset, 
    char* buff, 
    uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t ret = 0, sum = 0;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;
    pack.lba = block_to_lba(block_num, meta_data.block_size);
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_single_indirect_block\n");
        return -1;
    }

    for(uint32_t i = block_offset; i < meta_data.single_indirect_block_size; i++)
    {
        if(size == 0) break;
        ret = read_direct_block(fd, block_buff[i], buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
    }

    return sum;
}


int write_double_indirect_block(file_descriptor* fd,
    uint32_t double_block,
    uint32_t block_offset,
    char* buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, BLOCK_SIZE);

    uint32_t ret = 0, sum = 0;
    uint32_t start = block_offset / meta_data.single_indirect_block_size;
    uint32_t stop = meta_data.single_indirect_block_size;
    uint32_t offset = block_offset % meta_data.single_indirect_block_size;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;
    pack.lba = block_to_lba(double_block, meta_data.block_size);
    


    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_single_indirect_block\n");
        return -1;
    }

    for(uint32_t i = start; i < stop; i++)
    {
        if(size == 0) break;
        if(block_buff[i] == 0) block_buff[i] = allocate_block(fd->inode_num);
        ret = write_single_indirect_block(fd, block_buff[i], offset, buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
    }
    
    atapio_write_lba28(&pack);
    return sum;
}


int read_double_indirect_block(file_descriptor* fd,
    uint32_t double_block,  
    uint32_t block_offset, 
    uint8_t* buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t ret = 0, sum = 0;
    uint32_t start = block_offset / meta_data.single_indirect_block_size;
    uint32_t stop = meta_data.single_indirect_block_size;
    uint32_t offset = block_offset % meta_data.single_indirect_block_size;
    

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;
    pack.lba = block_to_lba(double_block, meta_data.block_size);
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_double_indirect_block\n");
        return -1;
    }

    for(uint32_t i = start; i < stop; i++)
    {
        if(size == 0) return sum;
        ret = read_single_indirect_block(fd, block_buff[i], offset, buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
        offset = 0;
    }

    atapio_write_lba28(&pack);
    return sum;
}

int write_triple_indirect_block(
    file_descriptor* fd,
    uint32_t triple_block,
    uint32_t block_offset,
    char* buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, BLOCK_SIZE);

    uint32_t ret = 0, sum = 0;
    uint32_t start = block_offset / meta_data.double_indirect_block_size;
    uint32_t stop = meta_data.single_indirect_block_size;
    uint32_t offset = block_offset % meta_data.double_indirect_block_size;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = meta_data.block_n_sectors;
    pack.lba = block_to_lba(triple_block, meta_data.block_size);
    

    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_double_indirect_block\n");
        return -1;
    }

    for(uint32_t i = start; i < stop; i++)
    {
        if(size == 0) break;
        if(block_buff[i] == 0) block_buff[i] = allocate_block(fd->inode_num);
        ret = write_double_indirect_block(fd, block_buff[i], offset, buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
        offset = 0;
    }

    return sum;
}

int read_triple_indirect_block(file_descriptor* fd, 
    uint32_t triple_block,
    uint32_t block_offset, 
    char* buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t sum = 0, ret = 0;
    uint32_t start = block_offset / meta_data.double_indirect_block_size;
    uint32_t stop = meta_data.single_indirect_block_size;
    uint32_t offset = block_offset % (meta_data.double_indirect_block_size);

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.lba = block_to_lba(triple_block, meta_data.block_size);
    pack.sector_count = meta_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_triple_indirect_block\n");
        return -1;
    }

    for(uint32_t i = start; i < stop; i++)
    {
        if(size == 0) return sum;
        ret = read_double_indirect_block(fd, block_buff[i], offset, buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
        offset = 0;
    }
    

    return sum;
}


int write_file(file_descriptor* fd, char* buff, uint32_t size)
{
    uint32_t block_offset = fd->file_pointer / meta_data.block_size;
    uint32_t ret = 0, sum = 0;
    
    for(uint32_t i = block_offset; i < meta_data.direct_block_size; i++)
    {
        if(size == 0) return sum;
        if(fd->in.direct[i] == 0) fd->in.direct[i] = allocate_block(fd->inode_num);
        ret = write_direct_block(fd, fd->in.direct[i], buff, size);
        sum += ret;
        buff += ret;
        size -= ret;
    }

    block_offset = fd->file_pointer / meta_data.block_size;
    block_offset -= meta_data.direct_block_size;
    if(fd->in.single_direct == 0) fd->in.single_direct = allocate_block(fd->inode_num);
    ret = write_single_indirect_block(fd, fd->in.single_direct, 
                                        block_offset, buff, size);
    sum += ret;
    buff += ret;
    size -= ret;
    if(size == 0) return sum;

    block_offset = fd->file_pointer / meta_data.block_size;
    block_offset -= meta_data.direct_block_size - meta_data.single_indirect_block_size;
    if(fd->in.double_indirect == 0) fd->in.double_indirect = allocate_block(fd->inode_num);
    ret = write_double_indirect_block(fd, fd->in.double_indirect, 
                                        block_offset, buff, size);
    sum += ret;
    buff += ret;
    size -= ret;

    return sum;

}


int read_file(file_descriptor* fd, char* buff, uint32_t size)
{
    uint32_t block_offset = fd->file_pointer / meta_data.block_size;
    uint32_t ret = 0, sum = 0;
    if(fd->file_pointer >= fd->in.size)
    {
        return EOF;
    }

    if(fd->file_pointer + size >= fd->in.size)
    {
        size = fd->in.size - fd->file_pointer;
    }

    
    for(uint32_t i = block_offset; i < meta_data.direct_block_size; i++)
    {
        if(size == 0) return sum;
        ret = read_direct_block(fd, fd->in.direct[i], buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
    }

    block_offset = fd->file_pointer / meta_data.block_size;
    block_offset -= meta_data.direct_block_size;
    ret = read_single_indirect_block(fd, 
        fd->in.single_direct, 
        block_offset, buff, size);
    buff += ret;
    size -= ret;
    sum += ret;
    
    if(size == 0) return sum;

    block_offset = fd->file_pointer / meta_data.block_size;
    block_offset -= meta_data.single_indirect_block_size - meta_data.direct_block_size;
    ret = read_double_indirect_block(fd, 
        fd->in.double_indirect,
        block_offset,
        buff, size);
    buff += ret;
    size -= ret;
    sum += ret;

    if(size == 0) return sum;

    block_offset = fd->file_pointer / meta_data.block_size;
    block_offset -= meta_data.double_indirect_block_size - meta_data.single_indirect_block_size - meta_data.direct_block_size;
    ret = read_triple_indirect_block(fd, 
        fd->in.triple_indirect,
        block_offset,
        buff, size);    
    buff += ret;
    size -= ret;
    sum += ret;
    
    return sum;
}








