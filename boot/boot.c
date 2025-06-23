#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"
#include "../include/memory.h"

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