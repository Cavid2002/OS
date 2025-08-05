#ifndef BOOT_H
#define BOOT_H

#include <stdint.h>


typedef struct 
{
    uint32_t partition_table_addr;
    uint32_t boot_disk_num;
    uint32_t mmap_addr;
    uint32_t frame_buffer_addr;
    uint32_t frame_buffer_width;
    uint32_t frame_buffer_height;
    uint32_t frame_buffer_bpp;
} boot_data;


void read_vbe_data(boot_data* bd)
{
    bd->frame_buffer_addr = 0x32000000;
    bd->frame_buffer_height = 768;
    bd->frame_buffer_width = 1024;
    bd->frame_buffer_bpp = 8;

    terminal_printf("Frame buffer address: 0x%X\n", bd->frame_buffer_addr);
    terminal_printf("Frame buffer size: %dx%d\n", bd->frame_buffer_width, bd->frame_buffer_height);
    terminal_printf("Frame buffer size: %d\n", bd->frame_buffer_bpp);
}

extern int read_drive_num();

#endif 
