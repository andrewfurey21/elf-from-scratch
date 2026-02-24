# notes

```
format ELF64 executable 3
entry main
main:
          mov rax, 0x01
          mov rdi, 0x01
          mov rsi, txt
          mov rdx, txt_len
          syscall
          mov rax, 0x3c
          xor rdi, rdi
          syscall
txt:      db "Hello!", 10
txt_len = $ - txt
```


nasm:
```
global _start

section .data
message: db 'Hello!', 10

section .text
_start:
  mov rax, 1 ; syscall: write
  mov rdi, 1 ; fd arg: stdout
  mov rsi, message ; const char * buf
  mov rdx, 7 ; num bytes
  syscall
  mov rax, 60 ; exit(0)
  xor rdi, rdi
  syscall

; syscall args, return
; rdi, rsi, rdx, r10, r8, r9
; rax
```

todo: translate to gas

## questions

what is the dynamic linker actually doing? what is INTERP in the elf file?
when you make a call to libc (so) the first time, what are the steps that happen?
what happens in subsequent calls?
entry point address? whats the remapping when you do cat /proc/<pid>/maps
PLT vs PIC vs PIE
what is the perf penalty associated with making a syscall? vDSO?
program header vs section header?
how to read from stdout?
