# PLANNING
This file will contain the rough outline for how the OS will be built.

Current TREE:
```
.
├── Documentation
│   ├── AI
│   │   ├── JASM_Documentation.md
│   │   └── JinX-C_Documentation.md
│   ├── JASM_Documentation_Original.md
│   └── JinX-C_Docs.md
├── JinX_Virt
│   ├── Core.cpp
│   ├── hello_world.ja
│   ├── JA2.cpp
│   ├── JA.cpp
│   ├── JASM_VM
│   ├── JinXVirtual.cpp
│   ├── JinXVirtual.h
│   ├── keys.ja
│   ├── Lexer.hpp
│   └── main.jc
├── OS
│   ├── bin
│   │   ├── build-scripts
│   │   ├── compiler
│   │   │   ├── generator.cpython-314.pyc
│   │   │   ├── lexer.cpython-314.pyc
│   │   │   └── parser.cpython-314.pyc
│   │   ├── kernel
│   │   └── lib
│   ├── build_scripts
│   ├── compiler
│   │   ├── generator.py
│   │   ├── jc2jasm.jasm
│   │   ├── jc2jasm.jc
│   │   ├── lexer.py
│   │   ├── main.py
│   │   ├── parser.py
│   │   └── __pycache__
│   │       ├── generator.cpython-314.pyc
│   │       ├── lexer.cpython-314.pyc
│   │       └── parser.cpython-314.pyc
│   ├── kernel
│   │   ├── boot.jasm
│   │   ├── boot.jc
│   │   ├── drivers
│   │   ├── main.jasm
│   │   └── main.jc
│   └── lib
│       ├── stdio.jasm
│       └── stdio.jc
├── plan.md
└── README.md

16 directories, 34 files
```

## STAGES

### Stage 0
Since everything is going to be custom, we need our own programming language.
However, an OS cannot be written in a language without a compiler or an assembler.
We need to write a basic compiler for the root language using an existing language (like C or Python), using an existing OS (like Linux or Windows).
Then, we can use that "cross-compiler" to compile our kernel.
Once the OS is running, we rewrite the compiler in the cusom language so it can compile itself in the OS.

#### Progress
We have added `main.py`, `lexer.py`, and a very basic `main.jc`.
To run the compiler for `main.jc`, run:
```bash
vishesh@vishesh-fedora:~/[PATH_TO_PROJECT_ROOT]$ python OS/compiler/main.py OS/kernel/main.jc
```

### Stage 1
We need to communicate to the BIOS (or UEFI) with a bootloader.
However, we need either x86_64 or ARM.
Further research is required.

## CORE COMPONENTS (Priority build)

### Memory Management
We need to write a physical and a virtual memory manager.
* The physical memory manager tracks which pages of RAM are used.
* The virtual memory manager maps "fake" addresses to real ones so programs don't crash into each other.

### The Kernel
This can be either Monolithic or Microkernel style.
* A monolithic kernel has everything (drivers, file system) runs in the kernel space. This is fast, but not very stable.
* A microkernel has only the essentials run in the kernel, everything else runs as seperate "servers". This is much more complex, but is very stable.

### Interupts and Drivers
We need to write our own custom drivers for this project. This most likely will take the form of an **Interupt Descriptor Table (IDT)**. At the most basic level, we need a driver for the VGA/GOP (for display) and a keyboard controller.

### File System
We need to write own own file system format, or use FAT32. Either works, but using FAT32 will make development easier.
