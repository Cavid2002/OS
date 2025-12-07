#ifndef EXT2_H
#define EXT2_h

#include <stdint.h>

#define INODE_SIZE                      128
#define OFFSET_SUPERBLOCK               2
#define OFFSET_BG_DESC                  4
#define BLOCK_SIZE                      4096
#define SINGLE_INDIRECT_BLOCK_SIZE      BLOCK_SIZE * (BLOCK_SIZE / 4)
#define DOUBLE_INDIRECT_BLOCK_SIZE      SINGLE_DIRECT_BLOCK_SIZE * (BLOCK_SIZE / 4)
#define TRIPLE_INDIRECT_BLOCK_SIZE      DOUBLE_INDIRECT_BLOCK_SIZE * (BLOCK_SIZE / 4)

#define EXT2_TYPE_FIFO      0x1000
#define EXT2_TYPE_CHAR      0x2000
#define EXT2_TYPE_DIR       0x4000
#define EXT2_TYPE_BLCK      0x6000
#define EXT2_TYPE_FILE      0x8000
#define EXT2_TYPE_SYMLINK   0xA000
#define EXT2_TYPE_UNIX_SOCK 0xC000

#define EOF -1
#define SEEK_CUR    0 
#define SEEK_SET    1
#define SEEK_END    2

#define EXT2_SIGNATURE 0xEF53

#define ROOT_INODE 2

typedef struct
{
    uint32_t total_inode_count;
    uint32_t total_block_count;
    uint32_t su_block_count;
    uint32_t free_block_count;
    uint32_t free_inode_count;
    uint32_t sb_block_num;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t block_group_size;
    uint32_t block_group_fragment_size;
    uint32_t block_group_inode_count;
    uint16_t last_mount_time;
    uint16_t last_writen_time;
    uint16_t current_consistency;
    uint16_t allowed_consistency;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t error;
    uint16_t minor_version;
    uint32_t last_consistency_time;
    uint32_t os_id;
    uint32_t major_version;
    uint32_t r_user_id;
    uint32_t r_group_id;
    uint8_t unused[940];
} __attribute__((packed)) super_block;


typedef struct
{
    uint32_t block_bitmap_addr;
    uint32_t inode_bitmap_addr;
    uint32_t inode_table_addr;
    uint16_t free_block_count;
    uint16_t free_inode_count;
    uint16_t dir_count;
    uint8_t unused[14];
} __attribute__((packed)) block_group_descriptor;


typedef struct
{
    uint16_t type_perms;
    uint16_t user_id;
    uint32_t size;
    uint32_t last_access_time;
    uint32_t creation_time;
    uint32_t last_mod_time;
    uint32_t del_time;
    uint16_t group_id;
    uint16_t hard_link_count;
    uint32_t sector_count;
    uint32_t flags;
    uint32_t os;
    uint32_t direct[12];
    uint32_t single_direct;
    uint32_t double_indirect;
    uint32_t triple_indirect;
    uint32_t reserved_low[2];
    uint32_t frag_addr;
    uint32_t reserved_high[3];
} __attribute__((packed)) inode;

typedef struct 
{
    uint32_t inode_num;
    uint16_t entry_size;
    uint8_t name_lenght;
    uint8_t type;
    uint8_t* name;
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
    
} ext2_fs_data;


typedef struct
{
    inode in;
    uint32_t file_pointer;
    uint32_t inode_num;
} file_descriptor;

int create_ext2(uint8_t part_id);
int read_mbr();
int read_superblock(uint8_t part_id);

#endif