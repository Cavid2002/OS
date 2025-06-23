[extern software_interrupt_routine]
[extern keyboard_interrupt_routine]
[extern exception_handler]
[global load_idt]
[global software_interrupt]
[global keyboard_interrupt]
[global call_software_interrupt]


%macro isr_exception 1
[global isr_exception_%1]
isr_exception_%1:
    pushad
    cld
    push dword %1
    call exception_handler
    add esp, 4
    popad
    cli
    jmp $
    iret
%endmacro

isr_exception 0
isr_exception 1
isr_exception 2
isr_exception 3
isr_exception 4
isr_exception 5
isr_exception 6
isr_exception 7
isr_exception 8
isr_exception 9
isr_exception 10
isr_exception 11
isr_exception 12
isr_exception 13
isr_exception 14
isr_exception 15
isr_exception 16
isr_exception 17
isr_exception 18
isr_exception 19
isr_exception 20
isr_exception 21
isr_exception 22
isr_exception 23
isr_exception 24
isr_exception 25
isr_exception 26
isr_exception 27
isr_exception 28
isr_exception 29
isr_exception 30
isr_exception 31



software_interrupt:
    pushad
    cld
    call software_interrupt_routine
    popad
    iret

keyboard_interrupt:
    pushad
    cld
    call keyboard_interrupt_routine
    popad
    iret

load_idt:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]
    cli
    lidt [eax]
    sti

    mov esp, ebp
    pop ebp
    ret

call_software_interrupt:
    push ebp
    mov ebp, esp

    int 0x40

    mov esp, ebp
    pop ebp
    ret