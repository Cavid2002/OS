
VBE_MODE_STRUCT_ADDR    equ     0x1000
VBE_MODE_OFFSET_SIGNATURE    equ     0
VBE_MODE_OFFSET_VERSION      equ     4
VBE_MODE_OFFSET_OEMSTR       equ     6
VBE_MODE_OFFSET_CAPAB        equ     10
VBE_MODE_OFFSET_MODE_PTR     equ     14
VBE_MODE_OFFSET_TOTAL_MMR    equ     18
VBE_MODE_OFFSET_RESERVED     equ     20

VBE_MODE_INFO   equ     0x1500

buffer_width	equ 1024    
buffer_height	equ 768
buffer_bpp		equ 8
buffer_segment	equ 0
buffer_offset	equ 0
buffer_mode		equ 0


    

vbe_configure:
    push es
    push fs

    xor ax, ax
    mov es, ax

    mov di, VBE_MODE_ADDR
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    jne _vbe_err

    mov ax, VBE_MODE_ADDR + VBE_MODE_OFFSET_MODE_PTR
    mov fs, ax
    mov si, VBE_MODE_ADDR + VBE_MODE_OFFSET_MODE_PTR + 2

_vbe_loop:
    mov dx, [fs:si]
    add si, 2
    cmp dx, 0xFFFF
    je _vbe_err
    call vbe_read_mode
    call check_vbe_mode
    cmp ax, 0
    jeq _set_vbe
    jmp _vbe_loop


_set_vbe:
    mov bx, dx
    add bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne _vbe_err
    ret
    


vbe_read_mode:
    push es
    push di
    xor ax, ax
    mov es, ax
    mov di, VBE_MODE_INFO
    mov ax, 0x4F01
    mov cx, dx
    int 0x10
    cmp ax, 0x4F00
    jne _vbe_err
    pop es
    pop di
    ret
    
check_vbe_mode:
    mov bx, VBE_MODE_INFO + 18
    cmp bx, [buffer_width]
    jne check_vbe_mode_fail

    mov bx, VBE_MODE_INFO + 20
    cmp bx, [buffer_height]
    jne check_vbe_mode_fail


    mov bx, VBE_MODE_INFO + 25
    cmp bl, [buffer_bpp]
    jne check_vbe_mode_fail

    xor ax, ax
    ret
    
check_vbe_mode_fail:
    mov ax, 0xFFFF
    ret
    

_vbe_err:
    mov ah, 0x0e 
    mov al, 'G'
    int 0x10
    jmp $


