import os
import sys

from generator import Generator  # generator.py
from lexer import lex  # lexer.py
from parser import Parser  # parser.py


def build_jinx(source_file):
    print(f"--- JinX-C Compiler ---")

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

    # Step 2: Parsing
    parser = Parser(tokens)
    try:
        ast = parser.parse()
        print("\nAST generated successfully:")
        for node in ast:
            print(f"  {node}")
    except Exception as e:
        print(f"Parser Error: {e}")
        return

    print("\nCompilation successful!")

    # Step 3: Machine Code Generation
    print("\nStarting JASM generation...")

    gen = Generator(ast)
    jasm_code = gen.generate()

    # Save to file
    out_file = source_file.replace(".jc", ".jasm")
    with open(out_file, "w") as f:
        f.write(jasm_code)

    print(f"\nSuccess! Wrote JASM to {out_file}")
    print("--- Output ---")
    print(jasm_code)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python main.py <path_to_file.jc>")
    else:
        build_jinx(sys.argv[1])
