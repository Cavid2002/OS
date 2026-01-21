#ifndef FSYS_H
#define FSYS_h

#include <stdint.h>

#define FILE_NAME_MAX                   26
#define USED_BLOCK_COUNT                258
#define USED_INODE_COUNT                10
#define INODE_SIZE                      28
#define OFFSET_SUPERBLOCK               2
#define OFFSET_BG_DESC                  4
#define BLOCK_SIZE                      4096
#define SECTOR_PER_BLOCK                8
#define SINGLE_INDIRECT_BLOCK_SIZE      BLOCK_SIZE * (BLOCK_SIZE / 4)
#define DOUBLE_INDIRECT_BLOCK_SIZE      SINGLE_DIRECT_BLOCK_SIZE * (BLOCK_SIZE / 4)
#define TRIPLE_INDIRECT_BLOCK_SIZE      DOUBLE_INDIRECT_BLOCK_SIZE * (BLOCK_SIZE / 4)

#define FSYS_TYPE_FIFO      0x1000
#define FSYS_TYPE_CHAR      0x2000
#define FSYS_TYPE_DIR       0x4000
#define FSYS_TYPE_BLCK      0x6000
#define FSYS_TYPE_FILE      0x8000
#define FSYS_TYPE_SYMLINK   0xA000
#define FSYS_TYPE_UNIX_SOCK 0xC000

#define EOF -1
#define SEEK_CUR    0 
#define SEEK_SET    1
#define SEEK_END    2

#define FSYS_SIGNATURE 0xEF53

#define ROOT_INODE 2

typedef struct
{
    uint32_t total_inode_count;
    uint32_t total_block_count;
    uint32_t free_block_count;
    uint32_t free_inode_count;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t block_group_size;
    uint32_t block_group_inode_count;
    uint32_t block_group_fragment_size;
    uint32_t bg_table_size;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t error;
    uint16_t os_id;
    uint8_t unused[972];
} __attribute__((packed)) super_block;


typedef struct
{
    uint32_t block_bitmap_addr;
    uint32_t inode_bitmap_addr;
    uint32_t inode_table_addr;
    uint16_t free_block_count;
    uint16_t free_inode_count;
} __attribute__((packed)) block_group_descriptor;


typedef struct
{
    uint16_t type_perms;
    uint16_t user_id;
    uint32_t size;
    uint32_t direct[12];
    uint32_t single_direct;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    uint32_t frag_addr;
} __attribute__((packed)) inode;

typedef struct 
{
    uint32_t inode_num;
    uint8_t name_lenght;
    uint8_t type;
    char name[26];
} __attribute__((packed)) directory_entry;


typedef struct
{
    uint32_t block_size;
    uint32_t direct_block_size;
    uint32_t single_indirect_block_size;
    uint32_t double_indirect_block_size;
    uint32_t triple_indirect_block_size;
    uint32_t block_n_sectors;
    uint32_t block_group_count;
    
} fsys_meta_data;


typedef struct
{
    inode in;
    uint32_t file_pointer;
    uint32_t inode_num;
} file_descriptor;

int create_fsys(uint8_t part_id);
int read_mbr();
int read_superblock(uint8_t part_id);
int read_block_group_descriptor(uint8_t part_id);
int file_create(char* path);
file_descriptor file_open(char* path, uint8_t mode);
int lsdir(char* path);



#endif