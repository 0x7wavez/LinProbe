#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>


const char *get_elf_class(uint8_t class) {
    switch (class) {
        case ELFCLASS32: return "32-bit";
        case ELFCLASS64: return "64-bit";
        default: return "Unknown";
    }
}

const char *get_elf_data_encoding(uint8_t data) {
    switch (data) {
        case ELFDATA2LSB: return "Little Endian";
        case ELFDATA2MSB: return "Big Endian";
        default: return "Unknown";
    }
}

const char *get_elf_version(uint8_t version) {
    switch (version) {
        case EV_NONE: return "Invalid";
        case EV_CURRENT: return "Current";
        default: return "Unknown";
    }
}

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
        default: return "UNKNOWN";
    }
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
        printf("Segment %u:\n", i);

        /* p_type — what kind of segment this is (LOAD, DYNAMIC, etc)
         * get_segment_type converts the raw integer to a readable string */
        printf("  Type:             %s\n",
               get_segment_type(program_headers[i].p_type));

        /* p_offset — byte offset of this segment from start of file */
        printf("  Offset:           0x%lx\n", program_headers[i].p_offset);

        /* p_vaddr — virtual address where this segment is loaded in memory */
        printf("  Virtual Address:  0x%lx\n", program_headers[i].p_vaddr);

        /* p_paddr — physical address, relevant on embedded systems
         * on Linux this is usually the same as p_vaddr */
        printf("  Physical Address: 0x%lx\n", program_headers[i].p_paddr);

        /* p_filesz — size of the segment in the file
         * p_memsz  — size of the segment in memory
         * p_memsz can be larger than p_filesz — the difference is
         * zero-initialized at load time (this is how .bss works) */
        printf("  File Size:        %lu bytes\n", program_headers[i].p_filesz);
        printf("  Memory Size:      %lu bytes\n", program_headers[i].p_memsz);

        /* p_flags — permissions for this segment
         * PF_R=4 (read), PF_W=2 (write), PF_X=1 (execute)
         * a LOAD segment with flags 0x5 is readable and executable (.text)
         * a LOAD segment with flags 0x6 is readable and writable (.data) */
        printf("  Flags:            0x%x\n", program_headers[i].p_flags);

        /* p_align — memory alignment requirement for this segment
         * PT_LOAD segments are typically aligned to 0x200000 (2MB) */
        printf("  Alignment:        %lu\n", program_headers[i].p_align);

    }

    munmap(map, file_info.st_size);



    
    return EXIT_SUCCESS;
}