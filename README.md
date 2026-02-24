# elf from scratch

Program that generates minimal elf file for hello world, from [here](https://www.youtube.com/watch?v=JM9jX2aqkog).

## notes

- when you make a call to libc (so) the first time, what are the steps that happen? what happens in subsequent calls?
- what happens when you make a syscall? what is the perf penalty associated with making a syscall? vDSO?
- PLT vs PIC vs PIE
