# JinX-C (JC)
This is a custom programming language used for JinX.
It is built to be just like C.

## Documentation Notes
In this documentation, all code snippets will be marked as C for code formatting in Github. However, all code is JC code.

## Comments
Currently, the only way to use a comment is:
```C
// This is a comment
```
As of the time of writing, `/* comment */` syntax is not supported.

## Syntax
JC, like C, runs from the `main()` function.
A good example of some JC code is as follows:
```C
#include <stdio.h>

func main() u32 {
  printf("Hello World!");
  return 0;
}
```

**Line 1**: `#include <stdio.h>` is to include a *header file*.
In this case, the header in question is to let us use input/output functions such as `printf()` (used in line 4).

**Line 2**: This is a blank line, and is ignored by the compiler.
However, this makes the code more readable.

**Line 3**: `main()` is the main function.
All code in the curly brackets `{}` will be executed first.
`u32` is used to declare the output type, which is an *32-bit unsigned integer*.
If the `main()` function returns `1`, we assume an error has occured, and if it returns `0`, that means the code ran through and has completed running.

**Line 4**: `printf()` is a function derived from the `include` statment at the start of the code.
This function is used to output text to the screen.

> [!IMPORTANT]
> Every JC statement MUST end with a semicolon `;`. 

**Line 5**: `return 0;` ends the program, as the main loop returns a value.
If we returned `1`, then JinX will assume an error has occured, which caused termination.
However, returning `0` will let JinX know that the program has closed without errors.

**Line 6**: This is just the closing curly bracket `}` for the `main()` loop.

> [!NOTE]
> The entire `main()` can also be written as 
> ```C
>  func main() u32 {printf("Hello World!");return 0;}
> ```
> However, we add line breaks for readability.

## JC Output
JC uses the `<stdio.h>` library (which is named for consistancy with C).
This provides the `printf()` function.
An example of output:
```C
#include <stdio.h>

func main() u32 {
  printf("Hello World!\n");
  printf("I am reading JC documentation.\n");
  printf("I want to learn how to use JinX.")
  return 0;
}
```
Here, note that a `\n` is required.
However, in JC, the `<stdio.h>` library also contains another function, which is the `println()` function.
The above code could be rewritten as:
```C
#include <stdio.h>

func main() u32 {
  println("Hello World!");
  println("I am reading JC documentation.");
  println("I want to learn how to use JinX.")
  return 0;
}
```
The `println()` function simply adds a `\n` automatically.

> [!TIP]
> To avoid unexpected behavior in output, it is reccomended to use `println` for the majority of logging, with `printf` only being used for very specific cases.

## Variables
Similar to C, JC also requires a type declaration in variables.
JC has the following types:
* `u8`: 8-bit unsigned integer.
* `u32`: 32-bit unsigned integer.
* `ptr`: A raw memory pointer.
* `str`: A sequence of characters. In JC, this is internally treated as a pointer to a null-terminated array of u8 bytes.
* `bool`: A boolean value, either true (1) or false (0). Internally stored as a u8.
Example usage:
```C
u32 count = 10;
str message = "Hello world!";
ptr video_memory = 0xB8000;
```


## Functions
Functions are declared as such:
```C

```
