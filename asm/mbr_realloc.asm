[bits 16]
[org 0x0600]


mbr_realloc:
    cli       
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, ax  
    
    mov si, 0x7C00
    mov di, 0x500
    mov ax, 256

mbr_loop:
    mov bx, [si]
    mov [di], bx
    add si, 2
    add di, 2
    dec ax
    cmp ax, 0
    jez mbr_loop
    
    jmp 0:read_bootloader

read_bootloader:
    sti
    

