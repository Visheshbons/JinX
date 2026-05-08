#include <iostream>
#include <string>
#include "Parser2.hpp"

size_t Position = 0;
std::ofstream StringWriter;
std::vector<StringData> Strings;   

int main(int ArgumentC, char* ArgumentV[]) {
    if (ArgumentC < 2) {
        std::cerr << "Usage: jcc <input.jc>" << std::endl;
        return 1;
    }
    
    std::ifstream File(ArgumentV[1]);
    std::string Content((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
    std::string_view Input{std::move(Content)};
    
    auto Tokens = Lexer(Input);
    StringWriter.open("output.ja");

    Parser Compiler(Tokens);
    ASTNode Program = Compiler.Parse();
    Compiler.PrintAST(Program);
    Compiler.GenerateCode(Program, StringWriter, Strings);
    StringWriter.close();
}