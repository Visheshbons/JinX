class Generator:
    REGISTERS = [f"R{i}" for i in range(8)]
    U8_TYPES = {"u1", "u8", "bool", "s8"}
    WORD_TYPES = {"u32", "ptr", "s32", "str", "u64", "u128", "s64", "s128"}
    ARG_REGISTERS = ["R1", "R2", "R3", "R4", "R5", "R6"]

    def __init__(self, ast):
        self.ast = ast
        self.output = []
        self.variables = {}
        self.globals = {}
        self.deleted = set()
        self.next_reg = 1
        self.label_counter = 0
        self.current_function = None
        self.current_returned = False

    def generate(self):
        for node in self.ast:
            self.visit(node)
        if not self.output or self.output[-1] != "HALT":
            self.output.append("HALT")
        return "\n".join(self.output) + "\n"

    def visit(self, node):
        handler = getattr(self, f"visit_{node.node_type}", self.generic_visit)
        return handler(node)

    def generic_visit(self, node):
        self.output.append(f"    ; unsupported JC node: {node.node_type}")

    def visit_INCLUDE(self, node):
        self.output.append(f"; included {node.value}")

    def visit_FUNCTION_DECL(self, node):
        self.current_function = node.value["name"]
        self.current_returned = False
        self.variables = dict(self.globals)
        self.deleted = set()
        self.next_reg = self.next_available_register()
        self.output.append("")
        self.output.append(f"{self.current_function}:")
        for idx, arg in enumerate(node.value["args"]):
            reg = self.ARG_REGISTERS[idx] if idx < len(self.ARG_REGISTERS) else self.allocate_register(arg["name"])
            self.variables[arg["name"]] = {"type": arg["type"], "register": reg, "const": False, "storage": "soft"}
            self.output.append(f"    ; argument {arg['type']} {arg['name']} is represented by {reg}")
        for child in node.children:
            self.visit(child)
        if not self.current_returned:
            if node.value["ret"] == "void":
                self.output.append(f"    ; end void function {self.current_function}")
            else:
                self.emit_mov("R0", 0, node.value["ret"])
            self.output.append("    RETURN")
        self.current_function = None

    def visit_VAR_DECL(self, node):
        value = node.value
        reg = self.allocate_register(value["name"])
        record = {
            "type": value["type"],
            "register": reg,
            "const": value["const"],
            "storage": value["storage"],
            "collection": value["collection"],
            "array_size": value["array_size"],
        }
        self.variables[value["name"]] = record
        if self.current_function is None:
            self.globals[value["name"]] = record
        prefix = "const " if value["const"] else ""
        storage = f"{value['storage']} " if value["const"] else ""
        collection = f"{value['collection']} " if value["collection"] else ""
        array = f"[{value['array_size']}]" if value["array_size"] else ""
        self.output.append(f"    ; {prefix}{storage}{collection}{value['type']} {value['name']}{array} = {value['init']};")
        self.emit_expression(value["init_tokens"], reg, value["type"])

    def visit_STRUCT_DECL(self, node):
        self.output.append(f"    ; trad struct {node.value['name']} metadata is compile-time only in current JASM")

    def visit_ASSIGN(self, node):
        name = node.value["name"]
        if name in self.deleted:
            self.output.append(f"    ; passive error: assignment to deleted variable {name} ignored")
            return
        var = self.variables.get(name)
        if not var:
            self.output.append(f"    ; error: assignment to undeclared variable {name} ignored")
            return
        self.output.append(f"    ; {name} = {node.value['expr']};")
        self.emit_expression(node.value["expr_tokens"], var["register"], var["type"])

    def visit_DELETE(self, node):
        name = node.value["name"]
        var = self.variables.get(name)
        if not var:
            self.output.append(f"    ; del ignored: {name} is not declared")
            return
        if var["const"] and not node.value["force"]:
            self.output.append(f"    ; passive error: const {name} requires del force")
            return
        self.deleted.add(name)
        self.output.append(f"    ; {'del force' if node.value['force'] else 'del'} {name};")
        self.emit_mov(var["register"], 0, var["type"])

    def visit_RETURN(self, node):
        self.output.append(f"    ; return {node.value['expr']};")
        self.emit_expression(node.value["expr_tokens"], "R0", "u32")
        self.output.append(f"    ; {self.current_function} returns through R0")
        self.output.append("    RETURN")
        self.current_returned = True

    def visit_CALL(self, node):
        self.output.append(f"    ; call {node.value['name']}({', '.join(node.value['args'])});")
        self.emit_call(node.value["name"], node.value["arg_tokens"])

    def visit_WHILE(self, node):
        start = self.new_label("while")
        end = self.new_label("endwhile")
        self.output.append(f"{start}:")
        self.emit_branch_if_false(node.value["cond_tokens"], end)
        for child in node.children:
            self.visit(child)
        self.output.append(f"    JUMP {start}")
        self.output.append(f"{end}:")

    def visit_IF(self, node):
        else_label = self.new_label("else")
        end_label = self.new_label("endif")
        then_node, else_node = node.children
        self.emit_branch_if_false(node.value["cond_tokens"], else_label)
        for child in then_node.children:
            self.visit(child)
        self.output.append(f"    JUMP {end_label}")
        self.output.append(f"{else_label}:")
        for child in else_node.children:
            self.visit(child)
        self.output.append(f"{end_label}:")

    def allocate_register(self, name):
        if name in self.variables:
            return self.variables[name]["register"]
        used = {var["register"] for var in self.variables.values()}
        while self.next_reg < len(self.REGISTERS) and self.REGISTERS[self.next_reg] in used:
            self.next_reg += 1
        if self.next_reg >= len(self.REGISTERS):
            self.output.append(f"    ; register pressure: reusing R7 for {name}")
            return "R7"
        reg = self.REGISTERS[self.next_reg]
        self.next_reg += 1
        return reg

    def next_available_register(self):
        used = {int(var["register"][1:]) for var in self.variables.values() if var["register"].startswith("R")}
        for idx in range(1, len(self.REGISTERS)):
            if idx not in used:
                return idx
        return len(self.REGISTERS) - 1

    def new_label(self, prefix):
        label = f"{prefix}_{self.label_counter}"
        self.label_counter += 1
        return label

    def emit_branch_if_false(self, tokens, false_label):
        tokens = self.strip_outer_parens(tokens)
        if len(tokens) == 1 and tokens[0][1] == "true":
            return
        if len(tokens) == 1 and tokens[0][1] == "false":
            self.output.append(f"    JUMP {false_label}")
            return

        comp_index = self.find_top_level_operator(tokens, {"==", "!="})
        if comp_index is not None:
            op = tokens[comp_index][1]
            lhs = tokens[:comp_index]
            rhs = tokens[comp_index + 1 :]
            self.emit_expression(lhs, "R0", "u32")
            self.emit_expression(rhs, "R7", "u32")
            self.output.append("    SUB R0, R7")
            if op == "!=":
                self.output.append(f"    JUMP_IF_ZERO R0, {false_label}")
            else:
                true_label = self.new_label("cond_true")
                self.output.append(f"    JUMP_IF_ZERO R0, {true_label}")
                self.output.append(f"    JUMP {false_label}")
                self.output.append(f"{true_label}:")
            return

        self.emit_expression(tokens, "R0", "bool")
        self.output.append(f"    JUMP_IF_ZERO R0, {false_label}")

    def emit_expression(self, tokens, dest, value_type):
        tokens = self.strip_outer_parens(tokens)
        if not tokens:
            self.emit_mov(dest, 0, value_type)
            return

        if self.is_call_expression(tokens):
            name = tokens[0][1]
            args = self.split_args(tokens[2:-1])
            self.emit_call(name, args)
            if dest != "R0":
                self.copy_register("R0", dest, value_type)
            return

        split = self.find_top_level_operator(tokens, {"+", "-"}, from_right=True)
        if split is not None:
            op = tokens[split][1]
            lhs = tokens[:split]
            rhs = tokens[split + 1 :]
            self.emit_expression(lhs, dest, value_type)
            literal = self.single_literal(rhs)
            if op == "+" and literal is not None and 0 <= literal <= 255:
                self.output.append(f"    ADDI {dest}, {literal}")
                return
            self.emit_expression(rhs, "R7", value_type)
            self.output.append(f"    {'ADD' if op == '+' else 'SUB'} {dest}, R7")
            return

        comp_index = self.find_top_level_operator(tokens, {"==", "!="})
        if comp_index is not None:
            false_label = self.new_label("bool_false")
            end_label = self.new_label("bool_end")
            self.emit_branch_if_false(tokens, false_label)
            self.emit_mov(dest, 1, "bool")
            self.output.append(f"    JUMP {end_label}")
            self.output.append(f"{false_label}:")
            self.emit_mov(dest, 0, "bool")
            self.output.append(f"{end_label}:")
            return

        if len(tokens) == 1:
            self.emit_atom(tokens[0], dest, value_type)
            return

        if any(tok[0] == "STRING" for tok in tokens):
            self.output.append(f"    ; string expression {self.tokens_to_source(tokens)} is represented by the runtime string table")
            self.emit_mov(dest, 0, value_type)
            return

        if any(tok[1] in {"[", "]", ".", "{", "}"} for tok in tokens):
            self.output.append(f"    ; aggregate expression {self.tokens_to_source(tokens)} requires runtime support; using 0")
            self.emit_mov(dest, 0, value_type)
            return

        self.output.append(f"    ; unsupported expression {self.tokens_to_source(tokens)}; using 0")
        self.emit_mov(dest, 0, value_type)

    def emit_call(self, name, arg_tokens):
        for idx, arg in enumerate(arg_tokens):
            if idx >= len(self.ARG_REGISTERS):
                self.output.append(f"    ; extra argument {idx + 1} ignored by current calling convention")
                continue
            self.emit_expression(arg, self.ARG_REGISTERS[idx], "u32")
        self.output.append(f"    CALL {name}")

    def emit_atom(self, token, dest, value_type):
        literal = self.literal_value(token)
        if literal is not None:
            self.emit_mov(dest, literal, value_type)
            return
        if token[0] == "STRING":
            self.output.append(f"    ; string literal {token[1]} is represented by the runtime string table")
            self.emit_mov(dest, 0, value_type)
            return
        if token[0] == "IDENTIFIER":
            if token[1] in self.deleted:
                self.output.append(f"    ; passive error: read of deleted variable {token[1]}; using 0")
                self.emit_mov(dest, 0, value_type)
                return
            source = self.variables.get(token[1])
            if source:
                self.copy_register(source["register"], dest, value_type)
                return
        self.output.append(f"    ; unresolved atom {token[1]}; using 0")
        self.emit_mov(dest, 0, value_type)

    def copy_register(self, source, dest, value_type):
        if source == dest:
            return
        self.output.append(f"    ; copy {source} to {dest}")
        self.emit_mov(dest, 0, value_type)
        self.output.append(f"    ADD {dest}, {source}")

    def emit_mov(self, register, value, value_type):
        if value_type in self.U8_TYPES and 0 <= value <= 255:
            self.output.append(f"    MOV8 {register}, {value}")
        else:
            self.output.append(f"    MOV32 {register}, {value}")

    def single_literal(self, tokens):
        tokens = self.strip_outer_parens(tokens)
        if len(tokens) != 1:
            return None
        return self.literal_value(tokens[0])

    @staticmethod
    def literal_value(token):
        kind, value = token
        if kind == "NUMBER":
            return int(value, 0)
        if kind == "CHAR":
            inner = value[1:-1]
            if inner == r"\n":
                return 10
            if inner == r"\0":
                return 0
            return ord(inner[:1]) if inner else 0
        if value == "true":
            return 1
        if value == "false":
            return 0
        return None

    @staticmethod
    def is_call_expression(tokens):
        return len(tokens) >= 3 and tokens[0][0] == "IDENTIFIER" and tokens[1][1] == "(" and tokens[-1][1] == ")"

    @staticmethod
    def split_args(tokens):
        args = []
        current = []
        depth = 0
        for token in tokens:
            value = token[1]
            if value == "," and depth == 0:
                args.append(current)
                current = []
                continue
            if value in "([{":
                depth += 1
            elif value in ")]}" and depth > 0:
                depth -= 1
            current.append(token)
        if current:
            args.append(current)
        return args

    @classmethod
    def strip_outer_parens(cls, tokens):
        tokens = list(tokens)
        while len(tokens) >= 2 and tokens[0][1] == "(" and tokens[-1][1] == ")":
            depth = 0
            wraps = True
            for idx, token in enumerate(tokens):
                if token[1] == "(":
                    depth += 1
                elif token[1] == ")":
                    depth -= 1
                    if depth == 0 and idx != len(tokens) - 1:
                        wraps = False
                        break
            if not wraps:
                break
            tokens = tokens[1:-1]
        return tokens

    @staticmethod
    def find_top_level_operator(tokens, operators, from_right=False):
        iterable = range(len(tokens) - 1, -1, -1) if from_right else range(len(tokens))
        depth = 0
        for idx in iterable:
            value = tokens[idx][1]
            if from_right:
                if value in ")]}":
                    depth += 1
                    continue
                if value in "([{":
                    depth -= 1
                    continue
            else:
                if value in "([{":
                    depth += 1
                    continue
                if value in ")]}":
                    depth -= 1
                    continue
            if depth == 0 and value in operators:
                return idx
        return None

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