#pragma once

#include "AST.hpp"
#include "Lexer.hpp"
#include <vector>
#include <iostream>
#include <fstream>

extern std::vector<StringData> Strings;

class Parser {
public:
    std::vector<Token> Tokens;
    size_t Position = 0;
    SymbolTable Symbols;
    
    Token Current() { return Tokens[Position]; }
    void Advance() { Position++; }
    
    bool Match(TokenType Type) {
        if (Current().Type == Type) {
            Advance();
            return true;
        }
        return false;
    }
    
    void Expect(TokenType Type, const std::string& Error) {
        if (Current().Type != Type) {
            std::cerr << "Error: " << Error << std::endl;
            exit(1);
        }
        Advance();
    }

    Parser(const std::vector<Token>& SomeTokens) : Tokens(SomeTokens) {}
    
    ASTNode Parse() {
        ASTNode Program(PROGRAM);
        
        while (Current().Type != TokenType::T_EOF) {
            if (Current().Type == TokenType::T_INT || 
                Current().Type == TokenType::T_STR) {
                Program.Children.push_back(ParseVariableDeclaration());
            } else if (Current().Value == "println") {
                Program.Children.push_back(ParsePrintLine());
            } else {
                Advance();
            }
        }
        
        return Program;
    }

    void PrintAST(const ASTNode& Node, int Depth = 0) {
        std::string Indent(Depth * 2, ' ');
        
        std::cout << Indent;
        switch (Node.Type) {
            case PROGRAM: std::cout << "PROGRAM"; break;
            case VAR_DECLARATION: std::cout << "VARIABLE: " << Node.Value; break;
            case BINARY_OP: std::cout << "OPERATION: " << Node.Value; break;
            case NUMBER: std::cout << "NUMBER: " << Node.Value; break;
            case IDENTIFIER: std::cout << "IDENTIFIER: " << Node.Value; break;
            case STRING: std::cout << Indent << "STRING: " << Node.Value << std::endl; break;
            case PRINTLN: std::cout << "PRINTLINE"; break;
            default: std::cout << "UNKNOWN";
        }
        std::cout << std::endl;
        
        for (const auto& Child : Node.Children) {
            PrintAST(Child, Depth + 1);
        }
    }

    void GenerateCode(const ASTNode& Node, std::ofstream& Output, std::vector<StringData>& Strings) {
        switch (Node.Type) {
            case PROGRAM: {
                for (const auto& Child : Node.Children) {
                    GenerateCode(Child, Output, Strings);
                }
                Output << "HALT" << std::endl;
                
                for (const auto& String : Strings) {
                    Output << String.Label << ":" << std::endl;
                    Output << "DB ";
                    for (size_t i = 0; i < String.Value.size(); i++) {
                        Output << "'" << String.Value[i] << "'";
                        if (i < String.Value.size() - 1) Output << ", ";
                    }
                    Output << ", 0" << std::endl;
                }
                break;
            }
            
            case VAR_DECLARATION: {
                GenerateCode(Node.Children[0], Output, Strings);
                auto NodeSymbol = Symbols.Find(Node.Value);
                if (NodeSymbol.has_value()) {
                    Output << "WRITE_MEM " << NodeSymbol->Address << ", R0" << std::endl;
                }
                break;
            }
            
            case BINARY_OP: {
                GenerateCode(Node.Children[0], Output, Strings);
                Output << "PUSH R0" << std::endl;
                GenerateCode(Node.Children[1], Output, Strings);
                Output << "POP R1" << std::endl;
                if (Node.Value == "+") Output << "ADD R0, R1" << std::endl;
                if (Node.Value == "-") Output << "SUB R1, R0" << std::endl;
                break;
            }
            
            case NUMBER: {
                Output << "MOV32 R0, " << Node.Value << std::endl;
                break;
            }
            
            case IDENTIFIER: {
                auto NodeSymbol = Symbols.Find(Node.Value);
                if (NodeSymbol.has_value()) {
                    Output << "READ_MEM " << NodeSymbol->Address << ", R0" << std::endl;
                }
                break;
            }
            
            case STRING: {
                static int StringCounter = 0;
                std::string Label = "STR_" + std::to_string(StringCounter++);
                Strings.push_back({Label, Node.Value});
                Output << "OB " << Label << std::endl;
                break;
            }
            
            case PRINTLN: {
                ASTNode Expression = Node.Children[0];
    
                if (Expression.Type == STRING) {
                    GenerateCode(Expression, Output, Strings); 
                } else {
                    GenerateCode(Expression, Output, Strings);
                    Output << "INTOUT R0" << std::endl;
                }

                Output << "MOV8 R0, 10" << std::endl;
                Output << "CHAROUT R0" << std::endl;
                break;
            }
            
            default:
                break;
        }
    }
private:
    ASTNode ParseVariableDeclaration() {
        TokenType Type = Current().Type;
        Advance();
        
        std::string Name = Current().Value;
        Expect(TokenType::T_IDENT, "expected variable name");
        Expect(TokenType::T_EQ, "expected character =");
        
        ASTNode Expression = ParseExpression();
        
        Expect(TokenType::T_SEMI_COLON, "expected character ;");
        
        std::string TypeString = (Type == TokenType::T_INT) ? "int" : "str";
        Symbols.Add(Name, TypeString);
        
        ASTNode VariableNode(VAR_DECLARATION, Name);
        VariableNode.Children.push_back(Expression);
        
        return VariableNode;
    }
    
    ASTNode ParsePrintLine() {
        Advance();
        Expect(TokenType::T_LP, "expected character (");

        ASTNode Expression = ParseExpression();
        std::cerr << "Note: received expression " << Expression.Value << std::endl;
        
        Expect(TokenType::T_RP, "expected character )");
        Expect(TokenType::T_SEMI_COLON, "expected character ;");
        
        ASTNode PrintLineNode(PRINTLN, "println");
        PrintLineNode.Children.push_back(Expression);
        
        return PrintLineNode;
    }
    
    ASTNode ParseExpression() {
        ASTNode LeftTerm = ParseTerm();
        
        while (Current().Type == TokenType::T_ADD || Current().Type == TokenType::T_SUB) {
            char Operation = Current().Value[0];
            Advance();
            ASTNode RightTerm = ParseTerm();
            
            ASTNode Binary(BINARY_OP, std::string(1, Operation));
            Binary.Children.push_back(LeftTerm);
            Binary.Children.push_back(RightTerm);
            LeftTerm = Binary;
        }
        
        return LeftTerm;
    }
    
    ASTNode ParseTerm() {
        ASTNode LeftTerm = ParseFactor();
        
        while (Current().Type == TokenType::T_MUL || Current().Type == TokenType::T_DIV) {
            char Operation = Current().Value[0];
            Advance();
            ASTNode RightTerm = ParseFactor();
            
            ASTNode Binary(BINARY_OP, std::string(1, Operation));
            Binary.Children.push_back(LeftTerm);
            Binary.Children.push_back(RightTerm);
            LeftTerm = Binary;
        }
        
        return LeftTerm;
    }
    
    ASTNode ParseFactor() {
        if (Current().Type == TokenType::T_NUM) {
            ASTNode Node(NUMBER, Current().Value);
            Advance();
            return Node;
        }
        
        if (Current().Type == TokenType::T_IDENT) {
            ASTNode Node(IDENTIFIER, Current().Value);
            Advance();
            return Node;
        }
        
        if (Current().Type == TokenType::T_STRING_LITERAL) {
            ASTNode Node(STRING, Current().Value);
            Advance();
            return Node;
        }
        
        if (Current().Type == TokenType::T_LP) {
            Advance();
            ASTNode Expression = ParseExpression();
            Expect(TokenType::T_RP, "Expected )");
            return Expression;
        }
        
        std::cerr << "Error: unexpected token named " << Current().Value << std::endl;
        return ASTNode(PROGRAM);
    }
};