#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>


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

    /* e_ident[EI_CLASS] tells us whether this is a 32 or 64 bit binary
     * ELFCLASS64 — 64-bit, ELFCLASS32 — 32-bit */
    if (header->e_ident[EI_CLASS] == ELFCLASS64) {
        printf("Architectural Class: 64-bit\n");
    } else if (header->e_ident[EI_CLASS] == ELFCLASS32) {
        printf("Architectural Class: 32-bit\n");
    }


    /* e_ident[EI_DATA] tells us the byte order of the binary
     * ELFDATA2LSB — little endian (x86-64 is little endian)
     * ELFDATA2MSB — big endian */
    if (header->e_ident[EI_DATA] == ELFDATA2LSB) {
        printf("Data Encoding: Little Endian\n");
    } else if (header->e_ident[EI_DATA] == ELFDATA2MSB) {
        printf("Data Encoding: Big Endian\n");
    }



    /* e_type tells us what kind of ELF file this is
     * ET_EXEC — standalone executable
     * ET_DYN  — shared object or position independent executable (PIE)
     * ET_REL  — relocatable object file (.o file)
     * ET_CORE — core dump */

    uint64_t type = header->e_type;

    switch (type) {
        case ET_EXEC:
            printf("File Type: Executable\n");
            break;
        case ET_DYN:
            printf("File Type: Shared Object\n");
            break;
        case ET_REL:
            printf("File Type: Relocatable\n");
            break;
        case ET_CORE:
            printf("File Type: Core Dump\n");
            break;    
        default:
            printf("File Type: Unknown\n");
    }

     /* e_entry — the virtual address where execution begins
     * this is where the kernel jumps after loading the binary */
    printf("Entry Point (Virtual Address): 0x%lx\n",
           (unsigned long)header->e_entry);

    /* e_shnum — total number of section headers in this binary */
    uint32_t total_sections = header->e_shnum;
    printf("Total Sections: %u\n", total_sections);

    /* e_phnum — total number of program headers (segments) in this binary */
    uint32_t total_segments = header->e_phnum;
    printf("Total Segments: %u\n", total_segments);

    /* navigate to the program header table
     * e_phoff is the byte offset from the start of the file
     * casting map + e_phoff to Elf64_Phdr* gives us the array of segments */
    Elf64_Phdr *program_headers = (Elf64_Phdr *)(map + header->e_phoff);

    printf("\nProgram Headers:\n");
    for (uint32_t i = 0; i < total_segments; i++) {
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