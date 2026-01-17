OUTPUT_FORMAT("binary")
ENTRY(boot_main)

SECTIONS
{
    . = 0x7E00;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
