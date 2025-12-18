#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"
#include "../include/boot.h"
#include "../include/ATAPIO.h"
#include "../include/PS2.h"
#include "../include/ext2.h"
#include "../include/delay.h"

boot_data bd;

void foo()
{
    terminal_clean();
    disk_packet_lba28 pack;
    char str[BLOCK_SIZE] = "HELLO WORLD";
    char temp[BLOCK_SIZE] = "";
    pack.buff = str;
    pack.lba = 1000;
    pack.sector_count = 4;

    for(int i = 0; i < 10; i++)
    {
        atapio_write_lba28(&pack);
        pack.lba += 8;
    }


    pack.lba = 1000;
    pack.buff = temp;
    for(int i = 0; i < 10; i++)
    {
        atapio_read_lba28(&pack);
        terminal_printf("%d content", pack.lba);
        terminal_puts(pack.buff);
        terminal_printf("\n");
        pack.lba += 8;
    }
    
}


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

    foo();
    terminal_clean();
    delay_in_ms(200);
    create_ext2(1);
    read_superblock(1);
    delay_in_ms(200);
    terminal_clean();
    read_block_group_descriptor(1);
    while(1)
    {
        
    }
}