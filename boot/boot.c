#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"
#include "../include/boot.h"
#include "../include/ATAPIO.h"

boot_data bd;
uint8_t s_buff[512];

void boot_main()
{
    bd.boot_disk_num = read_drive_num();
    bd.mmap_addr = MEM_LIST_ADDR;
    terminal_init();
    interrupt_init();
    init_mem_list();
    int status = atapio_init();
    if(status == -1)
    {
        terminal_printf("ATAPIO ERR: %d", status);
    }

    uint16_t word;

    disk_packet_lba28 pack;
    pack.buff = s_buff;
    pack.sector_count = 1;
    pack.lba = 0;

    if(atapio_read_lba28(&pack) != 512)
    {
        terminal_printf("FAIL\n");
    }

    
    if(s_buff[510] == 0x55 && s_buff[511] == 0xAA)
    {
        terminal_printf("MBR FOUND\n");
    }

    terminal_clean();
    
    for(int i = 0; i < 512; i++)
    {
        terminal_printf("%X ", (uint32_t )s_buff[i]);
    }

    // terminal_printf("%X%X\n", (uint32_t)s_buff[511], (uint32_t)s_buff[510]);
    while(1)
    {
        
    }
}