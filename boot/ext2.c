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



uint32_t memncpy(char* dst, char* src, uint32_t size)
{
    uint32_t i;
    for(i = 0; i < size; i++)
    {
        dst[i] = src[i];
    }
    return i;
}

void memset(uint8_t* buff, uint8_t val, uint32_t size)
{
    while(size)
    {
        buff[--size] = val;
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
    fs_data.direct_block_size = 12;
    fs_data.single_indirect_block_size = fs_data.block_size / 4;
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


int allocate_block(uint32_t inode_num)
{
    disk_packet_lba28 pack;
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t block_group_idx = (inode_num - 1) / s_block.block_group_inode_count;
    while(bg_table[block_group_idx].free_block_count != 0)
    {
        block_group_idx++;
    }

    block_group_descriptor* bgd = bg_table + block_group_idx;
    pack.buff = block_buff;
    pack.lba = block_to_lba(bgd->block_bitmap_addr, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;

    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]locate_free_block\n");
        return -1;
    }

    uint32_t index, offset;
    uint32_t start = (fs_data.block_size << 3) - bgd->free_block_count; 
    uint32_t stop = fs_data.block_size << 3;
    uint32_t block_num = -1;
    for(uint32_t i = start; i < stop; i++)
    {
        index = i / 32;
        offset = i % 32;
        if(block_buff[i] ^ 1 << offset)
        {
            block_num = i;
            block_buff[i] = block_buff[i] ^ (1 << offset);
            break;
        }
    }

    if(block_num == -1)
    {
        for(uint32_t i = 0; i < stop; i++)
        {
            index = i / 32;
            offset = i % 32;
            if(block_buff[i] ^ 1 << offset)
            {
                block_num = i;
                block_buff[i] = block_buff[i] ^ (1 << offset);
                break;
            }
        }          
    }

    bgd->free_block_count -= 1;
    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]allocated_free_block\n");
        return -1;
    }
    return block_num + block_group_idx * s_block.block_group_size;
}


int read_direct_block(file_descriptor* fd, 
    uint32_t block_num, 
    uint8_t* buff, 
    uint32_t size)
{
    uint8_t block_buff[BLOCK_SIZE];
    uint32_t internal_offset = fd->file_pointer % fs_data.block_size;
    uint32_t transfer_size = fs_data.block_size - internal_offset;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;
    pack.lba = block_to_lba(block_num, fs_data.block_size);
    
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
    uint8_t* buff, uint32_t size)
{
    uint8_t block_buff[BLOCK_SIZE];
    memset(buff, 0, fs_data.block_size);
    uint32_t offset = fd->file_pointer % fs_data.block_size;
    uint32_t transfer_size = fs_data.block_size - offset;
    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;

    transfer_size = transfer_size > size ? size : transfer_size;

    if(offset != 0)
    {
        pack.lba = block_to_lba(block_num, fs_data.block_size);
        
        if(atapio_read_lba28(&pack) != pack.sector_count << 9)
        {
            terminal_printf("[ERROR]write_direct_block\n");
            return -1;
        }
    }
    
    
    memncpy(block_buff + offset, buff, transfer_size);
    pack.lba = block_to_lba(block_num, fs_data.block_size);
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
    uint8_t buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset(block_buff, 0, fs_data.block_size);

    uint32_t ret = 0, sum = 0;
    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.lba = block_to_lba(block_num, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
    if(atapio_read_lba28(&pack) == pack.sector_count << 9)
    {
        terminal_printf("[ERROR]write_single_indirect_block\n");
        return -1;
    }

    for(uint32_t i = block_offset; i < fs_data.single_indirect_block_size; i++)
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
    uint8_t* buff, 
    uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t ret = 0, sum = 0;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;
    pack.lba = block_to_lba(block_num, fs_data.block_size);
    
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]read_single_indirect_block\n");
        return -1;
    }

    for(uint32_t i = block_offset; i < fs_data.single_indirect_block_size; i++)
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
    uint8_t buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset(block_buff, 0, BLOCK_SIZE);

    uint32_t ret = 0, sum = 0;
    uint32_t start = block_offset / fs_data.single_indirect_block_size;
    uint32_t stop = fs_data.single_indirect_block_size;
    uint32_t offset = block_offset % fs_data.single_indirect_block_size;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;
    pack.lba = block_to_lba(double_block, fs_data.block_size);
    


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
    uint32_t start = block_offset / fs_data.single_indirect_block_size;
    uint32_t stop = fs_data.single_indirect_block_size;
    uint32_t offset = block_offset % fs_data.single_indirect_block_size;
    

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;
    pack.lba = block_to_lba(double_block, fs_data.block_size);
    
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
    uint8_t* buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset(block_buff, 0, BLOCK_SIZE);

    uint32_t ret = 0, sum = 0;
    uint32_t start = block_offset / fs_data.double_indirect_block_size;
    uint32_t stop = fs_data.single_indirect_block_size;
    uint32_t offset = block_offset % fs_data.double_indirect_block_size;

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.sector_count = fs_data.block_n_sectors;
    pack.lba = block_to_lba(triple_block, fs_data.block_size);
    

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
    uint8_t* buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t sum = 0, ret = 0;
    uint32_t start = block_offset / fs_data.double_indirect_block_size;
    uint32_t stop = fs_data.single_indirect_block_size;
    uint32_t offset = block_offset % (fs_data.double_indirect_block_size);

    disk_packet_lba28 pack;
    pack.buff = block_buff;
    pack.lba = block_to_lba(triple_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    
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


int write_file(file_descriptor* fd, uint8_t buff, uint32_t size)
{
    uint32_t block_offset = fd->file_pointer / fs_data.block_size;
    uint32_t ret = 0, sum = 0;
    
    for(uint32_t i = block_offset; i < fs_data.direct_block_size; i++)
    {
        if(size == 0) return sum;
        if(fd->in.direct[i] == 0) fd->in.direct[i] = allocate_block(fd->inode_num);
        ret = write_direct_block(fd, fd->in.direct[i], buff, size);
        sum += ret;
        buff += ret;
        size -= ret;
    }

    block_offset = fd->file_pointer / fs_data.block_size;
    block_offset -= fs_data.direct_block_size;
    if(fd->in.single_direct == 0) fd->in.single_direct = allocate_block(fd->inode_num);
    ret = write_single_indirect_block(fd, fd->in.single_direct, 
                                        block_offset, buff, size);
    sum += ret;
    buff += ret;
    size -= ret;
    if(size == 0) return sum;

    block_offset = fd->file_pointer / fs_data.block_size;
    block_offset -= fs_data.direct_block_size - fs_data.single_indirect_block_size;
    if(fd->in.double_indirect == 0) fd->in.double_indirect = allocate_block(fd->inode_num);
    ret = write_double_indirect_block(fd, fd->in.double_indirect, 
                                        block_offset, buff, size);
    sum += ret;
    buff += ret;
    size -= ret;

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

    
    for(uint32_t i = block_offset; i < fs_data.direct_block_size; i++)
    {
        if(size == 0) return sum;
        ret = read_direct_block(fd, fd->in.direct[i], buff, size);
        buff += ret;
        size -= ret;
        sum += ret;
    }

    block_offset = fd->file_pointer / fs_data.block_size;
    block_offset -= fs_data.direct_block_size;
    ret = read_single_indirect_block(fd, 
        fd->in.single_direct, 
        block_offset, buff, size);
    buff += ret;
    size -= ret;
    sum += ret;
    
    if(size == 0) return sum;

    block_offset = fd->file_pointer / fs_data.block_size;
    block_offset -= fs_data.single_indirect_block_size - fs_data.direct_block_size;
    ret = read_double_indirect_block(fd, 
        fd->in.double_indirect,
        block_offset,
        buff, size);
    buff += ret;
    size -= ret;
    sum += ret;

    if(size == 0) return sum;

    block_offset = fd->file_pointer / fs_data.block_size;
    block_offset -= fs_data.double_indirect_block_size - fs_data.single_indirect_block_size - fs_data.direct_block_size;
    ret = read_triple_indirect_block(fd, 
        fd->in.triple_indirect,
        block_offset,
        buff, size);    
    buff += ret;
    size -= ret;
    sum += ret;
    
    return sum;
}

int file_open(uint8_t* name, uint8_t mode)
{
    
}


void create_ext2()
{
    read_mbr();
}




