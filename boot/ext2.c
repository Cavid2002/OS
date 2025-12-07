#include "../include/MBR.h"
#include "../include/ATAPIO.h"
#include "../include/VGA.h"
#include "../include/ext2.h"
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
    print_super_block();
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

int allocate_inode(uint32_t inode_num)
{
    disk_packet_lba28 pack;
    uint32_t block_buff[BLOCK_SIZE / 4];
    uint32_t block_group_idx = (inode_num - 1) / s_block.block_group_inode_count;
    
    while(bg_table[block_group_idx].free_inode_count != 0)
    {
        block_group_idx++;
    }

    block_group_descriptor* bgd = bg_table + block_group_idx;
    pack.buff = block_buff;
    pack.lba = block_to_lba(bgd->block_bitmap_addr, fs_data.block_size);
    pack.sector_count = fs_data.block_n_sectors;

    
    
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
    if(size > sizeof(directory_entry)) return -1;
    dir->inode_num = *((uint32_t*)buff);
    dir->entry_size = *(uint16_t*)(buff + 4);
    dir->name_lenght = *(uint8_t*)(buff + 6);
    dir->type = *(buff + 7);
    dir->name = buff + 8;
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

            ptr += dir.entry_size;
            remaining -= dir.entry_size;
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

    char* token = strtok(path, "/");

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

            buff += dir.entry_size;
            size -= dir.entry_size;
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
    char block_buff[BLOCK_SIZE];
    char* ptr = block_buff;
    int size = BLOCK_SIZE;
    char* filename;
    char* token = strtok(path, "/");

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
    }

    
    

}

int create_ext2(uint8_t part_id)
{
    disk_packet_lba28 pack;
    read_mbr();
    // clear_partition(part_id);
    super_block sb;
    sb.free_block_count = (mbr_table[part_id].sector_num << 9) / BLOCK_SIZE;
    sb.block_size = BLOCK_SIZE >> 10;
    sb.block_group_size = BLOCK_SIZE * 8;
    sb.block_group_inode_count = BLOCK_SIZE * 8;
    sb.total_block_count = sb.free_block_count;
    sb.total_inode_count = sb.total_block_count / 4;
    sb.signature = EXT2_SIGNATURE;
    sb.major_version = 0;
    sb.minor_version = 0;
    
    pack.lba = mbr_table[part_id].lba_start + OFFSET_SUPERBLOCK;
    pack.buff = &sb;
    pack.sector_count = 2;
    if(atapio_write_lba28(&pack) != pack.sector_count << 9)
    {
        terminal_printf("[ERROR]create_ext2\n");
        return -1;
    }

    terminal_printf("EXT2 WRITE SUCCESS!\n");
    return 0;
}






