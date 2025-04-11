# 🧠 ELF Binary Loader in C

This is a custom ELF binary loader written in C. It reads a 32-bit ELF executable file, loads it into memory using `mmap()`, and jumps to its entry point to run it — mimicking how a basic OS loader works.

---

## ✨ Features

| Feature               | Description                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| ELF32 validation        | Verifies the file is a 32-bit executable ELF                               |
| Program header parsing  | Reads all segments and identifies executable segments                      |
| Virtual memory mapping  | Allocates memory using `mmap()` for the relevant segment                   |
| Manual execution        | Jumps to `_start` in the ELF executable and prints its return value        |
| Safe cleanup            | Cleans up file descriptors, allocated memory, and mapped memory properly   |

---

## 🛠️ How It Works

1. Reads the ELF header to ensure it's a valid 32-bit executable.
2. Loads all program headers and identifies the executable loadable segment.
3. Allocates memory using `mmap()` with `PROT_READ | PROT_WRITE | PROT_EXEC`.
4. Copies segment data into memory and calculates the offset to the entry point.
5. Casts the entry point to a function pointer and invokes `_start()`.
6. Outputs the return value of `_start`.

---

## 📦 Build & Run

### Compile:
```bash
gcc -o elf_loader elf_loader.c
