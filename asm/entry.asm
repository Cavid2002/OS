[extern boot_main]
[global read_drive_num]
_entry:
    mov al, 'A'
    mov ah, 0x0f
    mov [0xb8002], ax
    call boot_main
    jmp $

read_drive_num:
    push ebp
    mov ebp, esp

    mov ax, dx
    
    mov esp, ebp
    pop ebp
    ret