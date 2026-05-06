import re

TOKEN_SPEC = [
    ("COMMENT", r"//.*"),
    ("INCLUDE", r"#include\s+<[^>]+>"),
    ("STRING", r'"[^"\\]*(?:\\.[^"\\]*)*"'),
    ("CHAR", r"'[^'\\]*(?:\\.[^'\\]*)?'"),
    ("NUMBER", r"0x[0-9a-fA-F]+|\d+"),
    (
        "KEYWORD",
        r"\b(?:const|hard|soft|trad|js|struct|u1|u8|u32|u64|u128|s8|s32|s64|s128|ptr|str|bool|void|return|if|else|while|true|false|del|force)\b",
    ),
    ("IDENTIFIER", r"[a-zA-Z_][a-zA-Z0-9_]*"),
    ("OPERATOR", r"==|!=|<=|>=|&&|\|\||[=+*/\-^%<>!]"),
    ("PUNCTUATION", r"[;,:.(){}\[\]]"),
    ("SKIP", r"[ \t\n\r]+"),
    ("MISMATCH", r"."),
]

_TOK_REGEX = re.compile("|".join(f"(?P<{name}>{pattern})" for name, pattern in TOKEN_SPEC))


def lex(code):
    """Tokenize documented JinX-C source.

    The lexer recognizes the keywords, operators, and punctuation listed in
    Documentation/JinX-C_Docs.md while preserving literals for the parser and
    generator. Comments and whitespace are discarded.
    """
    tokens = []
    for match in _TOK_REGEX.finditer(code):
        kind = match.lastgroup
        value = match.group()
        if kind in ("SKIP", "COMMENT"):
            continue
        if kind == "MISMATCH":
            raise SyntaxError(f"Unexpected character: {value!r}")
        tokens.append((kind, value))
    return tokens