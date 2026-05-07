#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "Lexer.hpp"

struct Variable {
    std::string Name;
    std::string JASMLabel;
    std::string Type;
    int NumberValue;
    std::string StringValue = ""; // Unless the variable is of the string or char type, then set NumberValue = 0 and assign to StringValue
};

std::vector<Variable> Variables;
int VariableCounter = 0;

extern std::ofstream StringWriter;

void ParseDeclaration(std::vector<Token>& Tokens, size_t& Position) {
    if (Tokens[Position].Type < 0 || Tokens[Position].Type > 5) {
        std::cerr << "Error: Missing type for variable declaration" << std::endl;
        return;
    }
    auto Type = Tokens[Position].Type;
    Position++;

    if (Tokens[Position].Type != T_IDENT) {
        std::cerr << "Error: Missing identifier for variable declaration" << std::endl;
        return;
    }

    std::string Name = Tokens[Position].Value;
    Position++;

    if (Tokens[Position].Type != T_EQ) {
        std::cerr << "Error: Missing assignment (=) operator for variable declaration" << std::endl;
        return;
    }
    Position++;

    if (Tokens[Position].Type != T_NUM && Tokens[Position].Type != T_STRING_LITERAL) {
        std::cerr << "Error: Invalid argument for variable declaration" << std::endl;
        return;
    }
    auto Value = Tokens[Position].Value;
    Position++;


    if (Tokens[Position].Type != T_SEMI_COLON) {
        std::cerr << "Error: Missing period (;) operator for variable declaration" << std::endl;
        return;
    }
    Position++;

    if (Type == T_INT) {
        std::string Label = "VAR" + std::to_string(VariableCounter++);

        StringWriter << "MOV32 R0, " << Value << std::endl;
        StringWriter << "WRITE_MEM " << Label << ", R0" << std::endl;

        Variables.push_back({Name, Label, "int", std::stoi(Value), ""});
    }

    if (Type == T_STR) {
        std::string Label = "VAR" + std::to_string(VariableCounter++);
        
        StringWriter << Label << ":" << std::endl;
        StringWriter << "    DB ";
        for (char Character : Value) {
            StringWriter << "'" << Character << "', ";
        }
        StringWriter << "0" << std::endl;
        
        Variables.push_back({Name, Label, "str", 0, Value});
    }

    std::cout << Name << " is declared successfully" << std::endl;
}