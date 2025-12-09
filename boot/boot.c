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
    disk_packet_lba28 pack;
    char str[512] = "HELLO WORLD";
    char temp[512] = "";
    pack.buff = str;
    pack.lba = 1000;
    pack.sector_count = 1;

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

    if(atapio_write_lba28(&pack) != 512)
    {
        terminal_printf("msg error \n");
    }
    pack.buff = temp;
    pack.lba = 1000;
    pack.sector_count = 1;
    atapio_read_lba28(&pack);

    terminal_write(temp, 15);
    terminal_clean();
    create_ext2(1);
    read_superblock(1);
    read_block_group_descriptor(1);
    // terminal_clean();
    // file_create("/test");
    // lsdir("/");
    while(1)
    {
        
    }
}