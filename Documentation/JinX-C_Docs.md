# JinX-C Docs `(.jc)`
JinX-C (JC) is the custom systems language for JinX. It is intentionally C-like, but has some fundamental differences.

> [!NOTE]
>  This documentation will use code blocks marked with `c`, `cpp`, or `csharp`. This is purely for syntax highlighting, and should be assumed as JC code.

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

- Declaring variables to hold other types are shown below, in the section about 

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
JC is unique in it supporting variable deletion while the program is still running.
This uses the `del` keyword, and can be used with the `force` modifier to force deletion of variables still in use, and to delete constants.

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

> [!WARNING]
>  This is an incredibly powerful feature, and should be used with extreme caution. But who doesn't love a bit of chaos? \*cough\* anyways \*cough\*

### Arrays and Objects
JC has arrays and objects, but they are rather unique in their implementation.
They can be declared with either the `trad` (default) or `js` tag.

#### Traditional Arrays
JC arrays are built to be like regular C arrays, and therefore have similar syntax.

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    tradArr u32 myArr[5] = {1, 2, 3, 4, 5};
    println(myArr[0]); // 1
    println(myArr[4]); // 5

    return 0;
}
```
So just C arrays but rebranded.

#### Traditional Objects
Similar to C++, JC uses `struct Point` for objects.

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    tradObj struct Point {
        u32 x;
        u32 y;
    }

    Point p = {10, 20};
    println("Point p is at: (" + p.x + ", " + p.y + ")"); // Point p is at: (10, 20)

    return 0;
}
```
This is literally C++ arrays

#### JS Arrays
JC also supports JavaScript-style arrays, which are more dynamic and easier to use.
Also gigachad

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    jsarr u32 myArr = [1, 2, 3, 4, 5];
    println(myArr[0]); // 1
    println(myArr[4]); // 5

    return 0;
}
```

#### JS Objects
This is the greatest form of object to ever exist (in @Visheshbons's opinion)
If you thing otherwise, you're wrong

```cpp (jc)
#include <stdio.jc.h>

u32 main() {
    jsobj myObj = {
        name: str "JinX",
        version:  str "0.6.0",
        features: str ["Slow", "Insecure", "Impossible to Use"],
        languages: {
            JinX-C: {
                version: str "0.2.4",
                status: str "In Development",
                good: bool false
            },
            JinX-ASM: {
                version: str "0.1.0",
                status: str "In Development",
                good: bool false
            }
        }
    }

    println(myObj.name); // JinX
    println(myObj.version); // 0.6.0
    println(myObj.features[0]); // Slow
    println(myObj.languages.JinX-C.version); // 0.2.
    println(myObj.languages[0].version); // 0.2.
    println(myObj[3][1][2]); // false
    println(myObj.languages["JinX-C"].version); // 0.2.

    return 0;
}
```
As you can see, JavaScript-style objects are true gigachad.

> [!IMPORTANT]
>  JS objects are gigachad

Details and explanations:
- JS arrays and objects are more dynamic than traditional ones
- They can be resized and modified at runtime, which is not possible with traditional arrays and objects.
- Shown by `println(myObj.name);` and `println(myObj.version);`, we can access object properties using dot notation.
- Shown by `println(myObj.features[0]);`, we can access array elements using bracket notation.
- Shown by `println(myObj.languages[0].version); // 0.2.`, we can mix and match.
- Shown by `println(myObj[3][1][2]); // false`, we can also access properties using bracket notation with string keys, which is useful for dynamic property access.
- Shown by `println(myObj.languages["JinX-C"].version); // 0.2.`, we can also use bracket notation with string keys to access properties, which is useful for dynamic property access.
- JS objects are gigachad

#### JS Classes
For class declarations, JC has a more deviant syntax from JS to hold true with other JC syntaxing.

```cpp
class Car {
    constructor ( u32 x, u32 y, u32 size = 15 ) {
        u32 this.x = x;
        u32 this.y = y;
        u32 this.size = size;

        u32 this.speed = 0;
        u32 this.friction = 0.05;

        bool this.useBrain = (controlType == 'AI');
    };
```
The `constructor` is almost identical to JS, but has type declarations in it. In the contructor params itself, defaults can be set, as shown by the variable `size`.
```cpp
    global void accelerate() {
        this.speed += 1;
    }
```
The `global` tag means it can be called from outside the class. 
```cpp
    local void airResistance() {
        this.speed -= this.friction;
    }
```
The `local` tag means it can only be called from other *internal* functions.
```cpp
    global u32 update() {
        airResistance();
    }
}
```
...such as in here.

Creating instances of js classes are easy.
```cpp
var example = new ExampleClass()
```

---

## Functions

### Function Declarations
Functions in JC are declared using their return type.

> [!NOTE]
>  `void` can be used here.

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

This is pretty intuitive, and should be easy to pick up for anyone with programming experience.

Example with other unique return types:

Returning a string:
```cpp (jc)
str greet(str name) {
    return "Hello, " + name + "!";
}
```

Returning a pointer:
```cpp (jc)
ptr getRawPointer() {
    return 0xDEADBEEF;
}
```

Returning a boolean:
```cpp (jc)
bool isEven(u32 x) {
    return (x % 2) == 0;
}
```

### Calling Functions
Calling a function is very universal, and the standard applies in JC.

Any function can be called with the following syntax:
```cpp
exampleFunc();
```
This also holds when the function has parameters:
```cpp
addition(2, 3);
```
The `return`ed data from a function can be fed into a variable:
> [!IMPORTANT]
>  This will throw an error if the function has a return type of `void`, or if the return type and the variable type do not fit certain criteria (shown below).
```cpp
u32 result = addition(2, 3);
```

The following rules apply when doing the above:
- If it variable matches the return type, there will be no issues.
- Return type `str` can be fed into JS arrays of `u8`.
- Return types of unsigned integers can be fed into higher capacity unsigned integer types (eg: `u8` into `u32`)
- Return types of signed integers can be fed into higher capacity signed integer types (eg: `s8` into `s32`)
- Retrun types of unsigned intgers can be fed into signed integers of a higer order (eg: `u8` goes into `s32`, but does not go into `s8`)

## Opertaors and Punctuation

### Operators
JC supports the following operators:
- Assignment: `=`
- Arithmetic: `+` `-` `*` `/` `^` `%`
- Comparison: `==` `!=` `<` `>` `<=` `>=`
- Logical: `&&` `||` `!`

### Punctuation
All lines in JC must end in a semicolon.
Yes, this means minification. (YAYAYAYAYAY LETS GOOO UNREADABLE CODE HAHA YAAA)
Arrays are declared using square brackets. (`[]`)
Function parameters are declared using parentheses. (`()`)
Code blocks are declared using curly braces. (`{}`)