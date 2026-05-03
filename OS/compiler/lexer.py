# Define JinX-C syntax
import re

TOKEN_SPEC = {
    "COMMENT": r"//.*",
    "INCLUDE": r"#include\s+<[^>]+>",
    "KEYWORD": r"\b(func|u32|u8|ptr|str|bool|return|var|if|while|true|false)\b",
    "NUMBER": r"0x[0-9a-fA-F]+|\d+",
    "IDENTIFIER": r"[a-zA-Z_][a-zA-Z0-9_]*",
    "OPERATOR": r"[=+*/-]|==|!=|<=|>=",
    "PUNCTUATION": r"[;(){}\[\]]",
    "STRING": r'"[^"]*"',
    "SKIP": r"[ \t\n]+",
    "MISMATCH": r"."
}


def lex(code):
    tokens = []
    # Join all patterns into one master regex
    tok_regex = "|".join("(?P<%s>%s)" % pair for pair in TOKEN_SPEC)

    for mo in re.finditer(tok_regex, code):
        kind = mo.lastgroup
        value = mo.group()

        if kind == "SKIP" or kind == "COMMENT":  # Ignore comments just like spaces
            continue
        elif kind == "MISMATCH":
            raise SyntaxError(f"Unexpected character: {value}")
        else:
            tokens.append((kind, value))
    return tokens
