#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>








/*
 * get_elf_class - converts an ELF class value to a human readable string.
 * class is a raw integer in Elf64_Ehdr — this function maps it to the standard class name.
 */
const char *get_elf_class(uint8_t class) {
    switch (class) {
        case ELFCLASS32: return "32-bit";
        case ELFCLASS64: return "64-bit";
        default: return "Unknown";
    }
}


/*
 * get_elf_data_encoding - converts an ELF data encoding value to a human readable string.
 * data is a raw integer in Elf64_Ehdr — this function maps it to the standard data encoding name.
 */
const char *get_elf_data_encoding(uint8_t data) {
    switch (data) {
        case ELFDATA2LSB: return "Little Endian";
        case ELFDATA2MSB: return "Big Endian";
        default: return "Unknown";
    }
}

/*  
 * get_elf_version - converts an ELF version value to a human readable string.
 * version is a raw integer in Elf64_Ehdr — this function maps it to the standard version name.
 */
const char *get_elf_version(uint8_t version) {
    switch (version) {
        case EV_NONE: return "Invalid";
        case EV_CURRENT: return "Current";
        default: return "Unknown";
    }
}

/*
 * get_elf_OS_ABI - converts an ELF OS/ABI value to a human readable string.
 * os_abi is a raw integer in Elf64_Ehdr — this function maps it to the standard OS/ABI name.
 */
const char *get_elf_OS_ABI(uint8_t os_abi) {
    switch (os_abi) {
        case ELFOSABI_SYSV: return "System V";
        case ELFOSABI_HPUX: return "HP-UX";
        case ELFOSABI_NETBSD: return "NetBSD";
        case ELFOSABI_LINUX: return "Linux";
        case ELFOSABI_SOLARIS: return "Solaris";
        case ELFOSABI_AIX: return "AIX";
        case ELFOSABI_IRIX: return "IRIX";
        case ELFOSABI_FREEBSD: return "FreeBSD";
        case ELFOSABI_TRU64: return "TRU64";
        case ELFOSABI_MODESTO: return "Modesto";
        case ELFOSABI_OPENBSD: return "OpenBSD";
        default: return "Unknown";
    }
}

const char *get_elf_OS_ABI_version(uint8_t version) {
    switch (version) {
        case 0: return "0 (System V)";
        default: return "Unknown";
    }
}


/*
 * get_elf_type - converts an ELF type value to a human readable string.
 * type is a raw integer in Elf64_Ehdr — this function maps it to the standard file type name.
 */
const char *get_elf_type(uint16_t type) {
    switch (type) {
        case ET_NONE: return "No file type";
        case ET_REL: return "Relocatable file";
        case ET_EXEC: return "Executable file";
        case ET_DYN: return "Shared object file";
        case ET_CORE: return "Core file";
        default: return "Unknown";
    }
}

/*
 * get_elf_machine - converts an ELF machine value to a human readable string.
 * machine is a raw integer in Elf64_Ehdr — this function maps it to the standard architecture name.
 */
const char *get_elf_machine(uint16_t machine) {
    switch (machine) {
        case EM_NONE: return "No machine";
        case EM_386: return "Intel 80386";
        case EM_AARCH64: return "AArch64";
        case EM_MIPS: return "MIPS";
        case EM_PARISC: return "PA-RISC";
        case EM_SPARC32PLUS: return "SPARC v8+";
        case EM_PPC: return "PowerPC";
        case EM_PPC64: return "PowerPC 64-bit";
        case EM_S390: return "IBM S/390";
        case EM_ARM: return "ARM";
        case EM_SH: return "SuperH";
        case EM_SPARCV9: return "SPARC v9";
        case EM_IA_64: return "Intel Itanium";
        case EM_X86_64: return "AMD x86-64";
        case EM_VAX: return "DEC VAX";
        default: return "Unknown";
    }
}

/*
 * get_segment_type - converts a program header p_type value
 * to a human readable string.
 * p_type is a raw integer in Elf64_Phdr — this function
 * maps it to the standard segment name.
 */
const char *get_segment_type(uint32_t type) {
    switch (type) {
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        case PT_LOPROC: return "LOPROC";
        case PT_HIPROC: return "HIPROC";
        case PT_GNU_STACK: return "GNU_STACK";
        default: return "UNKNOWN";
    }
}

const char *get_symbol_type(unsigned char info) {
    switch (ELF64_ST_TYPE(info)) {
        case STT_NOTYPE:  return "NOTYPE";
        case STT_OBJECT:  return "OBJECT";
        case STT_FUNC:    return "FUNC";
        case STT_SECTION: return "SECTION";
        case STT_FILE:    return "FILE";
        default:          return "UNKNOWN";
    }
}

const char *get_symbol_binding(unsigned char info) {
    switch (ELF64_ST_BIND(info)) {
        case STB_LOCAL:  return "LOCAL";
        case STB_GLOBAL: return "GLOBAL";
        case STB_WEAK:   return "WEAK";
        default:         return "UNKNOWN";
    }
}



/*
* get_segment_flags - converts a program header p_flags value
* to a human readable string.
* p_flags is a raw integer in Elf64_Phdr — this function
* maps it to the standard flag name.
*/
const char *get_segment_flags(uint32_t flags) {
    static char buffer[4]; // Enough to hold "RWE" + null terminator
    int pos = 0;

    if (flags & PF_R) buffer[pos++] = 'R';
    if (flags & PF_W) buffer[pos++] = 'W';
    if (flags & PF_X) buffer[pos++] = 'E';
    buffer[pos] = '\0'; // Null-terminate the string

    return buffer;
}

/* 
 * get_section_type - converts a section header sh_type value
 * to a human readable string.
 * sh_type is a raw integer in Elf64_Shdr — this function
 * maps it to the standard section name.
 */
const char *get_section_type(uint32_t type) {
    switch (type) {
        case SHT_NULL: return "NULL";
        case SHT_PROGBITS: return "PROGBITS";
        case SHT_SYMTAB: return "SYMTAB";
        case SHT_STRTAB: return "STRTAB";
        case SHT_RELA: return "RELA";
        case SHT_HASH: return "HASH";
        case SHT_DYNAMIC: return "DYNAMIC";
        case SHT_NOTE: return "NOTE";
        case SHT_NOBITS: return "NOBITS";
        case SHT_LOPROC: return "LOPROC";
        case SHT_HIPROC: return "HIPROC";
        case SHT_DYNSYM: return "DYNSYM";
        default: return "UNKNOWN";
    }
}

/*
 * get_section_flags - converts a section header sh_flags value
 * to a human readable string.
 * sh_flags is a raw integer in Elf64_Shdr — this function
 * maps it to the standard flag name.
 */
const char *get_section_flags(uint64_t flags) {
    static char buffer[5]; // Enough to hold "WAXS" + null terminator
    int pos = 0;

    if (flags & SHF_WRITE) buffer[pos++] = 'W';
    if (flags & SHF_ALLOC) buffer[pos++] = 'A';
    if (flags & SHF_EXECINSTR) buffer[pos++] = 'X';
    if (flags & SHF_MASKPROC) buffer[pos++] = 'S';

    buffer[pos] = '\0'; // Null-terminate the string

    return buffer;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
     /* open the binary file in read-only mode
     * O_RDONLY — no writes, we are only inspecting */
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    /* fstat populates file_info with metadata about the file
     * we need file_info.st_size to tell mmap how many bytes to map */
    struct stat file_info;
    if (fstat(fd, &file_info) < 0) {
        perror("Error getting file size");
        close(fd);
        return EXIT_FAILURE;
    }


    /* mmap maps the entire file into our process's virtual address space
     * returns a void* pointer to the start of the mapped region
     * PROT_READ     — read only
     * MAP_PRIVATE   — changes (if any) do not affect the file
     * after mmap succeeds, fd is no longer needed */
    char *map = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return EXIT_FAILURE;
    }

    /* fd is closed immediately after mmap
     * the mapping stays alive independently of the file descriptor */
    close(fd);



    /* cast the raw mapped memory to Elf64_Ehdr*
     * the first 64 bytes of every ELF file match this struct's layout exactly
     * no data is copied — we are just telling C how to interpret the bytes */
    Elf64_Ehdr *header = (Elf64_Ehdr *)map;

    /* verify the ELF magic bytes — first 4 bytes of every valid ELF file
     * 0x7f 'E' 'L' 'F'
     * if these don't match, this is not an ELF file */
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Error: Not a valid ELF file\n");
        munmap(map, file_info.st_size);
        return EXIT_FAILURE;
    }
        printf("Valid ELF file: %s\n", argv[1]);


    printf("Architectural Class:            %s\n", get_elf_class(header->e_ident[EI_CLASS]));               // EI_CLASS is the index in e_ident where the class byte is located
    printf("Data Encoding:                  %s\n", get_elf_data_encoding(header->e_ident[EI_DATA]));        // EI_DATA is the index in e_ident where the data encoding byte is located
    printf("ELF Version:                    %s\n", get_elf_version(header->e_ident[EI_VERSION]));           // EI_VERSION is the index in e_ident where the version byte is located
    printf("OS ABI:                         %s\n", get_elf_OS_ABI(header->e_ident[EI_OSABI]));
    printf("ABI Version:                    %s\n", get_elf_OS_ABI_version(header->e_ident[EI_ABIVERSION]));                 // EI_ABIVERSION is the index in e_ident where the ABI version byte is located
    printf("File Type:                      %s\n", get_elf_type(header->e_type));                              // e_type is a field in Elf64_Ehdr that indicates the file type (executable, shared object, etc)
    printf("Machine Architecture:           %s\n", get_elf_machine(header->e_machine));                        // e_machine is a field in Elf64_Ehdr that indicates the target architecture (x86, ARM, etc)


    printf("Entry Point:                    0x%lx\n", (unsigned long)header->e_entry);                         // e_entry is the virtual address where execution starts when the program is loaded
    printf("Program Header Table Offset:    0x%lx\n", (unsigned long)header->e_phoff);                      // e_phoff is the byte offset from the start of the file to the program header table        
    printf("Section Header Table Offset:    0x%lx\n", (unsigned long)header->e_shoff);                      // e_shoff is the byte offset from the start of the file to the section header table    
    printf("Flags:                          0x%x\n",  header->e_flags);                                     // e_flags is a field in Elf64_Ehdr that contains architecture-specific flags (often 0 for x86-64)
    printf("ELF Header Size:                %u bytes\n", header->e_ehsize);                                 // e_ehsize is the size of the ELF header itself (should be 64 bytes for 64-bit ELF)
    printf("Program Header Entry Size:      %u bytes\n", header->e_phentsize);                              // e_phentsize is the size of each entry in the program header table (should be 56 bytes for 64-bit ELF)   
    printf("Section Header Entry Size:      %u bytes\n", header->e_shentsize);                              // e_shentsize is the size of each entry in the section header table (should be 64 bytes for 64-bit ELF)
    printf("String Table Section Index:     %u\n",    header->e_shstrndx);                                  // e_shstrndx is the index of the section header that contains the string table for section names
    printf("Total Sections:                 %u\n",    header->e_shnum);                                     // e_shnum is the total number of entries in the section header table   
    printf("Total Segments:                 %u\n",    header->e_phnum);                    // e_phnum is the total number of entries in the program header table (i.e. total segments)  



    /* navigate to the program header table
     * e_phoff is the byte offset from the start of the file
     * casting map + e_phoff to Elf64_Phdr* gives us the array of segments */
    Elf64_Phdr *program_headers = (Elf64_Phdr *)(map + header->e_phoff);

    printf("\nProgram Headers:\n");

    for (uint32_t i = 0; i < header->e_phnum; i++) {
        printf("\nSegment %u:\n", i);

        printf("Type:                       %s (0x%x)\n", get_segment_type(program_headers[i].p_type), program_headers[i].p_type);
        printf("Offset:                     0x%lx\n", program_headers[i].p_offset);
        printf("Virtual Address:            0x%lx\n", program_headers[i].p_vaddr);
        printf("Physical Address:           0x%lx\n", program_headers[i].p_paddr);
        printf("File Size:                  %lu bytes\n", program_headers[i].p_filesz);
        printf("Memory Size:                %lu bytes\n", program_headers[i].p_memsz);
        printf("Flags (R/W/X):              %s (0x%x)\n", get_segment_flags(program_headers[i].p_flags), program_headers[i].p_flags);
        printf("Alignment:                  %lu\n", program_headers[i].p_align);

    }

    /* navigate to the section header table 
     * e_shoff is the byte offset from the start of the file
     * casting map + e_shoff to Elf64_Shdr* gives us the array of sections */
    Elf64_Shdr *section_headers = (Elf64_Shdr *)(map + header->e_shoff); 

    printf("\nSection Headers:\n");

    /*
     * find the string table using e_shstrndx 
     * this sections's data is a flat array of a null-terminated strings
     * every section's sh_name is an offset into this array
     */

    Elf64_Shdr *strtab= &section_headers[header->e_shstrndx];    // The section header for the string table that contains section names
    char *strdata = (char *)(map + strtab->sh_offset);          // The string data for the section names

    for (uint16_t i = 0; i < header->e_shnum; i++) {
        printf("\nSection %u:\n", i);

        printf("Name:                       %s\n", strdata + section_headers[i].sh_name);                                                 // sh_name is a field in Elf64_Shdr that is an offset into the section header string table, giving the name of the section
        printf("Type:                       %s (0x%x)\n", get_section_type(section_headers[i].sh_type), section_headers[i].sh_type);          // sh_type is a field in Elf64_Shdr that indicates the type of the section (code, data, symbol table, etc)
        printf("Flags:                      %s (0x%lx)\n", get_section_flags(section_headers[i].sh_flags), section_headers[i].sh_flags);      // sh_flags is a field in Elf64_Shdr that contains flags for the section (writable, executable, etc)
        printf("Address:                    0x%lx\n", section_headers[i].sh_addr);                                                            // sh_addr is the virtual address of the section in memory when loaded
        printf("Offset:                     0x%lx\n", section_headers[i].sh_offset);                                          // sh_offset is the byte offset from the start of the file to the section's data    
        printf("Size:                       %lu bytes\n", section_headers[i].sh_size);                                                        // sh_size is the size of the section's data in bytes   
        printf("Link:                       %u\n", section_headers[i].sh_link);                                      // sh_link is a field in Elf64_Shdr that has different meanings depending on the section type (e.g. index of related section)
        printf("Info:                       %lu\n", (unsigned long)section_headers[i].sh_info);                                      // sh_info is a field in Elf64_Shdr that has different meanings depending on the section type (e.g. number of symbols in a symbol table)
        printf("Address Align:              %lu\n", section_headers[i].sh_addralign);                                           // sh_addralign is the required alignment of the section in memory (e.g. 16 for code sections)    
        printf("Entry Size:                 %lu bytes\n", section_headers[i].sh_entsize);                                              // sh_entsize is the size of each entry in the section if it contains fixed-size entries (e.g. symbol table entries)


    }

    /* Find the dynamic symbol table and its associated string table */
    uint16_t dynsym_index = 0;
    for (uint16_t i = 0; i < header->e_shnum; i++) {
        if (section_headers[i].sh_type == SHT_DYNSYM) {
            dynsym_index = i;
            break;
        }
    }

    Elf64_Shdr *dynsym = &section_headers[dynsym_index];
    Elf64_Shdr *dynstr = &section_headers[dynsym->sh_link];         // The linked string table for the dynamic symbols
    char *sym_names = (char *)(map + dynstr->sh_offset);            // The string data for the dynamic symbols

    printf("\n.dynsym found at section index:             %u\n", dynsym_index);
    printf("sh_offset:                                  0x%lx\n", dynsym->sh_offset);
    printf("sh_size:                                    %lu bytes\n", dynsym->sh_size);
    printf("sh_entsize:                                 %lu bytes\n", dynsym->sh_entsize);
    printf("sh_link:                                    %u\n", dynsym->sh_link);



    Elf64_Sym *symbols = (Elf64_Sym *)(map + dynsym->sh_offset); 


    printf("\nSymbol Table (.dynsym)  — %lu symbols:\n\n", dynsym->sh_size / dynsym->sh_entsize);  // Tells us how many symbols there are by dividing the total size of the .dynsym section by the size of each symbol entry

    for (uint64_t i = 0; i < dynsym->sh_size / dynsym->sh_entsize; i++) {
        /* st_name is an offset into sym names (the .dynstr section)
         * same mechanic as sh_name into shstrtab */

        char *name = sym_names + symbols[i].st_name;  // Get the symbol name from the string table using st_name as an offset


        printf("[%3lu]  0x%016lx  %-8s  %-7s  %s\n", i, symbols[i].st_value, get_symbol_binding(symbols[i].st_info), get_symbol_type(symbols[i].st_info), name);

    }
    

    



    munmap(map, file_info.st_size);



    
    return EXIT_SUCCESS;
}