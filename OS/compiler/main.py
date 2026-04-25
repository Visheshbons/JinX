import sys

# Import lexer (we will build this in the compiler folder)
# from lexer import lex


def build_jinx(source_file):
    print(f"--- JinX-C Compiler ---")
    print(f"Reading {source_file}...")

    with open(source_file, "r") as f:
        code = f.read()

    # Step 1: Lexing (Todo)
    # tokens = lex(code)
    # print("Tokens generated.")

    # Step 2: Parsing (Todo)
    # Step 3: Machine Code Generation (Todo)

    print("Compilation successful! (Placeholder)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python main.py <file.jnx>")
    else:
        build_jinx(sys.argv[1])
