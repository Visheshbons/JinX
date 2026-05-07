#include <iostream>
#include <string>
#include "Parser.hpp"

std::ofstream StringWriter;

int main(int ArgumentC, char* ArgumentV[]) {
    if (ArgumentC < 2) {
        std::cerr << "Usage: jcc <input.jc>" << std::endl;
        return 1;
    }
    
    std::ifstream File(ArgumentV[1]);
    std::string Content((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
    
    auto Tokens = Lexer(Content);
    
    StringWriter.open("output.ja");
    
    size_t Position = 0;
    while (Position < Tokens.size() && Tokens[Position].Type != T_EOF) {
        if (Tokens[Position].Type == T_INT || Tokens[Position].Type == T_STR) {
            ParseDeclaration(Tokens, Position);
        } else {
            Position++;
        }
    }
    
    StringWriter << "HALT" << std::endl;
    StringWriter.close();
}