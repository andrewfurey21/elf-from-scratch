.intel_syntax noprefix

.global print_string

print_string:
    mov rax, 1   # write syscall
    mov rdx, rsi # length of string
    mov rsi, rdi # address of string
    mov rdi, 1   # write to stdout
    syscall
    ret

