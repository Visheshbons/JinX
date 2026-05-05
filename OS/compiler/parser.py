class Node:
    def __init__(self, node_type, value=None, children=None):
        self.node_type = node_type
        self.value = value
        self.children = children if children else []

    def __repr__(self):
        return f"({self.node_type}: {self.value} {self.children})"


TYPES = {"u32", "u8", "ptr", "str", "bool", "void"}


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
            else:
                self.consume()
        return nodes

    def _looks_like_function(self):
        t0 = self.peek()
        t1 = self.peek(1)
        t2 = self.peek(2)
        if not (t0 and t1 and t2):
            return False
        if t0[0] == "KEYWORD" and t0[1] == "func":
            return True
        return t0[0] == "KEYWORD" and t0[1] in TYPES and t1[0] == "IDENTIFIER" and t2[1] == "("

    def parse_function(self):
        if self.peek()[1] == "func":
            self.consume("KEYWORD", "func")
            name = self.consume("IDENTIFIER")[1]
            self.consume("PUNCTUATION", "(")
            args = self.parse_args()
            self.consume("PUNCTUATION", ")")
            ret_type = self.consume("KEYWORD")[1] if self.peek() and self.peek()[0] == "KEYWORD" and self.peek()[1] in TYPES else "void"
        else:
            ret_type = self.consume("KEYWORD")[1]
            name = self.consume("IDENTIFIER")[1]
            self.consume("PUNCTUATION", "(")
            args = self.parse_args()
            self.consume("PUNCTUATION", ")")

        self.consume("PUNCTUATION", "{")
        body = []
        while self.peek() and self.peek()[1] != "}":
            body.append(self.parse_statement())
        self.consume("PUNCTUATION", "}")
        return Node("FUNCTION_DECL", {"name": name, "ret": ret_type, "args": args}, body)

    def parse_args(self):
        args = []
        while self.peek() and self.peek()[1] != ")":
            arg_type = self.consume("KEYWORD")[1]
            arg_name = self.consume("IDENTIFIER")[1]
            args.append({"type": arg_type, "name": arg_name})
            if self.peek() and self.peek()[1] == ",":
                self.consume("PUNCTUATION", ",")
        return args

    def parse_statement(self):
        tok = self.peek()
        if tok[0] == "KEYWORD" and tok[1] in TYPES:
            return self.parse_declaration()
        if tok[0] == "KEYWORD" and tok[1] == "return":
            return self.parse_return()
        if tok[0] == "KEYWORD" and tok[1] == "while":
            return self.parse_while()
        if tok[0] == "IDENTIFIER" and self.peek(1) and self.peek(1)[1] == "(":
            return self.parse_call_stmt()
        self.consume()
        return Node("NOOP")

    def parse_declaration(self):
        var_type = self.consume("KEYWORD")[1]
        var_name = self.consume("IDENTIFIER")[1]
        self.consume("OPERATOR", "=")
        init = self.parse_value_until_semicolon()
        self.consume("PUNCTUATION", ";")
        return Node("VAR_DECL", {"type": var_type, "name": var_name, "init": init})
        
    def parse_variable_del(self):
        var_type = self.consume("KEYWORD")[1]
        var_name = self.consume("VAR_DECL")[1]
        # w.i.p.

    def parse_return(self):
        self.consume("KEYWORD", "return")
        value = self.parse_value_until_semicolon()
        self.consume("PUNCTUATION", ";")
        return Node("RETURN", value=value)

    def parse_while(self):
        self.consume("KEYWORD", "while")
        self.consume("PUNCTUATION", "(")
        cond = []
        while self.peek() and self.peek()[1] != ")":
            cond.append(self.consume()[1])
        self.consume("PUNCTUATION", ")")
        self.consume("PUNCTUATION", "{")
        body = []
        while self.peek() and self.peek()[1] != "}":
            body.append(self.parse_statement())
        self.consume("PUNCTUATION", "}")
        return Node("WHILE", value=" ".join(cond), children=body)

    def parse_call_stmt(self):
        name = self.consume("IDENTIFIER")[1]
        self.consume("PUNCTUATION", "(")
        args = []
        current = []
        while self.peek() and self.peek()[1] != ")":
            if self.peek()[1] == ",":
                args.append(" ".join(current).strip())
                current = []
                self.consume()
            else:
                current.append(self.consume()[1])
        if current:
            args.append(" ".join(current).strip())
        self.consume("PUNCTUATION", ")")
        if self.peek() and self.peek()[1] == ";":
            self.consume("PUNCTUATION", ";")
        return Node("CALL", value={"name": name, "args": args})

    def parse_value_until_semicolon(self):
        vals = []
        while self.peek() and self.peek()[1] != ";":
            vals.append(self.consume()[1])
        return " ".join(vals).strip()