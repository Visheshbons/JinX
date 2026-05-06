import os
import sys

from generator import Generator
from lexer import lex
from parser import Parser


def build_jinx(source_file, verbose=True):
    if verbose:
        print("--- JinX-C Compiler ---")

    if not os.path.exists(source_file):
        raise FileNotFoundError(f"Could not find file at {source_file}")

    if verbose:
        print(f"Reading {source_file}...")
    with open(source_file, "r", encoding="utf-8") as f:
        code = f.read()

    tokens = lex(code)
    if verbose:
        print("Tokens generated successfully")

    parser = Parser(tokens)
    ast = parser.parse()
    if verbose:
        print("AST generated successfully")

    gen = Generator(ast)
    jasm_code = gen.generate()

    out_file = source_file.rsplit(".", 1)[0] + ".jasm"
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(jasm_code)

    if verbose:
        print(f"Success! Wrote JASM to {out_file}")
    return out_file, jasm_code


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python main.py <path_to_file.jc>")
        sys.exit(1)
    try:
        build_jinx(sys.argv[1])
    except Exception as exc:
        print(f"Compiler Error: {exc}")
        sys.exit(1)