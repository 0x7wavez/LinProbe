# LinProbe

A lightweight, modular Linux process introspection and
binary analysis engine written in pure C.

No library wrappers. No dependencies. One binary.
Designed to run in constrained environments where
tools like strace, GDB, or Frida cannot be installed.

---

## Why LinProbe?

Most Linux security and analysis tools are either too
heavy, require a runtime, or do only one thing.

LinProbe is being built to do what currently requires
four separate tools — in a single dependency-free
binary that runs anywhere Linux runs.

Target environments: IoT devices, minimal containers,
stripped-down servers, embedded Linux systems.

---

## Modules

| Module   | Description                     | Status         |
|----------|---------------------------------|----------------|
| elfscope | ELF binary parser (no libelf)   | 🔨 in progress |
| tracer   | ptrace syscall interceptor      | ⬜ upcoming    |
| memmap   | process memory region dumper    | ⬜ upcoming    |
| rawnet   | raw socket network probe        | ⬜ upcoming    |

---

## elfscope

Parses any ELF64 binary from disk without libelf.
Raw mmap and struct casting only.

```bash
gcc -Wall -Wextra -o elfscope src/elfscope.c
./elfscope /bin/ls
```

Output includes:
- Full ELF header — class, encoding, type, machine,
  entry point, all offsets and sizes
- Program headers with segment types and permissions
- Section headers with resolved names via string table
- Dynamic symbol table with binding and type

Sessions completed:
- Session 1: ELF magic byte validation
- Session 2: Full ELF header parsing
- Session 3: Program headers
- Session 4: Section headers with string table resolution
- Session 5: Dynamic symbol table

Sessions remaining:
- Session 6: Dynamic section and GOT
- Session 7: Edge cases and stress testing

---

## Build

```bash
git clone https://github.com/0x7wavez/linprobe
cd linprobe/src
gcc -Wall -Wextra -o elfscope elfscope.c
./elfscope /bin/ls
```

---

## Status

Active development.

---

## Author

wavez — Linux systems programmer
X: @0x7wavez
Linkedin: Nwadi Favour Chukwuka
