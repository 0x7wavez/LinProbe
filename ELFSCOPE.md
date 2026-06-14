# elfscope — Code Guide

This document explains what elfscope does, how it works,
and why every major piece of code exists.

Read this if the code feels like jargon. By the end you
should be able to look at any line in elfscope.c and
explain what it does and why it is there.

---

## The one-sentence version

elfscope opens a Linux binary file, maps it into memory,
and reads its internal structure out loud.

---

## What is an ELF file?

Every program on your Linux system is an ELF file.
/bin/ls, /bin/bash, /usr/bin/python3 — all ELF files.

ELF stands for Executable and Linkable Format. It is just
a file format — a specific arrangement of bytes that
describes a program. Like how a .jpg has a specific
structure for image data, an ELF file has a specific
structure for program data.

That structure has four parts:

```
┌─────────────────────────────┐
│        ELF Header           │  ← What kind of file is this?
├─────────────────────────────┤
│      Program Headers        │  ← How should the kernel load it?
├─────────────────────────────┤
│      Section Headers        │  ← What named regions are inside?
├─────────────────────────────┤
│      Symbol Table           │  ← What functions does it import?
└─────────────────────────────┘
```

elfscope reads all four parts and prints them.

---

## How elfscope reads the file

Most programs read files like this:
- open the file
- read a chunk of bytes into a buffer
- process the buffer
- read the next chunk
- repeat

elfscope does something different. It uses a system call
called mmap() to map the entire file directly into memory.

After mmap(), the file is just a block of bytes sitting at
a memory address. We store that address in a variable called
`map`. Then we read the file by casting C structs onto `map`.

What does "casting a struct onto memory" mean?

```c
char *map = mmap(...);          // map is the address of the file in memory
Elf64_Ehdr *header = (Elf64_Ehdr *)map;  // tell C: treat these bytes as an ELF header struct
printf("%lx\n", header->e_entry);        // read the entry point field directly
```

The bytes did not move. We just told C how to interpret them.
This is the core mechanic of the entire project.

---

## Part 1 — The ELF Header

The ELF header is always the first 64 bytes of the file.
The struct Elf64_Ehdr describes those 64 bytes exactly.

After the cast, every field in the struct is directly readable:

```
header->e_ident   — 16 bytes of identity information
header->e_type    — what kind of file (executable, shared library)
header->e_machine — what CPU (x86-64, ARM, etc.)
header->e_entry   — the memory address where execution starts
header->e_phoff   — byte offset to the program header table
header->e_shoff   — byte offset to the section header table
header->e_phnum   — how many program headers exist
header->e_shnum   — how many section headers exist
header->e_shstrndx — index of the section that holds section names
```

The first thing we check is the magic bytes.
Every valid ELF file starts with these 4 bytes: 0x7f 'E' 'L' 'F'
If they are not there, the file is not an ELF file and we stop.

---

## Part 2 — Program Headers (Segments)

Program headers are written for the Linux kernel.
They tell the kernel: "here is how to load this program into memory."

Each program header describes one segment — a chunk of the file
that needs to be placed at a specific memory address with specific
permissions.

The most important segment type is PT_LOAD.
There are usually 2 to 4 PT_LOAD segments in a binary:

```
Segment with flags RE (read + execute) → the code (.text)
Segment with flags R  (read only)      → read-only data (.rodata)
Segment with flags RW (read + write)   → writable data (.data, .bss)
```

Key fields per program header:
```
p_type    — what kind of segment (LOAD, DYNAMIC, INTERP, etc.)
p_offset  — where this segment starts in the file
p_vaddr   — the memory address where this segment will be placed
p_filesz  — how many bytes this segment takes in the file
p_memsz   — how many bytes this segment takes in memory
            (can be larger than p_filesz — the extra bytes are zeroed)
p_flags   — read/write/execute permissions
```

To get to the program header table, we use e_phoff:

```c
Elf64_Phdr *program_headers = (Elf64_Phdr *)(map + header->e_phoff);
```

map + e_phoff moves forward e_phoff bytes from the start of the file.
Casting to Elf64_Phdr* lets us index through the array: program_headers[0], [1], [2], etc.

---

## Part 3 — Section Headers

Section headers are written for tools — debuggers, linkers,
and programs like elfscope. They describe named regions inside
the binary.

Common sections:
```
.text       — the actual machine code instructions
.data       — global variables that have initial values
.bss        — global variables with no initial value (zero at startup)
.rodata     — read-only strings and constants
.got        — Global Offset Table (a table of function addresses)
.plt        — Procedure Linkage Table (stub code for imported functions)
.dynsym     — dynamic symbol table (list of imported functions)
.dynstr     — string table for dynamic symbol names
.shstrtab   — string table for section names
```

The puzzle with section headers:
Section names are not stored directly in the section header struct.
Only a number (sh_name) is stored — it is a byte offset into a
special section called .shstrtab.

.shstrtab is just a flat block of null-terminated strings:
```
\0.text\0.data\0.bss\0.got.plt\0
 0  1      7     12   16
```

Section 13 might have sh_name = 1, meaning its name starts
at byte 1 of .shstrtab, which gives us ".text".

The ELF header field e_shstrndx tells us which section IS
the string table. We find it, get its data, then use each
section's sh_name as an offset to get the actual name.

```c
Elf64_Shdr *strtab  = &section_headers[header->e_shstrndx];
char       *strdata = (char *)(map + strtab->sh_offset);
// to get a section name:
char *name = strdata + section_headers[i].sh_name;
```

---

## Part 4 — Dynamic Symbol Table

The dynamic symbol table (.dynsym) lists every external function
and variable this binary needs from shared libraries at runtime.

When you see printf, malloc, fopen in the output — those are
functions from libc that this binary calls but does not contain.
The dynamic linker loads libc and fills in their real addresses
via the GOT when the program starts.

Each entry in .dynsym is an Elf64_Sym struct with these fields:
```
st_name   — byte offset of the symbol name in .dynstr
st_info   — packs binding and type into one byte
              upper 4 bits = binding (LOCAL / GLOBAL / WEAK)
              lower 4 bits = type    (FUNC / OBJECT / NOTYPE)
st_value  — the symbol's address (0 for external imports)
st_size   — size of the symbol's data
```

The connection between .dynsym and .dynstr:
The section header of .dynsym has a field called sh_link.
For symbol table sections, sh_link holds the index of the
section containing the name strings. That section is .dynstr.

```c
Elf64_Shdr *dynsym  = &section_headers[dynsym_index];
Elf64_Shdr *dynstr  = &section_headers[dynsym->sh_link];
char *sym_names     = (char *)(map + dynstr->sh_offset);
// to get a symbol name:
char *name = sym_names + symbols[i].st_name;
```

This is the exact same mechanic as section name resolution —
just pointed at a different string table.

---

## The helper functions

Every helper function in elfscope follows the same pattern:

```c
const char *get_something(uint??_t raw_value) {
    switch (raw_value) {
        case CONSTANT_A: return "human readable name";
        case CONSTANT_B: return "human readable name";
        default:         return "Unknown";
    }
}
```

ELF fields are stored as integers. The helper functions convert
those integers to strings so the output is readable.

Example: e_type stores the number 3 for a shared object file.
get_elf_type(3) returns "Shared object file".

---

## The pointer chain — how every navigation works

Every piece of data in an ELF file is reached by the same pattern:

```
map + some_offset → cast to the right struct type → read fields
```

Examples:
```c
// ELF header — at byte 0
Elf64_Ehdr *header = (Elf64_Ehdr *)map;

// program header table — at byte e_phoff
Elf64_Phdr *phdrs = (Elf64_Phdr *)(map + header->e_phoff);

// section header table — at byte e_shoff
Elf64_Shdr *shdrs = (Elf64_Shdr *)(map + header->e_shoff);

// string table data — at byte sh_offset of the strtab section
char *strdata = (char *)(map + strtab->sh_offset);

// symbol array — at byte sh_offset of the dynsym section
Elf64_Sym *symbols = (Elf64_Sym *)(map + dynsym->sh_offset);
```

If you understand this pattern, you understand elfscope.

---

## Sessions

| Session | What was built                              |
|---------|---------------------------------------------|
| 1       | Open file, verify ELF magic bytes           |
| 2       | Parse and print the full ELF header         |
| 3       | Parse and print all program headers         |
| 4       | Parse and print all section headers         |
|         | including string table name resolution      |
| 5       | Parse and print the dynamic symbol table    |

---

## Part of

LinProbe — a modular Linux process introspection engine
built from scratch in pure C.

github.com/0x7wavez/linprobe
