class Generator:
    def __init__(self, ast):
        self.ast = ast
        self.output = []
        self.local_vars = {}  # Tracks variable names and their stack offsets
        self.current_offset = 0

    def generate(self):
        """Iterates through the root nodes of the AST."""
        for node in self.ast:
            self.visit(node)

        return "\n".join(self.output)

    def visit(self, node):
        """Directs the node to the correct handler based on its type."""
        method_name = f"visit_{node.node_type}"
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node):
        """Fallback for nodes we haven't implemented yet."""
        self.output.append(f"; WARNING: Unhandled node type: {node.node_type}")

    def visit_INCLUDE(self, node):
        # We just leave a comment in the JASM file
        self.output.append(f"; included {node.value}")

    def visit_FUNCTION_DECL(self, node):
        func_name = node.value["name"]

        # Reset stack offset and variables for the new function
        self.local_vars = {}
        self.current_offset = 0

        self.output.append(f"\nglobal {func_name}")
        self.output.append(f"{func_name}:")

        # Intel Function Prologue
        self.output.append("  push rbp")
        self.output.append("  mov rbp, rsp")

        # Generate code for the body of the function
        for child in node.children:
            self.visit(child)

        # Intel Function Epilogue
        self.output.append("  mov rsp, rbp")  # Restore stack pointer
        self.output.append("  pop rbp")  # Restore base pointer
        self.output.append("  ret")

    def visit_VAR_DECL(self, node):
        var_name = node.value["name"]
        var_val = node.value["init"]
        var_type = node.value["type"]

        # Calculate stack offset based on type
        # u32 is 4 bytes (dword). We subtract from the offset because the stack grows downwards.
        if var_type == "u32":
            self.current_offset -= 4
            size_directive = "dword"
        elif var_type == "u8" or var_type == "bool":
            self.current_offset -= 1
            size_directive = "byte"
        elif var_type == "ptr":
            self.current_offset -= 8  # 64-bit pointers
            size_directive = "qword"
        else:
            self.current_offset -= 8  # Default fallback
            size_directive = "qword"

        self.local_vars[var_name] = self.current_offset

        self.output.append(f"  ; {var_type} {var_name} = {var_val};")
        self.output.append(
            f"  mov {size_directive} [rbp{self.current_offset}], {var_val}"
        )
