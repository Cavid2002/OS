OUTPUT_FORMAT("binary")

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