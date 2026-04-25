import os
import sys

from lexer import lex  # lexer.py


def build_jinx(source_file):
    print(f"--- JinX-C Compiler ---")
    print(f"Reading {source_file}...")

    # Check if file exists before trying to open it
    if not os.path.exists(source_file):
        print(f"Error: Could not find file at {source_file}")
        print(f"Current Working Directory: {os.getcwd()}")
        return

    print(f"Reading {source_file}...")
    with open(source_file, "r") as f:
        code = f.read()

    # Step 1: Lexing
    try:
        tokens = lex(code)
        print("Tokens generated successfully:")
        for t in tokens:
            print(f"  {t}")
    except SyntaxError as e:
        print(f"Lexer Error: {e}")
        return

    # Step 2: Parsing (Todo)
    # Step 3: Machine Code Generation (Todo)

    print("\nCompilation successful! (Placeholder)")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python main.py <path_to_file.jc>")
    else:
        build_jinx(sys.argv[1])
