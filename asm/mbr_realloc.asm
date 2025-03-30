[bits 16]

mbr_realloc:
    cli       
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, 0x7C00
    mov di, 0x500
    mov ax, 256

mbr_realloc_loop:
    mov bx, [si]
    mov [di], bx
    add si, 2
    add di, 2
    dec ax
    jez mbr_loop
    jmp 0x60:read_bootloader

read_bootloader:
    sti
    mov bx, PT
    mov cx, 4
read_bootloader_loop:
    mov al, [PT]
    test al, 0x80
    jnz boot_partition_found
    add bx, 16
    dec cx
    jnz read_bootloader_loop
boot_partition_found:
    
    

drive_num: db 0x00

times (0x1b8 - ($ - $$)) db 0x00

disk_id: dd 0x0x00000000 
reserv: dw 0x0000

PT: times db 64  

dw 0xAA55   

