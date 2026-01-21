#include "../include/MBR.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"
#include "../include/fsys.h"
#include "../include/string.h"
#include "../include/string.h"

static uint8_t mbr_buff[512];
static mbr_partition_table_entry* mbr_table;     
static super_block s_block;
static uint16_t block_size;
static fsys_meta_data meta_data;



int print_super_block()
{
    terminal_printf("[SUPER BLOCK DATA]\n");
    terminal_printf("Total Inode count: %d\n", s_block.total_inode_count);
    terminal_printf("Total Block count: %d\n", s_block.total_block_count);
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

int read_block_group_descriptors(uint32_t part_id)
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

int set_superblock(uint8_t part_id)
{
    read_mbr();
    uint32_t total_sector = mbr_table[part_id].sector_num;
    uint32_t total_block = total_sector / SECTOR_PER_BLOCK;
    uint32_t block_size = BLOCK_SIZE;

    memset((uint8_t*)&s_block, 0, sizeof(s_block));

    s_block.block_size = BLOCK_SIZE;
    s_block.total_block_count = total_block;
    s_block.total_inode_count = s_block.total_block_count >> 2;
    s_block.free_block_count = total_block - 1;
    s_block.free_inode_count = s_block.total_inode_count - 10;
    s_block.block_group_size = s_block.block_size << 3;
    
    s_block.bg_table_size = 
            s_block.total_block_count / s_block.block_group_size; 

    s_block.block_group_inode_count = 
            s_block.total_inode_count / (s_block.bg_table_size);


    disk_packet_lba28 pack;
    pack.buff = &s_block;
    pack.lba = mbr_table[part_id].lba_start;
    pack.sector_count = 2;
    
    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]set_superblock\n");
        return -1;
    }

    terminal_printf("[SUCCESS]set_superblock\n");
    return 0;    
}


int set_bgd_table(uint32_t part_id)
{
    read_mbr();
    read_superblock(part_id);
    
    block_group_descriptor table[1000]; 
    uint32_t bg_size = s_block.bg_table_size;
    uint32_t bg_size_inbytes = sizeof(block_group_descriptor) * 1000; 
    uint32_t sector_size = (bg_size_inbytes + SECTOR_SIZE - 1) / SECTOR_SIZE;   
    uint32_t offset = sector_size / (s_block.block_size >> 9) + 1;
    memset((uint8_t*)table, 0, bg_size_inbytes);

    for(uint32_t i = 0; i < bg_size; i++)
    {
        table[i].block_bitmap_addr = offset * i;
        table[i].inode_bitmap_addr = offset * i + 1;
        table[i].inode_table_addr = offset * i + 2;
        table[i].free_block_count = i;
        table[i].free_inode_count = i;
    }

    uint8_t* temp = (uint8_t*)table;
    disk_packet_lba28 pack;

    for(uint32_t i = 0; i < sector_size; i++)
    {
        pack.buff = temp;
        pack.lba = 2 + i;
        pack.sector_count = 1;
        
        if(atapio_write_lba28(&pack) != 1 << 9)
        {
            terminal_printf("[ERROR]set_bgd_table\n");
            return -1;
        }
        temp += 512;
    }

    terminal_printf("bgd_table_set complete");
    return 0;
}


int create_fsys(uint8_t part_id)
{
    if(set_superblock(part_id) < 0) return -1;
    if(set_bgd_table(part_id) < 0) return -1;

    return 0;
}
