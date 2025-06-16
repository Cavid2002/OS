#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"

void kernel_main()
{
    terminal_init();

    int a = 12 / 0;

    terminal_puts("Hello World");
    terminal_printf("\n%d %X %c\n", 256, 256, 'l');

    interrupt_init();

    call_software_interrupt();
    while(1)
    {
        
    }
}