global load_idt
global init_pic


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