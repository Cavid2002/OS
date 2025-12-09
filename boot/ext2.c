#include "../include/MBR.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"
#include "../include/ext2.h"
#include "../include/string.h"
#include "../include/string.h"

static uint8_t mbr_buff[512];
static mbr_partition_table_entry* mbr_table;
static mbr_partition_table_entry root_part;     
static super_block s_block;
static uint16_t block_size;
static ext2_fs_data fs_data;
static block_group_descriptor bg_table[10000];

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
    pack.lba = mbr_table[part_id].lba_start; 
    pack.sector_count = 2;
    pack.buff = &s_block;

    if(atapio_read_lba28(&pack) != 1024)
    {
        terminal_printf("read_superblock error\n");
        return -1;
    }

    // fs_data.block_size = 1024 << s_block.block_size;
    // fs_data.block_n_sectors = fs_data.block_size >> 9;
    // fs_data.block_group_count = s_block.total_block_count / s_block.block_group_size;
    // fs_data.direct_block_size = 12;
    // fs_data.single_indirect_block_size = fs_data.block_size / 4;
    // fs_data.double_indirect_block_size = fs_data.single_indirect_block_size * (fs_data.block_size / 4);
    // fs_data.triple_indirect_block_size = fs_data.double_indirect_block_size * (fs_data.block_size / 4);
    print_super_block();
    return 0;
}

int read_block_group_descriptor(uint8_t part_id)
{
    disk_packet_lba28 pack;
    uint32_t bgdt_size = s_block.total_block_count / s_block.block_group_size;
    pack.lba = mbr_table[part_id].lba_start + 2;
    pack.sector_count = (bgdt_size * sizeof(block_group_descriptor) + SECTOR_SIZE - 1 )/ SECTOR_SIZE;
    pack.buff = bg_table;

    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("read block descriptor error\n");
        return -1;
    }
    terminal_clean();
    for(int i = 0; i < bgdt_size; i++)
    {
        terminal_printf("block descriptor content %d\n", i);
        terminal_printf("block bitmap: %d\n", bg_table[i].block_bitmap_addr);
        terminal_printf("inode bitmap: %d\n", bg_table[i].inode_bitmap_addr);
        terminal_printf("inode table: %d\n", bg_table[i].inode_table_addr);
        terminal_printf("free inode: %d\n", bg_table[i].free_inode_count);
        terminal_printf("free block: %d\n", bg_table[i].free_block_count);
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
    char block_buff[BLOCK_SIZE];
    disk_packet_lba28 pack;
    uint32_t block_group = (inode_num - 1) / s_block.block_group_inode_count;
    uint32_t block_inode = (inode_num - 1) % s_block.block_group_inode_count;
    
    uint32_t inode_block = bg_table[block_group].inode_table_addr + 
                           (block_inode * INODE_SIZE) / fs_data.block_size;
    
    uint32_t offset = (block_inode * INODE_SIZE) % fs_data.block_size;
    
    pack.lba = block_to_lba(inode_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;
    pack.buff = block_buff;
    if(atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("read_inode error\n");
        return -1;
    }
    
    *in = *(inode*)(pack.buff + offset);
    
    return 0;
}

int write_inode(uint32_t inode_num, inode* in)
{
    char block_buff[BLOCK_SIZE];
    disk_packet_lba28 pack;

    uint32_t group = (inode_num - 1) / s_block.block_group_inode_count;
    uint32_t index = (inode_num - 1) % s_block.block_group_inode_count;

    block_group_descriptor* bgd = &bg_table[group];

    uint32_t inode_block = bgd->inode_table_addr + (index * INODE_SIZE) / fs_data.block_size;

    uint32_t offset = (index * INODE_SIZE) % fs_data.block_size;

    pack.buff = block_buff;
    pack.lba = block_to_lba(inode_block, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;

    if (atapio_read_lba28(&pack) != (pack.sector_count << 9))
    {
        terminal_printf("[ERROR] write_inode\n");
        return -1;
    }

    memncpy(block_buff + offset, (char*)in, sizeof(inode));

    if (atapio_write_lba28(&pack) != (pack.sector_count << 9))
    {
        terminal_printf("[ERROR] write_inode\n");
        return -1;
    }

    return 0;
}


int allocate_block(uint32_t inode_num)
{
    disk_packet_lba28 pack;
    uint32_t block_buff[BLOCK_SIZE / 4];

    uint32_t group = (inode_num - 1) / s_block.block_group_inode_count;

    while (bg_table[group].free_block_count == 0)
    {
        group++;
    }


    block_group_descriptor* bgd = &bg_table[group];

    pack.buff = block_buff;
    pack.lba = block_to_lba(bgd->block_bitmap_addr, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;

    if (atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR] Read block bitmap\n");
        return -1;
    }

    uint32_t total_bits = fs_data.block_size << 3;
    uint32_t block_num = (uint32_t)-1;
    uint32_t index, offset;
    for (uint32_t bit = USED_BLOCK_COUNT; bit < total_bits; bit++)
    {
        index = bit / 32;
        offset = bit % 32;

        if ((block_buff[index] & (1 << offset)) == 0)
        {
            block_buff[index] |= (1 << offset);
            block_num = bit;
            break;
        }
    }

    if (block_num == (uint32_t)-1)
    {
        terminal_printf("[ERROR] No free block in bitmap\n");
        return -1;
    }

    bgd->free_block_count--;

    if (atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR] Write block bitmap\n");
        return -1;
    }

    uint32_t absolute_block = block_num + group * s_block.block_group_size;

    return absolute_block;
}


int allocate_inode(uint32_t inode_num)
{
    disk_packet_lba28 pack;
    uint32_t block_buff[BLOCK_SIZE / 4];

    uint32_t group = (inode_num - 1) / s_block.block_group_inode_count;

    while (bg_table[group].free_inode_count == 0)
    {
        group++;
    }


    block_group_descriptor* bgd = &bg_table[group];

    pack.buff = block_buff;
    pack.lba = block_to_lba(bgd->inode_bitmap_addr, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;

    if (atapio_read_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR] Read block bitmap\n");
        return -1;
    }
    uint32_t res_inode = (uint32_t)-1;
    uint32_t total_bits = s_block.block_group_inode_count;
    uint32_t index, offset;
    
    for(int bit = USED_INODE_COUNT; bit < total_bits; bit++)
    {
        index = bit / 32;
        offset = bit % 32;
        if(block_buff[index] & (1 << offset) == 0)
        {
            block_buff[index] |= (1 << offset);
            res_inode = bit;
            break;
        }
    }

    if(res_inode == (uint32_t)-1)
    {
        terminal_printf("[ERROR]allocate inode");
        return -1;
    }

    bgd->free_inode_count--;
    if (atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR] Write inode bitmap\n");
        return -1;
    }

    uint32_t res = (res_inode + group * s_block.block_group_inode_count) + 1;
    return res;
}


int read_direct_block(file_descriptor* fd, 
    uint32_t block_num, 
    char* buff, 
    uint32_t size)
{
    char block_buff[BLOCK_SIZE];
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
    char* buff, uint32_t size)
{
    char block_buff[BLOCK_SIZE];
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
    char* buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, fs_data.block_size);

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
    char* buff, 
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
    char* buff, uint32_t size)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, BLOCK_SIZE);

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
    char* buff,
    uint32_t size
)
{
    uint32_t block_buff[BLOCK_SIZE / 4];
    memset((char*)block_buff, 0, BLOCK_SIZE);

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
    char* buff, uint32_t size)
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


int write_file(file_descriptor* fd, char* buff, uint32_t size)
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


int read_file(file_descriptor* fd, char* buff, uint32_t size)
{
    uint32_t block_offset = fd->file_pointer / fs_data.block_size;
    uint32_t ret = 0, sum = 0;
    if(fd->file_pointer >= fd->in.size)
    {
        return EOF;
    }

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

int parse_dir_entry(directory_entry* dir, char* buff, uint32_t size)
{
    if(size < sizeof(directory_entry)) return -1;
    dir->inode_num = *((uint32_t*)buff);
    dir->entry_size = *(uint16_t*)(buff + 4);
    dir->name_lenght = *(uint8_t*)(buff + 6);
    dir->type = *(buff + 7);
    memncpy(buff, dir->name, dir->name_lenght);
    return 0;
}

int write_dir_entry(directory_entry* dir, char* buff, uint32_t size)
{
    if(size < sizeof(directory_entry)) return -1;
    *((uint32_t*)buff) = dir->inode_num;
    *(uint16_t*)(buff + 4) = dir->entry_size;
    *(uint8_t*)(buff + 6) = dir->name_lenght;
    *(buff + 7) = dir->type;
    memncpy(buff + 8, dir->name, dir->name_lenght);
    return 0;
}

file_descriptor file_open(char* path, uint8_t mode)
{
    file_descriptor fd;

    fd.file_pointer = 0;
    fd.inode_num = ROOT_INODE;
    read_inode(ROOT_INODE, &fd.in);
    if (!path || path[0] == '\0' || strncmp(path, "/", 1) == 0) return fd;

    char* token = strtok(path, "/");
    if (!token) return fd;

    char block_buff[BLOCK_SIZE];

    while (token)
    {
        int found = 0;
        int size = read_file(&fd, block_buff, BLOCK_SIZE);
        if (size <= 0) break;

        char* ptr = block_buff;
        int remaining = size;

        directory_entry dir;

        while (parse_dir_entry(&dir, ptr, remaining) > 0)
        {
            if (strncmp(token, dir.name, dir.name_lenght) == 0)
            {
                read_inode(dir.inode_num, &fd.in);
                fd.inode_num = dir.inode_num;
                fd.file_pointer = 0;
                found = 1;
                break;
            }

            ptr += sizeof(directory_entry);
            remaining -= sizeof(directory_entry);
        }

        if (!found)
        {
            terminal_printf("file_open: \"%s\" not found\n", token);
            fd.inode_num = 0;
            return fd;
        }

        token = strtok(NULL, "/");
    }

    return fd;
}


int file_tell(file_descriptor* fd)
{
    return fd->file_pointer;
}

int file_seek(file_descriptor* fd, uint32_t offset, uint32_t origin)
{
    if(origin == SEEK_CUR) fd->file_pointer += offset;
    else if(origin == SEEK_END) fd->file_pointer = fd->in.size + origin;
    else if(origin == SEEK_SET) fd->file_pointer = origin;
    else return -1;
    return 0;
}

int lsdir(char* path)
{
    char path_cpy[255];
    memncpy(path_cpy, path, strlen(path));
    path_cpy[strlen(path)] = '\0';
    directory_entry dir;
    char block_buff[BLOCK_SIZE];
    char* buff;
    uint32_t size;
    file_descriptor fd;

    fd.file_pointer = 0;
    fd.inode_num = ROOT_INODE;
    if (read_inode(ROOT_INODE, &fd.in) < 0)
    {
        terminal_printf("[ERROR] lsdir: cannot read root inode\n");
        return -1;
    }

    if (read_file(&fd, block_buff, BLOCK_SIZE) < 0)
    {
        terminal_printf("[ERROR] lsdir: cannot read root data\n");
        return -1;
    }

    char* token = strtok(path_cpy, "/");

    while (token)
    {
        buff = block_buff;
        size = BLOCK_SIZE;

        int found = 0;

        while (parse_dir_entry(&dir, buff, size) >= 0)
        {
            if (parse_dir_entry(&dir, buff, size) < 0)
                break;

            if (strncmp(dir.name, token, dir.name_lenght) == 0)
            {
                found = 1;
                fd.file_pointer = 0;
                fd.inode_num = dir.inode_num;

                if (read_inode(dir.inode_num, &fd.in) < 0)
                    return -1;

                if (read_file(&fd, block_buff, BLOCK_SIZE) < 0)
                    return -1;

                break;
            }

            buff += sizeof(directory_entry);
            size -= sizeof(directory_entry);
        }

        if (!found)
        {
            terminal_printf("Path not found: %s\n", token);
            return -1;
        }

        token = strtok(NULL, "/");
    }

    buff = block_buff;
    size = BLOCK_SIZE;

    while (1)
    {
        if (parse_dir_entry(&dir, buff, size) < 0)
            break;

        if (dir.inode_num != 0)
        {
            terminal_write(dir.name, dir.name_lenght);
            terminal_printf("\n");
        }

        buff += dir.entry_size;
        size -= dir.entry_size;
    }

    return 0;
}

int file_create(char* path)
{
    char path_cpy[255];
    memncpy(path_cpy, path, strlen(path));
    path_cpy[strlen(path)] = '\0';
    char block_buff[BLOCK_SIZE];
    char* ptr = block_buff;
    int size = BLOCK_SIZE;
    char* filename;
    char* token = strtok(path_cpy, "/");

    file_descriptor fd;
    directory_entry dir;
    
    fd.file_pointer = 0;
    fd.inode_num = ROOT_INODE;
    read_inode(ROOT_INODE, &fd.in);

    read_file(&fd, ptr, size);

    int found = 0;
    while(token)
    {
        filename = token;
        ptr = block_buff;
        size = BLOCK_SIZE;
        found = 0;
        while(1)
        {
            if (parse_dir_entry(&dir, ptr, size) < 0)
                break;

            if(strncmp(dir.name, token, dir.name_lenght) == 0)
            {
                read_inode(dir.inode_num, &fd.in);
                fd.inode_num = dir.inode_num;
                fd.file_pointer = 0;
                read_file(&fd, block_buff, BLOCK_SIZE);
                found = 1;
                break;
            }

            ptr += dir.entry_size;
            size -= dir.entry_size;

        }

        token = strtok(NULL, "/");
        if(token && !found)
        {
            terminal_printf("[ERROR]Dir not found!\n");
            return -1;
        }

        if(!token) break;
    }
    
    ptr = block_buff;
    size = BLOCK_SIZE;

    while (1)
    {
        if (parse_dir_entry(&dir, ptr, size) < 0)
            break;

        if (strncmp(dir.name, filename, dir.name_lenght) == 0)
        {
            terminal_printf("[ERROR] File already exists!\n");
            return -1;
        }

        ptr += dir.entry_size;
        size -= dir.entry_size;
    }

    int new_inode_num = allocate_inode(fd.inode_num);
    if(new_inode_num == -1)
    {
        terminal_printf("[ERROR]file_create\n");
        return -1;
    }

    directory_entry new_dir;
    inode new_inode;
    memset((char*)(&new_inode), 0, sizeof(new_inode));
    if(write_inode(new_inode_num, &new_inode) < 0)
    {
        terminal_printf("[ERROR]file_create\n");
        return -1;
    }

    new_dir.inode_num = new_inode_num;
    new_dir.name_lenght = strlen(filename);
    memncpy(new_dir.name, filename, strlen(filename));
    new_dir.entry_size = sizeof(new_dir) - sizeof(new_dir.name) + strlen(filename);

    file_seek(&fd, 0, SEEK_END);

    if(write_file(&fd, (char*)&new_dir, sizeof(new_dir)) != sizeof(new_dir))
    {
        return -1;
    }
        
    return 0;
}

int create_ext2(uint8_t part_id)
{
    char block_buff[BLOCK_SIZE];
    memset(block_buff, 0, BLOCK_SIZE);
    disk_packet_lba28 pack;
    read_mbr();
    // clear_partition(part_id);
    super_block sb;
    sb.free_block_count = (mbr_table[part_id].sector_num << 9) / BLOCK_SIZE;
    sb.block_size = 2;
    sb.total_block_count = sb.free_block_count;
    sb.total_inode_count = sb.total_block_count / 4;
    sb.block_group_size = BLOCK_SIZE * 8;
    sb.block_group_inode_count = (BLOCK_SIZE * 8) / 4;
    sb.signature = EXT2_SIGNATURE;
    sb.major_version = 0;
    sb.minor_version = 0;
    
    uint32_t bgdt_size = sb.total_block_count / sb.block_group_inode_count; 

    for(int i = 0; i < bgdt_size; i++)
    {
        bg_table[i].block_bitmap_addr = i * sb.block_group_size + 2;
        bg_table[i].inode_bitmap_addr = i * sb.block_group_size + 3;
        bg_table[i].inode_table_addr = i * sb.block_group_size + 4;
        bg_table[i].free_inode_count = (BLOCK_SIZE * 8) / 4;
        bg_table[i].free_block_count = (BLOCK_SIZE * 8) - USED_BLOCK_COUNT;
        bg_table[i].dir_count = 0;

        pack.buff = block_buff;
        pack.lba = block_to_lba(bg_table[i].inode_bitmap_addr, BLOCK_SIZE);
        pack.sector_count = 8;
        if(atapio_write_lba28(&pack) != pack.sector_count << 9)
        {
            terminal_printf("[ERROR]create_ext2\n");
            return -1;
        } 

        pack.lba = block_to_lba(bg_table[i].block_bitmap_addr, BLOCK_SIZE);
        if(atapio_write_lba28(&pack) != pack.sector_count << 9)
        {
            terminal_printf("[ERROR]create_ext2\n");
            return -1;
        } 

    }

    bg_table[0].free_block_count -= 10;
    bg_table[0].free_inode_count -= 10;
    
    memncpy(block_buff, (char*)bg_table, sizeof(block_group_descriptor) * bgdt_size);
    pack.buff = block_buff;
    pack.lba = mbr_table[part_id].lba_start + 2;
    pack.sector_count = (sizeof(block_group_descriptor) * bgdt_size) / SECTOR_SIZE;

    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]create_ext2\n");
        return -1;
    }


    pack.lba = mbr_table[part_id].lba_start;
    pack.buff = &sb;
    pack.sector_count = 2;
    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]create_ext2\n");
        return -1;
    }

    atapio_flush_cache();
    terminal_printf("EXT2 WRITE SUCCESS!\n");
    return 0;
}






