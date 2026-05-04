import re

TOKEN_SPEC = {
    "COMMENT": r"//.*",
    "INCLUDE": r"#include\s+<[^>]+>",
    "KEYWORD": r"\b(func|u32|u8|ptr|str|bool|void|return|var|if|while|true|false)\b",
    "NUMBER": r"0x[0-9a-fA-F]+|\d+",
    "IDENTIFIER": r"[a-zA-Z_][a-zA-Z0-9_]*",
    "OPERATOR": r"==|!=|<=|>=|[=+*/-]",
    "PUNCTUATION": r"[;,(){}\[\]]",
    "STRING": r'"[^"\\]*(?:\\.[^"\\]*)*"',
    "SKIP": r"[ \t\n\r]+",
    "MISMATCH": r".",
}

_TOK_REGEX = re.compile("|".join(f"(?P<{name}>{pattern})" for name, pattern in TOKEN_SPEC.items()))


def lex(code):
    tokens = []
    for mo in _TOK_REGEX.finditer(code):
        kind = mo.lastgroup
        value = mo.group()
        if kind in ("SKIP", "COMMENT"):
            continue
        if kind == "MISMATCH":
            raise SyntaxError(f"Unexpected character: {value!r}")
        tokens.append((kind, value))
    return tokens