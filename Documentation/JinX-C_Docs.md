# JinX-C Docs `(.jc)`
JinX-C (JC) is the custom systems language for JinX. It is intentionally C-like, but has some fundamental differences.
This documentation will use code blocks marked with `c`, `cpp`, or `csharp`. This is purely for syntax highlighting, and should be assumed as JC code.

---

## File Structure
JC files use the `.jc` file extention.

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    println("Hello, world!");
    return 0;
}
```

Notes:
- Our `#include` statement is including a JC header, not a C header.
- The `main` function is declared with a return type of `u32`, which is a JC-specific variable. We will cover this further in the docs.
- We are using the `println` function, which is native to JC via `<stdio.jc.h>`. Note that `printf()` is also supported.


---

## Variables

### Variable Types
JC's variable types are slightly different from standard C or C++. The current version of JC supports the following:

- `u8` - An 8-bit unsigned integer. This is rarely used, except in more optimised kernel code. This can be used in the place of other integer types, but is not reccomended for general use. In kernel development, if there will never be a value above 255, then `u8` should be used.

- `u32` - A 32-bit unsigned integer. This is the most commonly used integer type in JC, and is the default for integer literals. For example, `u32 x = 5;` is valid, but `u8 x = 5;` is also valid. However, `u32 x = 5;` is more common and reccomended for general use.

- `u64`, `u128` - Larger unsigned integer types. These are used for more specific use cases where larger numbers are required.

- `ptr` - A raw pointer value. This is used for low-level memory management and interfacing with hardware. It is not type-safe, and should be used with caution.

- `str` - A string type. This is compiled down to an array of `u8` values in memory.

- `u1` or `bool` - A simple boolean. It can be defined interchangeably between the two declarations. This is a single bit value.

- `s8` - A signed 8-bit integer. This is equivalent to `int8_t` in C. It can hold values from -128 to 127.

- `s32` - A signed 32-bit integer. This is equivalent to `int32_t` in C. It can hold values from -2,147,483,648 to 2,147,483,647.

- `s64`, `s128` - Larger signed integer types. These are used for more specific use cases where larger numbers are required.

### Variable Declarations
Variable declarations in JC are very similar to C and C++.

```c++ (jc)
#include <stdio.jc.h>

u32 main() {
    u32 x = 5;
    u8 y = 255;
    str name = "JinX";
    ptr rawPointer = 0xDEADBEEF;
    bool isTrue = true;
    u1 isFalse = 0;

    return 0;
}
```

Most of this can be derived by intuition if you are familiar with almost any other programming language.
However, some important notes include:
- The `str` type is a string, which is compiled down to an array of `u8` values in memory. String literals can be assigned directly to `str` variables.
- When using `bool`, you need to use the `true` and `false` keywords. However, when using the `u1` type, you must declare it as `1` or `0`. This is so it is easier to build the compiler (lazy devs hehe), and also to make it easier for the use of the variable. When reassigning the variable, you can interchange between `true`/`false` and `1`/`0`.

### Constants
JC also supports constants, which are declared as such:

```cpp (jc)
#include <stdio.jc.h>

const u32 MAX = 100; // Stored in memory
const hard u32 MAX_2 = 200; // Stored in SSD
```
Here, constants can be `soft` (default) or `hard`. Soft constants are memory-based, and `hard` ones are in SSD storage.

### Variable Deletion
JC is unique in it supporting variable deletion while the program is still running. This uses the `del` keyword, and can be used with the `force` modifier to force deletion of variables still in use, and to delete constants.

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    u32 x = 5;
    println(x); // 5

    del x;
    println(x); // Causes an error

    const hard u128 VISHESH_AURA = u128.MAX;
    println("Vishesh's aura is: " + VISHESH_AURA);
    // Vishesh's aura is: 340282366920938463463374607431768211455")

    del force VISHESH_AURA;
    println("Vishesh's aura is: " + VISHESH_AURA);
    // Causes an error

    u32 y = 10;
    while (y > 0) {
        println(y);
        if (y == 5) {
            // del y will not run, and will cause a passive error (no crash)
            // del force y will run tho
            
            del force y; // Force delete y while it is still in use
        }
        y = y - 1;
    }
    // Will crash when y is deleted

    return 0;
}
```
This is an incredibly powerful feature, and should be used with extreme caution. But who doesn't love a bit of chaos? \*cough\* anyways \*cough\*

---

## Functions
Functions in JC are declared using their return type. `void` can be used here.

```cpp (jc)
#include <stdio.jc.h>

u32 exampleFunction(u32 x, u32 y) {
    return x + y;
}

u32 main() {
    println("Hello world!");
    u32 result = exampleFunction(5, 10);
    println("5 + 10 is: " + result);
    return 0;
}
```