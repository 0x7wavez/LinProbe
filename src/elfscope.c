#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    struct stat file_info;
    if (fstat(fd, &file_info) < 0) {
        perror("Error getting file size");
        close(fd);
        return EXIT_FAILURE;
    }

    char *map = mmap(NULL, file_info.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    Elf64_Ehdr *header = (Elf64_Ehdr *)map;
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Error: Not a valid ELF file\n");
        munmap(map, file_info.st_size);
        return EXIT_FAILURE;
    }
        printf("Valid ELF file: %s\n", argv[1]);

    if (header->e_ident[EI_CLASS] == ELFCLASS64) {
        printf("Architectural Class: 64-bit\n");
    } else if (header->e_ident[EI_CLASS] == ELFCLASS32) {
        printf("Architectural Class: 32-bit\n");
    }

    if (header->e_ident[EI_DATA] == ELFDATA2LSB) {
        printf("Data Encoding: Little Endian\n");
    } else if (header->e_ident[EI_DATA] == ELFDATA2MSB) {
        printf("Data Encoding: Big Endian\n");
    }


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

    unsigned long entry_point = header->e_entry;
    printf("Entry Point (Virtual Address): 0x%lx\n", entry_point);

    uint32_t total_sections = header->e_shnum;
    printf("Total Sections: %u\n", total_sections);

    uint32_t total_segments = header->e_phnum;
    printf("Total Segments: %u\n", total_segments);

    munmap(map, file_info.st_size);

    
    return EXIT_SUCCESS;
}