#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

u64 write_bytes(FILE * f, void * bytes, u64 num_bytes) {
  u64 s = fwrite(bytes, 1, num_bytes, f);
  if (s == 0) {
    printf("error writing object code to file\n");
    exit(-1);
  }
  return s;
}

int main(int argc, char ** argv) {
  // needs to be page aligned. for security, must be >= mmap_min_addr (65536). want NULL pointer derefs to segfault.
  // on ubuntu 25.04: /etc/sysctl.d/10-zeropage.conf
  u64 address = 0x10000;

  union {
    u64 address;
    u8 b[sizeof(u64)];
  } string_address;

  union {
    u32 size;
    u8 b[sizeof(u32)];
  } string_size;

  char string[] = "Hi!!!\n";
  string_size.size = sizeof(string);

  u64 string_offset =
    sizeof(Elf64_Ehdr) +
    sizeof(Elf64_Phdr) * 2;

  u64 program_offset = string_offset + string_size.size;
  string_address.address = address + string_offset;

  u8 code[] = { // not sure if its rax or eax, i don't read intel binary.
    0xb8, // mov rax, 1 (write syscall)
    1, 0, 0, 0,

    0xbf, 1, 0, 0, 0, // mov rdi, 1 (stdout)

    0x48, 0xbe, // mov rsi, string address
    /* Address to string (little endian): */
    string_address.b[0],
    string_address.b[1],
    string_address.b[2],
    string_address.b[3],
    string_address.b[4],
    string_address.b[5],
    string_address.b[6],
    string_address.b[7],

    0xba, // mov rdx, sizeof(string)
    string_size.b[0],
    string_size.b[1],
    string_size.b[2],
    string_size.b[3],

    0xf, 0x5, // syscall (write "hello" to stdout)

    0xb8, // mov rax, 60
    0x3c, 0, 0, 0,

    0x48, 0x31, 0xff, // xor rdx, rdx
    0xf, 0x5 // syscall (exit program with exit code 0 for success)
  };

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
      0, 0, 0, 0, 0, 0, 0, 0
    },
    .e_type = ET_EXEC,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,
    .e_entry = address + program_offset, // virtual address to where to start process. points to the first instruction.
    .e_phoff = sizeof(Elf64_Ehdr), // program header file offset (bytes)
    .e_shoff = 0, // section header file offset (bytes)
    .e_flags = 0,
    .e_ehsize = sizeof(Elf64_Ehdr),
    .e_phentsize = sizeof(Elf64_Phdr), // size of entry of program header, all entries are same size
    .e_phnum = 2,
    .e_shentsize = sizeof(Elf64_Shdr),
    .e_shnum = 0,
    .e_shstrndx = SHN_UNDEF,
  };

  Elf64_Phdr string_header = {
    .p_type = PT_LOAD,
    .p_flags = PF_R,
    .p_offset = string_offset,
    .p_vaddr = address + string_offset,
    .p_paddr = 0,
    .p_filesz = string_size.size,
    .p_memsz = string_size.size,
    .p_align = 0x8
  };

  Elf64_Phdr program_header = {
    .p_type = PT_LOAD,
    .p_flags = PF_X | PF_R,
    .p_offset = program_offset, // pretty sure thats what _start is doing.
    .p_vaddr = address + program_offset,
    .p_paddr = 0,
    .p_filesz = sizeof(code),
    .p_memsz = sizeof(code),
    .p_align = 0x8
  };

  assert(argc == 2);
  FILE * f = fopen(argv[1], "w");
  if (f == NULL) {
    printf("error opening file\n");
    exit(-1);
  }

  size_t total = 0;
  total += write_bytes(f, &header, sizeof(header));
  total += write_bytes(f, &string_header, sizeof(string_header));
  total += write_bytes(f, &program_header, sizeof(program_header));
  total += write_bytes(f, &string, string_size.size);
  total += write_bytes(f, &code, sizeof(code));

  fclose(f);
  printf("Bytes written to %s: %ld\n", argv[1], total);
  return 0;
}
