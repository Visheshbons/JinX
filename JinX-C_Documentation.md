# JinX-C (JC) Documentation

JinX-C (JC) is the custom systems language for JinX. It is intentionally C-like, but the current compiler only supports a small teaching subset while the project is in early development.

## Current Status

The current Python compiler implements:
- Lexing/tokenization.
- Parsing for `#include` directives.
- Parsing for function declarations with optional typed arguments and an optional return type.
- Parsing for simple variable declarations with direct initialization.

It does **not** yet implement full code generation, full expression parsing, or full control-flow semantics.

---

## 1) File Structure and Entry Point

JC source files use the `.jc` extension.

Example:

```c
#include <stdio.h>

func main() u32 {
  u32 code = 0;
}
```

Notes:
- `func` declares a function.
- A return type can appear after the parameter list (for example: `u32`).
- The parser currently focuses on declarations and structure, not full execution semantics.

---

## 2) Comments

Single-line comments are supported:

```c
// This is a comment
```

Block comments (`/* ... */`) are not currently supported.

---

## 3) Supported Primitive Types

The lexer/parser currently recognize these keywords as language types:

- `u8` - 8-bit unsigned integer.
- `u32` - 32-bit unsigned integer.
- `ptr` - raw pointer value.
- `str` - string type.
- `bool` - boolean keyword (recognized by lexer; parser support is still partial in declarations).

---

## 4) Includes

The lexer recognizes include directives in this format:

```c
#include <stdio.h>
```

Currently, includes are parsed into the AST as top-level include nodes.

---

## 5) Functions

### Syntax

```c
func <name>(<typed_args>) <return_type> {
  // body
}
```

Examples:

```c
func k_main() u32 {
  u32 screen = 0xB8000;
}
```

```c
func putchar(u8 c) {
  ptr vga = 0xB8000;
}
```

### Parameters

Parameters are typed and follow a C-like style:

```c
func add(u32 a, u32 b) u32 {
  u32 sum = a;
}
```

The parser is designed to support comma-separated arguments.

---

## 6) Variable Declarations

Inside functions, the parser currently supports declarations with immediate initialization:

```c
u32 count = 10;
str text = "Hello";
ptr vga = 0xB8000;
```

Supported initializer token categories are currently:
- Number literals (including hex like `0xB8000`).
- String literals.
- Identifiers.

---

## 7) Literals and Tokens

### Numeric literals
- Decimal: `123`
- Hexadecimal: `0xB8000`

### String literals
- Double-quoted only:

```c
"Hello world"
```

### Operators (tokenized)
The lexer currently tokenizes single-character operators such as:
- `=` `+` `-` `*` `/`

Multi-character comparison operators like `==`, `!=`, `<=`, and `>=` are not currently tokenized as single operators by the current compiler implementation.
> Tokenization support does not automatically mean full semantic/codegen support yet.

---

## 8) Punctuation and Statement Rules

Recognized punctuation tokens include:
- `;` `(` `)` `{` `}` `[` `]`

Use semicolons for declarations/statements:

```c
u32 x = 1;
```

---

## 9) Minimal Working Example

```c
#include <stdio.h>

func main() u32 {
  u32 result = 0;
}
```

Run compiler (from repository root):

```bash
python OS/compiler/main.py OS/kernel/main.jc
```

The current compiler prints:
1. Tokens.
2. Parsed AST.
3. A success message if lexing/parsing complete.

---

## 10) Known Limitations (Current Compiler)

- No finished machine-code/JASM generation stage yet.
- Parser handles only a subset of statements (currently strongest support is declaration parsing).
- Control flow (`if`, `while`) is tokenized but not fully parsed/executed yet.
- Return statements are tokenized but not fully lowered to backend output yet.

---

## 11) Planned Next Steps

- Expand parser coverage for expressions and statements.
- Implement backend code generation.
- Add a documented standard library contract for built-ins like `println` and `putchar`.
- Align JC documentation with JASM output format as backend stabilizes.
