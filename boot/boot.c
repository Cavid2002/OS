#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"


typedef struct 
{
    uint32_t mmap_addr;
    uint32_t mmap_size;
    uint8_t drive_num;
    uint32_t vga_buff_addr;
    uint32_t vga_buff_size;
} boot_data;


void boot_main()
{
    terminal_init();


    terminal_puts("Hello World");
    terminal_printf("\n%d %X %c\n", -256, 256, 'l');

    interrupt_init();


    init_mem_list();
    while(1)
    {

    }
}