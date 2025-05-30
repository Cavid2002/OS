#include <stdint.h>
#include "../include/VGA.h"
#include "../include/interrupt.h"

void kernel_main()
{
    terminal_init();

    terminal_puts("Hello World");

    interrupt_init();

    call_software_interrupt();
    while(1)
    {
        
    }
}