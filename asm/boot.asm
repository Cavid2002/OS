[bits 16]
[org 0x7c00]

_main:
    mov ah, 0x0e
    mov al, 'H'
    int 0x10

load_kernel:
    mov ah, 0x02    ; BIOS function to read sectors
    mov al, 0x05    ; Number of sectors to read 
    mov ch, 0x00    ; Cylinder 0
    mov cl, 0x02    ; Starting Sector 2
    mov dh, 0x00    ; Head number
    mov dl, 0x80    ; Hard disk drive
    mov bx, 0
    mov es, bx
    mov bx, KERNEL_ADDR
    int 0x13
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al

enable_prtc_mode:
    cli
    lgdt [gtd_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:start_ptct_mode

[bits 32]
start_ptct_mode:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, STACK_ADDR
    mov ebp, esp

begin:
    mov al, 'A'
    mov ah, 0x0f
    mov [0xb8000], ax
    jmp KERNEL_ADDR   

loop:
    jmp loop




KERNEL_ADDR equ 0x7E00
STACK_ADDR equ 0x7B00

gtd_start:
gtd_null:
    dd 0x00000000
    dd 0x00000000
gtd_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011011b
    db 11001111b 
    db 0x00
gtd_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gtd_end:

gtd_descriptor:
    dw gtd_end - gtd_start - 1
    dd gtd_start


times 510 - ($ - $$) db 0x00
dw 0xaa55



