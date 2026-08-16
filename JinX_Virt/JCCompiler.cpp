#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cctype>
#include <sstream>
#include <fstream>

struct Token {
    std::string Value;
    std::string Type;
};

const char EOL_SENTINEL = '\0';

class Tokenizer {
private:
    std::string Line;
    size_t Position = 0;
    std::vector<Token> Tokens;
    std::string Current;

    char Peek(int Offset = 0) const {
        size_t Index = Position + Offset;
        if (Index < Line.length()) {
            return Line[Index];
        }
        return EOL_SENTINEL;
    }

    char Advance() {
        if (Position < Line.length()) {
            return Line[Position++];
        }
        return EOL_SENTINEL;
    }

    bool IsEndOfLine() const {
        return Position >= Line.length() || Peek() == EOL_SENTINEL;
    }

    size_t Remaining() const {
        if (Position < Line.length()) {
            return Line.length() - Position;
        }
        return 0;
    }

    std::string PeekString(size_t Length) const {
        std::string Result;
        for (size_t i = 0; i < Length && Peek(i) != EOL_SENTINEL; i++) {
            Result += Peek(i);
        }
        return Result;
    }

    void FlushCurrent() {
        if (!Current.empty()) {
            Tokens.push_back({Current, "identifier"});
            Current.clear();
        }
    }

    bool IsKeyword(const std::string& token) const {
        static const std::vector<std::string> Keywords = {
            "id", "int", "float", "double", "char", "void",
            "short", "long", "unsigned",
            "if", "else", "do", "for", "while", "continue", "break",
            "return", "const", "auto", "static",
            "enum", "typedef", "struct", "class",
            "sizeof", "nocontainer", "literal",
            "typeof", "litreval", "typeeval",
            "using", "nullptr", "true", "false",
            "compiler_error", "compile_error", "realloc", "asm"
        };
        for (const auto& k : Keywords) {
            if (token == k) return true;
        }
        return false;
    }

    void GetInlineAssembly(Token& token) { 
        if (token.Value == "asm") {
            auto c = Peek();
            if (c == '(') {
                Tokens.push_back({"(", "punctuation"});
                Advance();
                c = Peek();
                while (c != ')') {
                    if (c == '"') {
                        Advance();
                        std::string StringLiteral;
                        while (!IsEndOfLine() && Peek() != '"') {
                            if (Peek() == '\\') {
                                StringLiteral += Advance();
                                if (!IsEndOfLine()) StringLiteral += Advance();
                                continue;
                            }
                            StringLiteral += Advance();
                        }
                        if (Peek() == '"') {
                            Advance();
                        }
                        Tokens.push_back({StringLiteral, "asm_string"});
                    } else if (c == ',') {
                        Advance();
                        Tokens.push_back({",", "punctuation"});
                    } else if (c == ' ' || c == '\t') {
                        Advance();
                    } else {
                        Advance();
                    }
                    c = Peek();
                }
                if (c == ')') {
                    Advance();
                    Tokens.push_back({")", "punctuation"});
                }
            }
        }
    }

    bool IsNumber(const std::string& token) const {
        if (token.empty()) return false;
        for (char c : token) {
            if (!isdigit(static_cast<unsigned char>(c))) return false;
        }
        return true;
    }

    bool IsPunctuation(char c) const {
        return c == ';' || c == '(' || c == ')' || c == '{' || c == '}' ||
            c == '[' || c == ']' || c == ',' || c == ':' || c == '=' ||
            c == '*' || c == '&' || c == '<' || c == '>' || c == '#' ||
            c == '?' || c == '.' || c == '!' || c == '|' || c == '+';
    }

    bool IsMultiCharOperator() {
        std::string Next2 = PeekString(2);
        std::string Next3 = PeekString(3);
        
        if (Next2 == "==" || Next2 == "!=" || Next2 == "<=" || Next2 == ">=" || Next2 == "&&" || Next2 == "||" || Next2 == "++" || Next2 == "--") return true;
        return false;
    }

public:
    Tokenizer(const std::string& line) : Line(line) {}

    std::vector<Token> Tokenize() {
        Tokens.clear();
        Position = 0;
        Current.clear();

        while (!IsEndOfLine()) {
            char c = Peek();
            bool IsInType = false;

            if (c == '#') {
                FlushCurrent();
                std::string Directive;
                while (!IsEndOfLine() && Peek() != '\n') {
                    if (Peek() == '#' && !Directive.empty()) {
                        break;
                    }
                    Directive += Advance();
                }
                Tokens.push_back({Directive, "preprocessor"});
                continue;
            }

            if (c == 'o' && PeekString(8) == "operator") {
                if (Peek(8) == '"' && Peek(9) == '"') {
                    FlushCurrent();
                    for (int i = 0; i < 10; i++) Advance();
                    Tokens.push_back({"operator\"\"", "operator"});
                    continue;
                }
                if (Peek(8) == '{' && Peek(9) == '}') {
                    FlushCurrent();
                    for (int i = 0; i < 10; i++) Advance();
                    Tokens.push_back({"operator{}", "operator"});
                    continue;
                }
                if (Peek(8) == '[' && Peek(9) == ']') {
                    FlushCurrent();
                    for (int i = 0; i < 10; i++) Advance();
                    Tokens.push_back({"operator[]", "operator"});
                    continue;
                }
                FlushCurrent();
                for (int i = 0; i < 8; i++) Advance();
                Tokens.push_back({"operator", "keyword"});
                continue;
            }

            if (c == '[') {
                FlushCurrent();
                
                bool IsArrayContext = false;
                
                if (!Tokens.empty()) {
                    std::string LastValue = Tokens.back().Value;
                    
                    if (LastValue == "id" || LastValue == "char" || LastValue == "int" || LastValue == "float" || LastValue == "double" || LastValue == "t" || LastValue == "str" || LastValue == "bool" || LastValue == "void") {
                        IsArrayContext = true;
                    }
                    
                    if (Tokens.size() >= 2) {
                        if (Tokens.back().Type == "identifier" && Tokens[Tokens.size() - 2].Value == "*") {
                            IsArrayContext = true;
                        }
                    }
                }
                
                Advance();
                
                if (Peek() == '&' && Peek(1) == ']') {
                    Advance();
                    Advance();
                    if (!IsArrayContext) {
                        Tokens.push_back({"[&]", "lambda"});
                    } else {
                        Tokens.push_back({"[", "punctuation"});
                        Tokens.push_back({"&", "punctuation"});
                        Tokens.push_back({"]", "punctuation"});
                    }
                    continue;
                }
                
                if (Peek() == '=' && Peek(1) == ']') {
                    Advance();
                    Advance();
                    if (!IsArrayContext) {
                        Tokens.push_back({"[=]", "lambda"});
                    } else {
                        Tokens.push_back({"[", "punctuation"});
                        Tokens.push_back({"&", "punctuation"});
                        Tokens.push_back({"]", "punctuation"});
                    }
                    continue;
                }
                
                if (Peek() == ']') {
                    Advance();
                    if (!IsArrayContext) {
                        Tokens.push_back({"[]", "lambda"});
                    } else {
                        Tokens.push_back({"[", "punctuation"});
                        Tokens.push_back({"]", "punctuation"});
                    }
                    continue;
                }
                
                Tokens.push_back({"[", "punctuation"});
                continue;
            }

            if (IsMultiCharOperator()) {
                FlushCurrent();
                std::string Op;
                Op += Advance();
                Op += Advance();
                Tokens.push_back({Op, "operator"});
                continue;
            }

            if (c == '"') {
                FlushCurrent();
                Advance();
                std::string StringLiteral;
                StringLiteral += '"';
                while (!IsEndOfLine() && Peek() != '"') {
                    if (Peek() == '\\') {
                        StringLiteral += Advance();
                        if (!IsEndOfLine()) StringLiteral += Advance();
                        continue;
                    }
                    StringLiteral += Advance();
                }
                if (Peek() == '"') {
                    StringLiteral += Advance();
                }
                Tokens.push_back({StringLiteral, "string"});
                continue;
            }

            if (c == '\'') {
                FlushCurrent();
                Advance();
                std::string CharLiteral;
                CharLiteral += '\'';
                while (!IsEndOfLine() && Peek() != '\'') {
                    if (Peek() == '\\') {
                        CharLiteral += Advance();
                        if (!IsEndOfLine()) CharLiteral += Advance();
                        continue;
                    }
                    CharLiteral += Advance();
                }
                if (Peek() == '\'') {
                    CharLiteral += Advance(); 
                }
                Tokens.push_back({CharLiteral, "character"});
                continue;
            }

            if (c == '0' && Peek(1) == 'x') {
                FlushCurrent();
                std::string Hex;
                Hex += Advance();
                Hex += Advance();
                while (!IsEndOfLine() && isxdigit(static_cast<unsigned char>(Peek()))) {
                    Hex += Advance();
                }
                Tokens.push_back({Hex, "number"});
                continue;
            }

            if (c == '0' && Peek(1) == 'b') {
                FlushCurrent();
                std::string Binary;
                Binary += Advance();
                Binary += Advance();
                while (!IsEndOfLine() && (Peek() == '0' || Peek() == '1')) {
                    Binary += Advance();
                }
                Tokens.push_back({Binary, "number"});
                continue;
            }

            if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
                std::string Identifier;
                while (Position < Line.length() && 
                    (isalnum(static_cast<unsigned char>(Line[Position])) || 
                        Line[Position] == '_')) {
                    Identifier += Line[Position];
                    Position++;
                }
                Tokens.push_back({Identifier, "identifier"});
                Current.clear();
                continue;
            }

            if (isdigit(static_cast<unsigned char>(c))) {
                std::string Number;
                while (Position < Line.length() && 
                    isdigit(static_cast<unsigned char>(Line[Position]))) {
                    Number += Line[Position];
                    Position++;
                }
                
                if (Position < Line.length() && 
                    (isalpha(static_cast<unsigned char>(Line[Position])) || Line[Position] == '_')) {
                    while (Position < Line.length() && 
                        (isalnum(static_cast<unsigned char>(Line[Position])) || 
                            Line[Position] == '_')) {
                        Number += Line[Position];
                        Position++;
                    }
                    Tokens.push_back({Number, "identifier"});
                } else {
                    Tokens.push_back({Number, "number"});
                }
                Current.clear();
                continue;
            }

            if (isdigit(static_cast<unsigned char>(c))) {
                std::string Number;
                bool HasDot = false;
                bool HasExponent = false;

                while (!IsEndOfLine()) {
                    char ch = Peek();
                    if (isdigit(static_cast<unsigned char>(ch))) {
                        Number += Advance();
                    } else if (ch == '.') {
                        if (HasDot) break;
                        HasDot = true;
                        Number += Advance();
                    } else if (ch == 'e' || ch == 'E') {
                        HasExponent = true;
                        Number += Advance();
                        if (Peek() == '+' || Peek() == '-') {
                            Number += Advance();
                        }
                    } else if (ch == 'f' || ch == 'F' || ch == 'd' || ch == 'D') {
                        Number += Advance();
                        break;
                    } else {
                        break;
                    }
                }
                Tokens.push_back({Number, (HasDot || HasExponent) ? "float" : "number"});
                continue;
            }

            if (c == '/' && Peek(1) == '/') {
                break;
            }

            if (isspace(static_cast<unsigned char>(c))) {
                FlushCurrent();
                Advance();
                continue;
            }

            if (IsPunctuation(c)) {
                FlushCurrent();
                Tokens.push_back({std::string(1, Advance()), "punctuation"});
                continue;
            }

            Current += Advance();
        }

        FlushCurrent();

        static Token* ASMKeyword = nullptr;
        for (auto& t : Tokens) {
            if (IsKeyword(t.Value)) {
                t.Type = "keyword";
                ASMKeyword = (t.Value == "asm") ? nullptr : &t;
            } else if (IsNumber(t.Value)) {
                t.Type = "number";
            }
        }

        if (ASMKeyword != nullptr) GetInlineAssembly(*ASMKeyword);

        return Tokens;
    }
};

std::vector<std::string> GetLines(const std::string& source) {
    std::vector<std::string> Lines;
    std::stringstream Stream(source);
    std::string Line;
    while (std::getline(Stream, Line)) {
        if (!Line.empty() && Line.back() == '\r') {
            Line.pop_back();
        }
        Lines.push_back(Line);
    }
    return Lines;
}

std::vector<std::string> GetLines(const char* filepath) {
    std::vector<std::string> Lines;
    std::ifstream IStream(filepath);
    std::string Line;
    
    while (std::getline(IStream, Line)) {
        Lines.push_back(Line);
    }
    
    return Lines;
}

int main() {
    auto Lines = GetLines("__stl_def.jc");

    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    const std::string GRAY    = "\033[90m";

    for (size_t i = 0; i < Lines.size(); i++) {
        if (Lines[i].empty()) {
            std::cout << std::endl;
            continue;
        }

        Tokenizer tokenizer(Lines[i]);
        auto Tokens = tokenizer.Tokenize();

        std::cout << GRAY << "[" << std::setw(2) << i << "] " << RESET;

        for (const auto& t : Tokens) {
            std::string Color;
            std::string Style = "";

            if (t.Type == "keyword") {
                Color = MAGENTA;
                Style = BOLD;
            } else if (t.Type == "identifier") {
                Color = WHITE;
            } else if (t.Type == "number" || t.Type == "float") {
                Color = RED;
            } else if (t.Type == "string") {
                Color = GREEN;
            } else if (t.Type == "character") {
                Color = CYAN;
            } else if (t.Type == "operator") {
                Color = YELLOW;
                Style = BOLD;
            } else if (t.Type == "punctuation") {
                Color = YELLOW;
            } else if (t.Type == "preprocessor") {
                Color = CYAN;
            } else if (t.Type == "lambda") {
                Color = BLUE;
            } else {
                Color = WHITE;
            }

            std::cout << Style << Color << t.Value << RESET << " ";
        }

        std::cout << std::endl;
    }

    return 0;
}