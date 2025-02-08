#include <stdint.h>
#include "../include/vga.h"

void kernel_main()
{
    terminal_init();

    terminal_puts("Hello World");
}