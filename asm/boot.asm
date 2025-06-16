[bits 16]

_init:
    mov ax, 0x07C0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp 0x07C0:_main


_error_disk:
    mov ah, 0x0e 
    mov al, 'D'
    int 0x10
    jmp $

_error_mem:
    mov ah, 0x0e
    mov al, 'M'
    int 0x10
    jmp $

do_e820:
    xor ax, ax
    mov es, ax
    mov di, MEM_LIST_START + 4            ; memory detection
	xor ebx, ebx
	xor bp, bp
	mov edx, 0x0534D4150
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short _failed
	mov edx, 0x0534D4150
	cmp eax, edx
	jne short _failed
	test ebx, ebx
	je short _failed
	jmp short _jmpin
_e820lp:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short _e820f
	mov edx, 0x0534D4150
_jmpin:
	jcxz _skipent
	cmp cl, 20
	jbe short _notext
	test byte [es:di + 20], 1
	je short _skipent
_notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz _skipent
	inc bp
	add di, 24
_skipent:
	test ebx, ebx
	jne short _e820lp
_e820f:
	mov [es:MEM_LIST_START], bp
	clc
    mov ah, 0x0e
    mov al, 'S'
    int 0x10
	ret
_failed:
	stc
    call _error_mem
	ret                             

enable_a20:
    in al, 0x92                    
    or al, 2
    out 0x92, al
    ret

load_sectors:
    xor ax, ax
    mov ds, ax
    mov si, packet_addr_structure + 0x7C00   ;load_kernel
    mov ah, 0x42
    mov dl, [0x06FD]
    int 0x13
    jc _error_disk
    mov ax, 0x07C0
    mov ds, ax
    ret


_main:
    mov ah, 0x0e
    mov al, [START_LETTER]
    int 0x10
    call do_e820
    call load_sectors 
    call enable_a20


    mov ah, 0x0e
    mov al, [PROTECTED_LETTER]    ;enable_prtc_mode
    int 0x10

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:start_ptct_mode + 0x7C00

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
    jmp 0x08:0x7E00

KERNEL_ADDR equ 0x7E00
STACK_ADDR equ 0x7C00
MEM_LIST_START equ 0x0700

packet_addr_structure:
    db 0x10        ; packet size (16 bytes)
    db 0x00           ; reserved
    dw 0x000F        ; number of sectors to read (0xF)
    dw 0x7E00      ; offset of buffer
    dw 0x0000         ; segment of buffer
    dd 0x2         ; starting LBA (sector 2)
    dd 0x0
    


gdt_start:
;gdt_null
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start + 0x7C00
    dw 0x00
;kernel_gdt_code
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b 
    db 0x00
;kernel_gdt_data
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
;user_gdt_code
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11111010b
    db 11001111b
    db 0x00
;user_gdt_data
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 11110010b
    db 11001111b
    db 0x00
;task_segment 
gdt_end:

START_LETTER: db 'F'
PROTECTED_LETTER: db 'P'

times 512 - ($ - $$) db 0x00