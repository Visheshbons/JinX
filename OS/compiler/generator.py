class Generator:
    def __init__(self, ast):
        self.ast = ast
        self.output = []
        self.local_vars = {}  # Tracks variable names and their stack offsets
        self.current_offset = 0
        self.label_counter = 0

    def generate(self):
        """Iterates through the root nodes of the AST."""
        for node in self.ast:
            self.visit(node)

        return "\n".join(self.output)

    def visit(self, node):
        fn = getattr(self, f"visit_{node.node_type}", self.generic_visit)
        return fn(node)

    def generic_visit(self, node):
        self.output.append(f"  ; WARNING: Unhandled node type: {node.node_type}")

    def visit_INCLUDE(self, node):
        self.output.append(f"; included {node.value}")

    def visit_FUNCTION_DECL(self, node):
        name = node.value["name"]
        self.local_vars = {}
        self.current_offset = 0
        self.output.append(f"\nglobal {name}")
        self.output.append(f"{name}:")
        self.output.append("  push rbp")
        self.output.append("  mov rbp, rsp")
        for child in node.children:
            self.visit(child)
        self.output.append("  mov rsp, rbp")
        self.output.append("  pop rbp")
        self.output.append("  ret")

    def _stack_ref(self, offset):
        return f"[rbp{offset}]" if offset < 0 else f"[rbp+{offset}]"

    def visit_VAR_DECL(self, node):
        n = node.value["name"]
        t = node.value["type"]
        v = node.value["init"]
        if t == "u32":
            self.current_offset -= 4
            d = "dword"
        elif t in ("u8", "bool"):
            self.current_offset -= 1
            d = "byte"
        else:
            self.current_offset -= 8
            d = "qword"
        self.local_vars[n] = self.current_offset
        self.output.append(f"  ; {t} {n} = {v};")
        self.output.append(f"  mov {d} {self._stack_ref(self.current_offset)}, {v}")

    def visit_RETURN(self, node):
        val = node.value if node.value else "0"
        self.output.append(f"  mov eax, {val}")

    def visit_CALL(self, node):
        name = node.value['name']
        self.output.append(f"  ; call {name}({', '.join(node.value['args'])})")
        self.output.append(f"  call {name}")

    def visit_WHILE(self, node):
        lbl = self.label_counter
        self.label_counter += 1
        self.output.append(f".while_{lbl}:")
        # placeholder infinite/simple loop behaviour
        for child in node.children:
            self.visit(child)
        self.output.append(f"  jmp .while_{lbl}")