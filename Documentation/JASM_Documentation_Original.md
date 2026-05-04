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

1. HALT (0x00)
Halt is the terminator of the program. When scripting, write this on a seperate line to comply with syntax rules.

2. MOV8 (0x01)
Assigns an integer value or ASCII character (8 bit values from 0 to 255) literal to a register from R0 to R7. When scripting, write MOV8 <register> <value> to comply with syntax rules.

3. MOV32 (0x02)
When extending beyond the assignment of integer values and ASCII character literals (32 bit values) for registers R0 to R7, use MOV32 instead of MOV8 for demanding tasks.

4. ADD (0x10)
Adds two registers to get a result. The first register is referred to as the destination, and the other is referred to as the source. While addition is interchangable, the destination always adds to the source.

5. SUB (0x11)
Subtracts two registers to get a result. Similar to ADD, the first register and second register is referenced as a destination and a source. While subtraction is interchangable, the destination always subtracts from the source.

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

When scripting, write JUMP_IF_ZERO <register> <label>