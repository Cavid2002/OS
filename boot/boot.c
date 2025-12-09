#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"
#include "../include/boot.h"
#include "../include/ATAPIO.h"
#include "../include/PS2.h"
#include "../include/ext2.h"

boot_data bd;

void boot_main()
{
    bd.boot_disk_num = read_drive_num();
    bd.mmap_addr = MEM_LIST_ADDR;
    terminal_init();
    interrupt_init();
    init_mem_list();
    ps2_init();
    int status = atapio_init();
    if(status == -1)
    {
        terminal_printf("ATAPIO ERR: %d", status);
    }

    terminal_clean();
    atapio_init();
    read_mbr();
    create_ext2(1);
    read_superblock(1);
    read_block_group_descriptor(1);
    while(1)
    {
        
    }
}