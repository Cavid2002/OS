[extern boot_main]

_entry:
    mov al, 'A'
    mov ah, 0x0f
    mov [0xb8002], ax
    call boot_main
    jmp $