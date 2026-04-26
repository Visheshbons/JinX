# JinX Assembly (JASM) Documentation

JASM is the planned low-level assembly language target for the JinX toolchain.

At the current stage of the repository, JASM is a **design document / planned target**. The JC compiler backend (`generator.py`) is still unfinished, so this document defines the intended direction and conventions for implementation.

---

## 1) Purpose of JASM

JASM is intended to:
- Serve as the intermediate/target representation produced from JC.
- Stay human-readable for debugging early boot/kernel development.
- Map cleanly onto machine code in later compiler stages.

Typical future flow:

```text
JC source (.jc) -> JC parser/AST -> JASM (.jasm) -> machine code / binary image
```

---

## 2) File Extension

Recommended extension:

- `.jasm`

Example: `kernel.jasm`

---

## 3) Planned Core Structure

A JASM file should be structured as:
1. Optional metadata/directives.
2. Section declarations.
3. Labels/functions.
4. Instructions.
5. Data declarations.

Illustrative skeleton:

```asm
.section text
.global k_main

k_main:
    mov r0, 0xB8000
    ret
```

---

## 4) Lexical Conventions (Proposed)

### Comments
Single-line comments:

```asm
; this is a comment
```

### Labels
A label defines an addressable symbol:

```asm
k_main:
```

### Instruction format

```asm
<mnemonic> <operand1>, <operand2>
```

Examples:

```asm
mov r0, 1
add r0, r1
jmp loop_start
```

---

## 5) Planned Directives

Common directives to support first:

- `.section <name>` - switch section (`text`, `data`, `rodata`, etc.).
- `.global <symbol>` - export a symbol.
- `.extern <symbol>` - import external symbol.
- `.byte`, `.word`, `.dword`, `.qword` - data constants.
- `.ascii` / `.asciz` - string data.

Example:

```asm
.section rodata
msg:
    .asciz "Hello"
```

---

## 6) Registers and ABI (Draft)

The final register set and ABI are not yet fixed. Until defined, use placeholder register names in examples:

- `r0`, `r1`, `r2`, ...
- `sp` (stack pointer), `bp` (base pointer), `pc` (program counter), if applicable.

Planned ABI topics (to finalize later):
- Argument passing strategy.
- Return value register(s).
- Caller/callee-saved register rules.
- Stack alignment.

---

## 7) Instruction Classes (Planned)

### Data movement
- `mov dst, src`
- `load dst, [addr]`
- `store [addr], src`

### Arithmetic/logic
- `add`, `sub`, `mul`, `div`
- `and`, `or`, `xor`, `not`
- `cmp`

### Control flow
- `jmp label`
- `jz label`, `jnz label`, etc.
- `call label`
- `ret`

### Stack
- `push reg`
- `pop reg`

Mnemonic set may change when architecture decisions are finalized.

---

## 8) Mapping JC -> JASM (Planned)

### Function declaration
JC:

```c
func k_main() u32 {
  u32 screen = 0xB8000;
}
```

Possible JASM shape:

```asm
.section text
.global k_main
k_main:
    mov r0, 0xB8000
    ; local "screen" assigned from r0 / stack slot (backend-defined)
    ret
```

### Variable declarations
A JC local declaration with constant initialization should lower into either:
- A register assignment.
- A stack slot write.

Choice depends on register allocation and function complexity.

---

## 9) Error Handling Expectations

Future JASM assembler/compiler stages should report:
- Unknown mnemonics.
- Invalid operand counts/types.
- Undefined labels/symbols.
- Section/directive misuse.
- Numeric literal overflow for operand width.

---

## 10) Current Reality in This Repository

- JASM codegen is not implemented yet.
- `OS/compiler/generator.py` is currently empty.
- `OS/compiler/main.py` currently stops after lexing/parsing and reports success.

This document therefore defines the initial contract the backend can implement next.

---

## 11) Recommended Next Implementation Steps

1. Define a small architecture-neutral instruction IR.
2. Implement `generator.py` to lower parsed JC declarations/functions into preliminary JASM.
3. Emit `.jasm` files from `main.py` as an output artifact.
4. Add round-trip tests with tiny JC programs (`main.jc`, `stdio.jc`).
5. Freeze v0.1 JASM syntax and update this document accordingly.
