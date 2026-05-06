#include <vector>
#include <algorithm>
#include <string>

enum TokenType {
    T_INT, T_CHAR, T_BOOL, T_STR, T_FLOAT, T_IDENT, T_NUM, T_EQ, T_LB, T_RB, T_L_SQB, T_R_SQB, T_SEMI_COLON, T_EOF
};

struct Token {
    TokenType Type;
    std::string Value;
};

std::vector<Token> Lexer(std::string_view& Source) {
    std::vector<Token> Tokens;
    size_t Iteration = 0;

    while (Iteration < Source.size()) {
        char Character = Source[Iteration];

        if (isspace(Character)) Iteration++; continue;
        if (isalpha(Character)) {
            std::string Word;
            while (isalnum(Source[Iteration]))  Word += Source[Iteration++];

            if (Word == "int") Tokens.push_back({T_INT, Word});
            else if (Word == "char") Tokens.push_back({T_CHAR, Word});
            else if (Word == "bool") Tokens.push_back({T_BOOL, Word});
            else if (Word == "str") Tokens.push_back({T_STR, Word});
            else if (Word == "float") Tokens.push_back({T_FLOAT, Word});
        } else if (isdigit(Character)) {
            std::string Number;
            while (isnumber(Source[Iteration])) Number += Source[Iteration++];
            Tokens.push_back({T_NUM, Number});
        } else if (Character == '=') {
            Tokens.push_back({T_EQ, "="});
            Iteration++;
        } else if (Character == ';') {
            Tokens.push_back({T_SEMI_COLON, ";"});
            Iteration++;
        } else {
            Iteration++;
        }
    }

    Tokens.push_back({T_EOF, ""});
    return Tokens;
}