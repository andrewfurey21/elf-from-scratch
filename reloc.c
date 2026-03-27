#include <assert.h>
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned char u8;

u64 write_bytes(FILE * f, void * bytes, u64 num_bytes) {
  u64 total = fwrite(bytes, 1, num_bytes, f);
  if (total == 0) {
    printf("error writing object code to file\n");
    exit(-1);
  }
  return total;
}

int main(int argc, char ** argv) {
  assert(argc == 2);
  FILE * f = fopen(argv[1], "w");

  // void print_string(), assembly version in print_string.s
  u8 code[] = {
    0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, // mov rax, 1
    0x48, 0x89, 0xf2, // mov rdx, rsi (string length)
    0x48, 0x89, 0xfe, // mov rsi, rdi (string address)
    0x48, 0xc7, 0xc7, 0x01, 0x00, 0x00, 0x00, // mov rdi, 1

    0x0f, 0x05, // syscall

    0xc3 // ret
  };

  u64 symbol_table_offset = sizeof(Elf64_Ehdr) + sizeof(Elf64_Shdr) * 5;
  u64 program_offset = symbol_table_offset + sizeof(Elf64_Sym) * 2;
  u64 program_size = sizeof(code);

  u64 section_header_string_table_offset = program_offset + program_size;

  #define SHSTRTAB_SIZE 5
  char * section_header_string_table[SHSTRTAB_SIZE] = {
    "",
    ".text", // 1
    ".symtab", // 7
    ".shstrtab", // 15
    ".strtab", // 25
  };

  #define STRTAB_SIZE 2
  char * string_table[STRTAB_SIZE] = {
    "",
    "print_string", // 1
  };

  u64 section_header_string_table_size = 0;
  for (u64 i = 0; i < SHSTRTAB_SIZE; i++) {
    section_header_string_table_size +=
      strlen(section_header_string_table[i]) + 1;
  }

  u64 string_table_offset =
    section_header_string_table_offset + section_header_string_table_size;

  u64 string_table_size = 0;
  for (u64 i = 0; i < STRTAB_SIZE; i++) {
    string_table_size += strlen(string_table[i]) + 1;
  }

  Elf64_Sym null_symbol = {0};
  Elf64_Sym print_string_symbol_entry = {
    .st_name = 1, // index in string_table
    // .st_info = ELF64_ST_INFO(STB_GLOBAL, 0),
    .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC),
    .st_other = ELF64_ST_VISIBILITY(STV_DEFAULT),
    .st_shndx = 1, // must be >= 1. 0 is special undefined. (.text)
    .st_value = 0,
    .st_size = program_size,
  };

  Elf64_Ehdr elf_header = {
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
    .e_type = ET_REL,
    .e_machine = EM_X86_64,
    .e_version = EV_CURRENT,
    .e_entry = 0,
    .e_phoff = 0,
    .e_shoff = sizeof(Elf64_Ehdr),
    .e_flags = 0,
    .e_ehsize = sizeof(Elf64_Ehdr),
    .e_phentsize = 0,
    .e_phnum = 0,
    .e_shentsize = sizeof(Elf64_Shdr),
    .e_shnum = 5,
    .e_shstrndx = 2, // which section header is section header string table
  };

  Elf64_Shdr null_section_header = {0};

  Elf64_Shdr code_section_header = {
    .sh_name = 1,
    .sh_type = SHT_PROGBITS,
    .sh_flags = SHF_ALLOC | SHF_EXECINSTR,
    .sh_addr = 0,
    .sh_offset = program_offset,
    .sh_size = program_size,
    .sh_link = 0,
    .sh_info = 0,
    .sh_addralign = 16,
    .sh_entsize = 0, // 0 for non table.
  };

  Elf64_Shdr section_header_string_table_section_header = {
    .sh_name = 15,
    .sh_type = SHT_STRTAB,
    .sh_flags = 0,
    .sh_addr = 0,
    .sh_offset = section_header_string_table_offset,
    .sh_size = section_header_string_table_size,
    .sh_link = 0,
    .sh_info = 0,
    .sh_addralign = 16,
    .sh_entsize = 0,
  };

  Elf64_Shdr string_table_section_header = {
    .sh_name = 25,
    .sh_type = SHT_STRTAB,
    .sh_flags = 0,
    .sh_addr = 0,
    .sh_offset = string_table_offset,
    .sh_size = string_table_size,
    .sh_link = 0,
    .sh_info = 0,
    .sh_addralign = 16,
    .sh_entsize = 0,
  };

  Elf64_Shdr symbol_table_section_header = {
    .sh_name = 7,
    .sh_type = SHT_SYMTAB,
    .sh_flags = 0, // not sure
    .sh_addr = 0,
    .sh_offset = symbol_table_offset,
    .sh_size = sizeof(Elf64_Sym) * 2,
    .sh_link = 3, // which section header the string table is.
    .sh_info = 1,
    .sh_addralign = 16,
    .sh_entsize = sizeof(Elf64_Sym),
  };

  size_t total = 0;

  // Elf header
  total += write_bytes(f, &elf_header, sizeof(elf_header)); // good

  // Section table
  total += write_bytes(f, &null_section_header, sizeof(null_section_header)); // good
  total += write_bytes(f, &code_section_header, sizeof(code_section_header)); // good
  total += write_bytes(f, &section_header_string_table_section_header, sizeof(section_header_string_table_section_header));
  total += write_bytes(f, &string_table_section_header, sizeof(string_table_section_header));
  total += write_bytes(f, &symbol_table_section_header, sizeof(symbol_table_section_header));

  // Symbol table
  total += write_bytes(f, &null_symbol, sizeof(null_symbol));
  total += write_bytes(f, &print_string_symbol_entry, sizeof(print_string_symbol_entry));

  // Code
  total += write_bytes(f, &code, sizeof(code));
  printf("Code size: %ld\n", sizeof(code));

  // section header string table
  u64 shstrtab_size = 0;
  for (u64 i = 0; i < SHSTRTAB_SIZE; i++) {
    shstrtab_size += write_bytes(f, section_header_string_table[i], strlen(section_header_string_table[i]) + 1);
  }
  printf("shstrtabsize: %lld\n", shstrtab_size);
  total += shstrtab_size;

  // string table
  u64 strtab_size = 0;
  for (u64 i = 0; i < STRTAB_SIZE; i++) {
    strtab_size += write_bytes(f, string_table[i], strlen(string_table[i]) + 1);
  }
  total += strtab_size;
  printf("strtabsize: %lld\n", strtab_size);

  printf("Bytes written to %s: %ld\n", argv[1], total);
}
