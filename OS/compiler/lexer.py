import re

# Define JinX-C syntax
TOKEN_SPEC = [
    ("KEYWORD", r"\b(func|u32|u8|ptr|return|var)\b"),  # Language reserved words
    ("NUMBER", r"0x[0-9a-fA-F]+|\d+"),  # Hex (0x10) or Decimal (10)
    ("IDENTIFIER", r"[a-zA-Z_][a-zA-Z0-9_]*"),  # Variable/Function names
    ("OPERATOR", r"[=+*/-]"),  # Math and Assignment
    ("PUNCTUATION", r"[;(){}\[\]]"),  # Brackets, semicolons
    ("STRING", r'"[^"]*"'),  # "Hello World"
    ("SKIP", r"[ \t\n]+"),  # Spaces and Newlines
    ("MISMATCH", r"."),  # Anything else (Error)
]


def lex(code):
    tokens = []
    # Join all patterns into one master regex
    tok_regex = "|".join("(?P<%s>%s)" % pair for pair in TOKEN_SPEC)

    for mo in re.finditer(tok_regex, code):
        kind = mo.lastgroup
        value = mo.group()

        if kind == "SKIP":
            continue
        elif kind == "MISMATCH":
            raise SyntaxError(f"Unexpected character: {value}")
        else:
            tokens.append((kind, value))
    return tokens
