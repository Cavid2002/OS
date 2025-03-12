global out_byte
global out_word
global in_byte
global in_word


out_byte:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov edx, [ebp + 12]
    out dx, al

    mov esp, ebp
    pop ebp
    ret


out_word:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov edx, [ebp + 12]
    out dx, ax

    mov esp, ebp
    pop ebp
    ret


in_byte:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in al, dx

    mov esp, ebp
    pop ebp
    ret

in_word:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx

    mov esp, ebp
    pop ebp
    ret