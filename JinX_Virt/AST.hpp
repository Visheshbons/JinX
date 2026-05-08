#pragma once

#include <string>
#include <vector>
#include <variant>
#include <optional>

enum NodeType {
    PROGRAM,
    VAR_DECLARATION,
    BINARY_OP,
    NUMBER,
    STRING,
    IDENTIFIER,
    PRINTLN,
    ASSIGNMENT,
    NONE
};

struct StringData {
    std::string Label;
    std::string Value;
};

extern std::vector<StringData> Strings;

class ASTNode {
public:
    NodeType Type;
    std::string Value;
    std::vector<ASTNode> Children;
    std::variant<int, double, std::string> Literal;

    ASTNode(NodeType SomeType) : Type(SomeType) {}
    ASTNode(NodeType SomeType, const std::string& SomeValue) : Type(SomeType), Value(SomeValue) {}
    ASTNode(NodeType SomeType, int SomeValue) : Type(SomeType), Literal(SomeValue) {}
};

struct Symbol {
    std::string Name;
    std::string Type;
    int Address;
    std::string Label;
};

class SymbolTable {
private:
    std::vector<Symbol> Symbols;
    int NextAddress = 0x2000;
    int VariableCounter = 0;
    
public:
    void Add(const std::string& Name, const std::string& Type) {
        std::string Label = "VAR" + std::to_string(VariableCounter++);
        Symbol Added{Name, Type, NextAddress, Label};
        Symbols.push_back(Added);
        if (Type == "str") Strings.push_back({Label, Name});
        NextAddress += 4;
    }
    
    std::optional<Symbol> Find(const std::string& Name) {
        for (auto& Symbol : Symbols) {
            if (Symbol.Name == Name) {
                return Symbol;
            }
        }
        return std::nullopt;
    }
    
    void Clear() {
        Symbols.clear();
        NextAddress = 0x2000;
        VariableCounter = 0;
    }
};