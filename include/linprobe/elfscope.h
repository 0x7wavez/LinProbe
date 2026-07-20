#ifndef LINPROBE_ELFSCOPE_H
#define LINPROBE_ELFSCOPE_H

#include <elf.h>
#include <stdint.h>

const char *get_elf_class(uint8_t class);
const char *get_elf_data_encoding(uint8_t data);
const char *get_elf_version(uint8_t version);
const char *get_elf_OS_ABI(uint8_t os_abi);
const char *get_elf_OS_ABI_version(uint8_t version);
const char *get_elf_type(uint16_t type);
const char *get_elf_machine(uint16_t machine);
const char *get_segment_type(uint32_t type);
const char *get_segment_flags(uint32_t flags);
const char *get_section_type(uint32_t type);
const char *get_section_flags(uint64_t flags);
const char *get_symbol_type(unsigned char info);
const char *get_symbol_binding(unsigned char info);

#endif
