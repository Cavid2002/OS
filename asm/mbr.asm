[bits 16]

mbr_start:
    jmp 0x7C0:mbr_realloc

mbr_realloc:
    cli       
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov bp, sp
    
    push dx

    mov si, 0x7C00
    mov di, 0x500
    mov ax, 256
    

mbr_realloc_loop:
    mov bx, [si]
    mov [di], bx
    add si, 2
    add di, 2
    dec ax
    jnz mbr_realloc_loop
    jmp 0x50:read_bootloader

read_bootloader:
    sti
    mov bx, PT1 + 0x0500
    mov cx, 4
read_bootloader_loop:
    mov al, [bx]
    test al, 0x80
    jnz boot_partition_found
    add bx, 16
    dec cx
    jnz read_bootloader_loop
;disk_error
    mov ah, 0x0e
    mov al, 'B'
    int 0x10
    jmp $


boot_partition_found:
    mov si, packet_addr_structure + 0x0500
    mov ah, 0x42
    mov dx, [bp - 2]
    int 0x13
    jc _disk_error
    jmp 0x00:0x7C00
    jmp $

_disk_error:
    mov ah, 0x0e
    mov al, 'E'
    int 0x10
    jmp $
    

drive_num: db 0x00

packet_addr_structure:
    db 0x10
    db 0x00
    dw 0x0001
    dw 0x7C00
    dw 0x0000
    dd 0x1
    dd 0x0
     


times (0x1B8 - ($ - $$)) db 0x00

;disk_id
dd 0x0000
dw 0x0000

PT1:
    db 0x80     ;PT1_ATTRIB 
    db 0x01     ;PT1_START_CHS
    db 0x00
    db 0x00
    db 0x60     ;PT1_TYPE
    db 0x1F     ;PT1_STOP_CHS
    db 0x00
    db 0x00
    dd 0x01     ;PT1_LBA
    dd 0xFF     ;PT1_SECTOR_NUM
PT1_end:
PT2:
    db 0x00     ;PT2_ATTRIB
    db 0x00     ;PT2_START_CHS
    db 0x00
    db 0x00
    db 0x83     ;PT2_TYPE
    db 0x00     ;PT2_STOP_CHS
    db 0x00
    db 0x00     
    dd 0x100     ;PT2_LBA
    dd 10000000  ;PT2_SECTOR_NUM
times 32 db 0x00

dw 0xAA55   

