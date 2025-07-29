#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"
#include "../include/boot.h"
#include "../include/ATAPIO.h"

boot_data bd;

void boot_main()
{
    bd.boot_disk_num = read_drive_num();
    terminal_init();

    

    terminal_puts("Hello World");
    terminal_printf("\n%d %X %c %X\n", -256, 256, 'l', bd.boot_disk_num);

    interrupt_init();


    init_mem_list();

    atapio_init();

    while(1)
    {

    }
}