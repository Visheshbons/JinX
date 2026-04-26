class Node:
    def __init__(self, node_type, value=None, children=None):
        self.node_type = node_type
        self.value = value
        self.children = children if children else []

    def __repr__(self):
        return f"({self.node_type}: {self.value} {self.children})"


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self, distance=0):
        if self.pos + distance < len(self.tokens):
            return self.tokens[self.pos + distance]
        return None

    def consume(self, expected_type=None):
        token = self.peek()
        if not token:
            raise SyntaxError("Unexpected end of input")
        if expected_type and token[0] != expected_type:
            raise SyntaxError(
                f"Expected {expected_type}, got {token[0]} ('{token[1]}')"
            )
        self.pos += 1
        return token

    def parse(self):
        root_nodes = []
        while self.peek():
            token_type, value = self.peek()
            if token_type == "INCLUDE":
                root_nodes.append(Node("INCLUDE", value=self.consume()[1]))
            elif token_type == "KEYWORD" and value == "func":
                root_nodes.append(self.parse_function())
            else:
                self.consume()  # Safety skip
        return root_nodes

    def parse_function(self):
        self.consume("KEYWORD")  # func
        name = self.consume("IDENTIFIER")[1]
        self.consume("PUNCTUATION")  # (

        args = []
        # Keep parsing until we hit the closing parenthesis
        while self.peek() and self.peek()[1] != ")":
            arg_type = self.consume("KEYWORD")[1]  # e.g., 'u8'
            arg_name = self.consume("IDENTIFIER")[1]  # e.g., 'c'
            args.append({"type": arg_type, "name": arg_name})

            # If you support multiple args like (u8 x, u8 y), you'd consume commas here:
            if self.peek() and self.peek()[1] == ",":
                self.consume("PUNCTUATION")  # Consume the comma

        self.consume("PUNCTUATION")  # )

        # return type comes after the parens
        ret_type = (
            self.consume("KEYWORD")[1]
            if self.peek() and self.peek()[0] == "KEYWORD"
            else None
        )

        self.consume("PUNCTUATION")  # {

        body = []
        while self.peek() and self.peek()[1] != "}":
            # Check if variable declaration (like u32)
            if self.peek()[0] == "KEYWORD" and self.peek()[1] in [
                "u32",
                "u8",
                "ptr",
                "str",
            ]:
                body.append(self.parse_declaration())
            else:
                self.consume()  # Placeholder for other statements

        self.consume("PUNCTUATION")  # }
        return Node(
            "FUNCTION_DECL", value={"name": name, "ret": ret_type}, children=body
        )

    def parse_declaration(self):
        var_type = self.consume("KEYWORD")[1]
        var_name = self.consume("IDENTIFIER")[1]
        self.consume("OPERATOR")  # =
        value_token = self.consume()
        if value_token[0] not in ["NUMBER", "STRING", "IDENTIFIER"]:
            raise SyntaxError(f"Expected a value, got {value_token[0]}")
        value = value_token[1]
        self.consume("PUNCTUATION")  # ;
        return Node(
            "VAR_DECL", value={"type": var_type, "name": var_name, "init": value}
        )
