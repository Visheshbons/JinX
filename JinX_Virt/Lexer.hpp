#include <vector>
#include <algorithm>
#include <string>

enum TokenType {
    T_INT, T_IDENT, T_NUM, T_EQ, T_SEMI_COLON, T_EOF
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
            else Tokens.push_back({T_IDENT, Word}); 
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