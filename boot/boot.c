#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"
#include "../include/boot.h"
#include "../include/ATAPIO.h"
#include "../include/PS2.h"


boot_data bd;
uint8_t mbr_buff[512];

void boot_main()
{
    bd.boot_disk_num = read_drive_num();
    bd.mmap_addr = MEM_LIST_ADDR;
    terminal_init();
    interrupt_init();
    init_mem_list();
    ps2_init();
    // int status = atapio_init();
    // if(status == -1)
    // {
    //     terminal_printf("ATAPIO ERR: %d", status);
    // }


    // disk_packet_lba28 pack;
    // pack.buff = mbr_buff;
    // pack.sector_count = 1;
    // pack.lba = 0;
    // atapio_select(0, 0);
    // if(atapio_read_lba28(&pack) != 512)
    // {
    //     terminal_printf("FAIL\n");
    // }

    // terminal_clean();
    // if(mbr_buff[510] == 0x55 && mbr_buff[511] == 0xAA)
    // {
    //     terminal_printf("MBR FOUND\n");
    // }
    
    // for(int i = 0; i < 512; i++)
    // {
    //     terminal_printf("%x ", (uint32_t )mbr_buff[i]);
    // }

    while(1)
    {
        
    }
}