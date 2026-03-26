.intel_syntax noprefix

.global print_string

print_string:
    mov rax, 1
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, 1
    syscall
    ret

