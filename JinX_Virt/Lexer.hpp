#include <vector>
#include <string>

enum TokenType {
    T_INT,      // 0
    T_CHAR,     // 1
    T_BOOL,     // 2
    T_STR,      // 3
    T_FLOAT,    // 4
    T_VOID,     // 5
    T_IF,       // 6
    T_WHILE,    // 7
    T_RETURN,   // 8
    T_IDENT,    // 9
    T_NUM,      // 10
    T_EQ,       // 11
    T_SEMI_COLON, // 12
    T_LP,       // 13
    T_RP,       // 14
    T_LSB,      // 15
    T_RSB,      // 16
    T_LCB,      // 17
    T_RCB,      // 18
    T_EOF,       // 19
    T_STRING_LITERAL // 20
};

struct Token {
    TokenType Type;
    std::string Value;
};

std::vector<Token> Lexer(const std::string& Source) {
    std::vector<Token> Tokens;
    size_t Iteration = 0;

    while (Iteration< Source.size()) {
        char Character = Source[Iteration];

        if (isspace(Character)) { Iteration++; continue; }

        if (isalpha(Character)) {
            std::string Word;
            while (Iteration < Source.size() && isalnum(Source[Iteration])) {
                Word += Source[Iteration++];
            }

            if (Word == "int") Tokens.push_back({T_INT, Word});
            else if (Word == "char") Tokens.push_back({T_CHAR, Word});
            else if (Word == "bool") Tokens.push_back({T_BOOL, Word});
            else if (Word == "str") Tokens.push_back({T_STR, Word});
            else if (Word == "float") Tokens.push_back({T_FLOAT, Word});
            else if (Word == "void") Tokens.push_back({T_VOID, Word});
            else if (Word == "if") Tokens.push_back({T_IF, Word});
            else if (Word == "while") Tokens.push_back({T_WHILE, Word});
            else if (Word == "return") Tokens.push_back({T_RETURN, Word});
            else Tokens.push_back({T_IDENT, Word});
            continue;
        }

        if (isdigit(Character)) {
            std::string Number;
            while (Iteration < Source.size() && isdigit(Source[Iteration])) {
                Number += Source[Iteration++];
            }
            Tokens.push_back({T_NUM, Number});
            continue;
        }

        if (Character == '=') { Tokens.push_back({T_EQ, "="}); Iteration++; continue; }
        if (Character == ';') { Tokens.push_back({T_SEMI_COLON, ";"}); Iteration++; continue; }
        if (Character == '(') { Tokens.push_back({T_LP, "("}); Iteration++; continue; }
        if (Character == ')') { Tokens.push_back({T_RP, ")"}); Iteration++; continue; }
        if (Character== '[') { Tokens.push_back({T_LSB, "["}); Iteration++; continue; }
        if (Character == ']') { Tokens.push_back({T_RSB, "]"}); Iteration++; continue; }
        if (Character == '{') { Tokens.push_back({T_LCB, "{"}); Iteration++; continue; }
        if (Character == '}') { Tokens.push_back({T_RCB, "}"}); Iteration++; continue; }

        if (Character == '"') {
            Iteration++;
            std::string String;
            while (Iteration < Source.size() && Source[Iteration] != '"') {
                if (Source[Iteration] == '\\' && Iteration + 1 < Source.size()) {
                    Iteration++;
                    switch (Source[Iteration]) {
                        case 'n': String += '\n'; break;
                        case 't': String += '\t'; break;
                        case '\\': String += '\\'; break;
                        case '"': String += '"'; break;
                        default: String += Source[Iteration]; break;
                    }
                    Iteration++;
                } else {
                    String += Source[Iteration++];
                }
            }

            Iteration++;

            Tokens.push_back({T_STRING_LITERAL, String});
            continue;
        }

        Iteration++;
    }

    Tokens.push_back({T_EOF, ""});
    return Tokens;
}