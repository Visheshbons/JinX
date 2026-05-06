class Node:
    def __init__(self, node_type, value=None, children=None):
        self.node_type = node_type
        self.value = value
        self.children = children if children else []

    def __repr__(self):
        return f"({self.node_type}: {self.value} {self.children})"


TYPES = {"u1", "u8", "u32", "u64", "u128", "s8", "s32", "s64", "s128", "ptr", "str", "bool", "void"}
CONST_MODIFIERS = {"const", "hard", "soft"}
COLLECTION_TAGS = {"trad", "js"}


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self, distance=0):
        idx = self.pos + distance
        if idx < len(self.tokens):
            return self.tokens[idx]
        return None

    def consume(self, expected_type=None, expected_value=None):
        token = self.peek()
        if not token:
            raise SyntaxError("Unexpected end of input")
        if expected_type and token[0] != expected_type:
            raise SyntaxError(f"Expected {expected_type}, got {token[0]} ('{token[1]}')")
        if expected_value and token[1] != expected_value:
            raise SyntaxError(f"Expected '{expected_value}', got '{token[1]}'")
        self.pos += 1
        return token

    def parse(self):
        nodes = []
        while self.peek():
            tok = self.peek()
            if tok[0] == "INCLUDE":
                nodes.append(Node("INCLUDE", value=self.consume()[1]))
            elif self._looks_like_function():
                nodes.append(self.parse_function())
            elif tok[0] == "KEYWORD" and (tok[1] in TYPES or tok[1] in CONST_MODIFIERS or tok[1] in COLLECTION_TAGS):
                nodes.append(self.parse_declaration())
            else:
                raise SyntaxError(f"Unexpected top-level token {tok[1]!r}")
        return nodes

    def _looks_like_function(self):
        return (
            self.peek()
            and self.peek(1)
            and self.peek(2)
            and self.peek()[0] == "KEYWORD"
            and self.peek()[1] in TYPES
            and self.peek(1)[0] == "IDENTIFIER"
            and self.peek(2)[1] == "("
        )

    def parse_function(self):
        ret_type = self.consume("KEYWORD")[1]
        name = self.consume("IDENTIFIER")[1]
        self.consume("PUNCTUATION", "(")
        args = self.parse_args()
        self.consume("PUNCTUATION", ")")
        body = self.parse_block()
        return Node("FUNCTION_DECL", {"name": name, "ret": ret_type, "args": args}, body)

    def parse_args(self):
        args = []
        while self.peek() and self.peek()[1] != ")":
            arg_type = self.consume("KEYWORD")[1]
            if arg_type not in TYPES:
                raise SyntaxError(f"Expected argument type, got {arg_type!r}")
            arg_name = self.consume("IDENTIFIER")[1]
            args.append({"type": arg_type, "name": arg_name})
            if self.peek() and self.peek()[1] == ",":
                self.consume("PUNCTUATION", ",")
        return args

    def parse_block(self):
        self.consume("PUNCTUATION", "{")
        body = []
        while self.peek() and self.peek()[1] != "}":
            body.append(self.parse_statement())
        self.consume("PUNCTUATION", "}")
        return body

    def parse_statement(self):
        tok = self.peek()
        if tok[0] == "KEYWORD" and (tok[1] in TYPES or tok[1] in CONST_MODIFIERS or tok[1] in COLLECTION_TAGS):
            return self.parse_declaration()
        if tok[0] == "KEYWORD" and tok[1] == "return":
            return self.parse_return()
        if tok[0] == "KEYWORD" and tok[1] == "while":
            return self.parse_while()
        if tok[0] == "KEYWORD" and tok[1] == "if":
            return self.parse_if()
        if tok[0] == "KEYWORD" and tok[1] == "del":
            return self.parse_delete()
        if tok[0] == "IDENTIFIER" and self.peek(1) and self.peek(1)[1] == "(":
            return self.parse_call_stmt()
        if tok[0] == "IDENTIFIER" and self.peek(1) and self.peek(1)[1] == "=":
            return self.parse_assignment()
        raise SyntaxError(f"Unexpected statement token {tok[1]!r}")

    def parse_declaration(self):
        tags = []
        is_const = False
        storage = "soft"
        collection = None

        while self.peek() and self.peek()[0] == "KEYWORD" and self.peek()[1] in CONST_MODIFIERS | COLLECTION_TAGS:
            word = self.consume("KEYWORD")[1]
            tags.append(word)
            if word == "const":
                is_const = True
            elif word in {"hard", "soft"}:
                storage = word
            elif word in COLLECTION_TAGS:
                collection = word

        if self.peek() and self.peek()[1] == "struct":
            return self.parse_struct_declaration(tags, is_const, storage, collection)

        var_type = self.consume("KEYWORD")[1]
        if var_type not in TYPES:
            raise SyntaxError(f"Expected variable type, got {var_type!r}")
        var_name = self.consume("IDENTIFIER")[1]

        array_size = None
        if self.peek() and self.peek()[1] == "[":
            self.consume("PUNCTUATION", "[")
            size_tokens = self.collect_until({"]"})
            self.consume("PUNCTUATION", "]")
            array_size = self.tokens_to_source(size_tokens)

        init = []
        if self.peek() and self.peek()[1] == "=":
            self.consume("OPERATOR", "=")
            init = self.collect_until({";"})
        self.consume("PUNCTUATION", ";")
        return Node(
            "VAR_DECL",
            {
                "type": var_type,
                "name": var_name,
                "init": self.tokens_to_source(init) if init else "0",
                "init_tokens": init,
                "const": is_const,
                "storage": storage,
                "collection": collection,
                "array_size": array_size,
                "tags": tags,
            },
        )

    def parse_struct_declaration(self, tags, is_const, storage, collection):
        self.consume("KEYWORD", "struct")
        name = self.consume("IDENTIFIER")[1]
        fields = self.parse_block()
        if self.peek() and self.peek()[1] == ";":
            self.consume("PUNCTUATION", ";")
        return Node("STRUCT_DECL", {"name": name, "tags": tags, "const": is_const, "storage": storage, "collection": collection}, fields)

    def parse_assignment(self):
        name = self.consume("IDENTIFIER")[1]
        self.consume("OPERATOR", "=")
        expr = self.collect_until({";"})
        self.consume("PUNCTUATION", ";")
        return Node("ASSIGN", {"name": name, "expr": self.tokens_to_source(expr), "expr_tokens": expr})

    def parse_delete(self):
        self.consume("KEYWORD", "del")
        force = False
        if self.peek() and self.peek()[1] == "force":
            self.consume("KEYWORD", "force")
            force = True
        name = self.consume("IDENTIFIER")[1]
        self.consume("PUNCTUATION", ";")
        return Node("DELETE", {"name": name, "force": force})

    def parse_return(self):
        self.consume("KEYWORD", "return")
        expr = self.collect_until({";"})
        self.consume("PUNCTUATION", ";")
        return Node("RETURN", {"expr": self.tokens_to_source(expr), "expr_tokens": expr})

    def parse_while(self):
        self.consume("KEYWORD", "while")
        cond = self.parse_parenthesized_tokens()
        body = self.parse_block()
        return Node("WHILE", {"cond": self.tokens_to_source(cond), "cond_tokens": cond}, body)

    def parse_if(self):
        self.consume("KEYWORD", "if")
        cond = self.parse_parenthesized_tokens()
        then_body = self.parse_block()
        else_body = []
        if self.peek() and self.peek()[1] == "else":
            self.consume("KEYWORD", "else")
            else_body = self.parse_block()
        return Node("IF", {"cond": self.tokens_to_source(cond), "cond_tokens": cond}, [Node("THEN", children=then_body), Node("ELSE", children=else_body)])

    def parse_call_stmt(self):
        name = self.consume("IDENTIFIER")[1]
        args = self.parse_call_args()
        self.consume("PUNCTUATION", ";")
        return Node("CALL", {"name": name, "args": [self.tokens_to_source(arg) for arg in args], "arg_tokens": args})

    def parse_call_args(self):
        self.consume("PUNCTUATION", "(")
        args = []
        current = []
        depth = 0
        while self.peek() and not (self.peek()[1] == ")" and depth == 0):
            if self.peek()[1] == "," and depth == 0:
                args.append(current)
                current = []
                self.consume("PUNCTUATION", ",")
                continue
            tok = self.consume()
            if tok[1] in "([{":
                depth += 1
            elif tok[1] in ")]}" and depth > 0:
                depth -= 1
            current.append(tok)
        if current:
            args.append(current)
        self.consume("PUNCTUATION", ")")
        return args

    def parse_parenthesized_tokens(self):
        self.consume("PUNCTUATION", "(")
        expr = self.collect_until({")"})
        self.consume("PUNCTUATION", ")")
        return expr

    def collect_until(self, stop_values):
        values = []
        depth = 0
        while self.peek():
            value = self.peek()[1]
            if value in stop_values and depth == 0:
                break
            tok = self.consume()
            if tok[1] in "([{":
                depth += 1
            elif tok[1] in ")]}" and depth > 0:
                depth -= 1
            values.append(tok)
        return values

    @staticmethod
    def tokens_to_source(tokens):
        text = " ".join(value for _, value in tokens).strip()
        replacements = {
            " ,": ",",
            " . ": ".",
            " [ ": "[",
            " ]": "]",
            " (": "(",
            "( ": "(",
            " )": ")",
            " { ": "{",
            " }": "}",
        }
        for old, new in replacements.items():
            text = text.replace(old, new)
        return text