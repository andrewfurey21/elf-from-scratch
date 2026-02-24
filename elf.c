#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  Elf64_Ehdr header = {
    .e_ident = {
      ELFMAG0, // 7f
      ELFMAG1, // E
      ELFMAG2, // L
      ELFMAG3, // F
      ELFCLASS64,
      ELFDATA2LSB,
      EV_CURRENT,
      ELFOSABI_SYSV,
      // EI_ABIVERSION,
      // EI_PAD,
      // EI_NIDENT, // why not use these?
      0, 0, 0, 0, 0, 0, 0, 0
    },
    .e_type = ET_EXEC,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,
    .e_entry = 0x40007f, // virtual address to where to start process, why this number? points to the first instruction.
    .e_phoff = sizeof(Elf64_Ehdr), // program header file offset (bytes)
    .e_shoff = 0, // section header file offset (bytes)
    .e_flags = 0,
    .e_ehsize = sizeof(Elf64_Ehdr),
    .e_phentsize = sizeof(Elf64_Phdr), // size of entry of program header, all entries are same size
    .e_phnum = 1, // number of entries, phnum * phentsize = program header siez
    .e_shentsize = 0, // section header entry size
    .e_shnum = 0, // number of section headers
    .e_shstrndx = SHN_UNDEF, // index of the names' section in the table
  };

  Elf64_Phdr program_header = {
    .p_type = PT_LOAD,
    .p_flags = PF_X | PF_R,
    .p_offset = (sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr)), // 64 + 56 = 0x78
    .p_vaddr = 0x400078,
    .p_paddr = 0x400078,
    .p_filesz = 44, // code = 44 bytes
    .p_memsz = 44,
    .p_align = 0x8
  };

  char code[] = {
    // .data
    'H', 'e', 'l', 'l', 'o', '!', '\n',

    // .text
    0xb8, // mov rax, 1 (write syscall)
    1, 0, 0, 0,

    0xbf, // mov rdi, 1 (stdout)
    1, 0, 0, 0,

    0x48, 0xbe, // mov rsi, hello pointer
    0x78, 0x00, 0x40, 0, 0, 0, 0, 0,

    0xba, // mov rdx, 7 (sizeof("Hello!"))
    7, 0, 0, 0,

    0xf, 0x5, // syscall (write "hello" to stdout)

    0xb8, // mov rax, 60
    0x3c, 0, 0, 0,

    0x48, 0x31, 0xff, // xor rdx, rdx

    0xf, 0x5 // syscall (exit program with exit code 0 for success)
  };

  FILE * f = fopen("file.elf", "w");
  if (f == NULL) {
    printf("error opening file\n");
    exit(-1);
  }

  size_t total = 0;
  size_t s = fwrite(&header, 1, sizeof(header), f);
  if (s == 0) {
    printf("error writing elf header file\n");
    exit(-1);
  }

  total += s;
  s = fwrite(&program_header, 1, sizeof(program_header), f);
  if (s == 0) {
    printf("error writing program header file\n");
    exit(-1);
  }

  total += s;
  s = fwrite(&code, 1, sizeof(code), f);
  if (s == 0) {
    printf("error writing object code to file\n");
    exit(-1);
  }

  total += s;
  fclose(f);

  printf("Wrote: %ld bytes to file.elf\n", total);

  return 0;
}
