# JASM Original Documentation:

JASM or JinX Assembly is a low-level language which runs JinX OS. It is assembled by C++ code and then executed by a virtual machine named JASM_VM.

# To get started:

1. Download the JinX_Virt folder.
2. Install GCC if you have not.
3. Run the following commands:
    g++ Core.cpp JinXVirtual.cpp -std=c++11 -o JASM_VM
    g++ JA.cpp -std=c++11 ja
4. Assemble the task.ja file by running:
    ./ja task.ja kernel.jbin
5. Then run the virtual machine:
    ./JASM_VM

The terminal should display the expected output:
    Y%
% in zsh indicates the line ends "here".

# Commands:

Note: where <value> is used, exclude string types unless explicitly specified by <value : string>.
Additional comment: The program automatically begins on the first line. To end the program HALT must be used.

1. HALT (0x00)
Halt is the terminator of the program. When scripting, write this on a seperate line to comply with syntax rules.

2. MOV8 (0x01)
Assigns an integer value or ASCII character (8 bit values from 0 to 255) literal to a register from R0 to R7. When scripting, write MOV8 <register> <value> to comply with syntax rules.

3. MOV32 (0x02)
Typically (and a much better way of) storing integer values in registers. When scripting, write M0V32 <register> <value> to comply with syntax rules.

4. ADD (0x10)
Adds two registers to get a result. The first register is referred to as the destination, and the other is referred to as the source. While addition is interchangable, the destination always adds to the source.

5. SUB (0x11)
Subtracts two registers to get a result. Similar to ADD, the first register and second register is referenced as a destination and a source. Subtraction is not interchangable although the destination always subtracts from the source.

6. ADDI (0x12)
Adds an integer value to the chosen register. When scripting, write ADDI <register> <integer>

7. JUMP (0x20)
Unconditionally switches to starting from a label, changing the sequence of execution. This is a fundamental control flow tool.

A label is notated as,
    Label:
        ; This is a comment notated by the semi-colon and then followed by any text. The assembler will not parse this.
        ; Indented code to show instructions run under the label.

When scripting, write JUMP <label> to use the command.

8. JUMP_IF_ZERO (0x21)
On the condition a register's value is equalivent to 0, the sequence of execution switches to starting from a label.

When scripting, write JUMP_IF_ZERO <register> <label> to use the command.

9. NOT (0x22)
Forces an inverse condition of the original condition for conditional jumps. 

When scripting, write NOT on the line before the condition.

10. INTOUT (0x30)
Writes an integer value to the console according to the chosen register's value.

When scripting, write INTOUT <register> to use the command.

11. CHAROUT (0x31)
Writes a character to the console according to the chosen register's value.

When scripting, write CHAROUT <register> to use the command.

12. OB (0x32)
Outputs a character array to the console, assuming that the character array has at least two ASCII values and is in a label.

When scripting, write OB <label> to comply with syntax rules.

13. WRITE_MEM (0x40)
Writes 32-bit memory to a register according to the memory address.

Memory addresses start with 0x and can be of any size within 0x00000000 to 0xFFFFFFFF (at least I believe so :D)

When scripting, write WRITE_MEM <memory_address> <register>

14. READ_MEM (0x41)
Reads 32-bit memory to a register according to the memory address.

When scripting, write READ_MEM <memory_address> <register>

15. READ_REG (0x42)
Transfers the address register's value to the destination register.

When scripting, write READ_REG <register> <register>

16. WRITE_REG (0x42)
Transfers the destination register's value to the address register.

When scripting, write WRITE_REG <register> <register>

17. READ_KEY (0x60)
Reads keyboard input from the user, and stores the keyboard input in the desired register.

When scripting, write READ_KEY <register>

18. PUSH (0x70)
Add a value to the stack.

Syntax: PUSH <value>

19. POP (0x71)
Removes a value from the stack.

Syntax: POP <value>

20. CALL (0x72)
Jumps to a "promising" label.

Syntax: CALL <label>

21. RETURN (0x73)
Returns back to after the CALL comand.

22. CMP (0x80)
Compares two registers for the following conditional jump statements: JUMP_IF_LT, JUMP_IF_EQ, and JUMP_IF_GT.

Syntax: CMP <register> <register>

23. JUMP_IF_EQ (0x81)
Jumps to a label if the two registers equal.

Syntax: JUMP_IF_EQ <label>

24. JUMP_IF_LT (0x82)
Jumps to a label if the destination register is less than the source register.

25. JUMP_IF_GT (0x83)
Jumps to a label if the destination register is the greater than the source register.

26. LEA (0x90)
Loads a written memory address into the chosen register.

Syntax: LEA <memory_address> <register>

27. MUL (0xA0)
Multiplies the destination register's value by the source register's value. This is interchangable like addition.

28. DIV (0xA1)
Divides the destination register's value by the source register's value. This is not interchangable like subtraction.

Warning: avoid dividing by zero.

29. MOD (0xA2)
Returns the remainder created by the division operator where integers do not mathematically split into whole numbers.

30. BOOLMOV (0xB0)
Assigns a boolean value to a register. The boolean value is either TRUE or FALSE.

Syntax: BOOLMOV <register> <boolean>

31. BOOLAND (0xB1)
Assigns TRUE to the destination register if the destination register and the source register are 1, otherwise FALSE.

Syntax: BOOLAND <register> <register>

32. BOOLOR (0xB2)
Assigns TRUE to the destination register if the destination register or the source register are 1, otherwise FALSE.

Syntax: BOOLOR <register> <register>

33. BOOLNOT (0xB3)
Assigns the opposite of the chosen register's boolean.

Syntax: BOOLNOT <register>

34. BOOLXOR (0xB4)
Assigns TRUE to the destination register if the destination register and the source register are opposite values, otherwise FALSE.

Syntax: BOOLXOR <register> <register>