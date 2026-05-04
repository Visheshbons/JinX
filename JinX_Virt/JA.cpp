#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

std::map<std::string, uint8_t> OperationCodes = {
    {"HALT", 0x00},
    {"MOV8", 0x01},
    {"MOV32", 0x02},
    {"ADD", 0x10},
    {"SUB", 0x11},
    {"ADDI", 0x12},
    {"JUMP", 0x20},
    {"JUMP_IF_ZERO", 0x21},
    {"NOT", 0x22},
    {"OUTPUT", 0x30},
    {"WRITE", 0x40},
    {"READ", 0x41},
    {"READ_KEY", 0x60},
    {"PUSH", 0x70},
    {"POP", 0x71},
    {"CALL", 0x72},
    {"RETURN", 0x73},
    {"CMP", 0x80},
    {"JUMP_IF_EQ", 0x81},
    {"JUMP_IF_LT", 0x82},
    {"JUMP_IF_GT", 0x83},
    {"LEA", 0x90},
    {"DB", 0x91}
};

std::vector<uint8_t> Bytecode;
std::map<std::string, uint32_t> Labels;
std::vector<std::string> SourceLines;

void WriteByte(uint8_t Bit) {
    Bytecode.push_back(Bit);
}

void WriteByte32(uint32_t Value) {
    WriteByte(Value & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 24) & 0xFF);
}

int ParseRegister(const std::string& String) {
    if (String.empty() || String[0] != 'R') {
        std::cerr << "Error: expected register R0 to R7, received " << String << "" << std::endl;
        return 0;
    }
    return std::stoi(String.substr(1));
}

char ParseCharacterLiteral(std::string& String) {
    if (String[1] == '\\') {
        switch (String[2]) {
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case '\\': return '\\';
            case '\'': return '\'';
            default: return String[2];
        }
    }

    return (unsigned char)String[1];
}

int ParseValue(std::string& ValueString) {
    if (ValueString.size() >= 3 && ValueString.front() == '\'' && ValueString.back() == '\'') {
        if (ValueString.size() == 3) {
            return (unsigned char)ValueString[1];
        }
        ParseCharacterLiteral(ValueString);
    }

    if (ValueString.size() >= 3 && ValueString[0] == '0' && ValueString[1] == 'x') {
        return std::stoul(ValueString, nullptr, 16);
    }

    return std::stoi(ValueString);
}

int ParseNumber(char* String) {
    int Result = 0;
    while (*String >= '0' && *String <= '9') {
        Result = Result * 10 + (*String - '0');
        String++;
    }

    return Result;
}

char FindOperator(char* Expression) {
    std::string String = static_cast<std::string>(Expression);

    if (String.find('+')) {
        return '+';
    } else if (String.find('-')) {
        return '-';
    } else if (String.find('*')) {
        return '*';
    } else if (String.find('/')) {
        return '/';
    }

    return '+';
}

int Evaluate(char* Expression) {
    int LHS = ParseNumber(Expression);
    char Operation = FindOperator(Expression);
    int RHS = ParseNumber(Expression);

    switch (Operation) {
        case '+': return LHS + RHS;
        case '-': return LHS - RHS;
        case '*': return LHS * RHS;
        case '/': return LHS / RHS;
    }
    
    return -1;
}

std::string Trim(std::string String) {
    size_t Start = String.find_first_not_of(" \t");
    if (Start == std::string::npos) return "";
    size_t End = String.find_last_not_of(" \t");
    return String.substr(Start, End - Start + 1);
}

bool IsLabel(const std::string& Line) {
    return Line.size() >= 2 && Line.back() == ':';
}

std::string GetLabelName(const std::string& Line) {
    return Line.substr(0, Line.size() - 1);
}

void FirstPass(const std::vector<std::string>& Lines) {
    uint32_t Address = 0;

    for (auto& Line : Lines) {
        std::string Trimmed = Trim(Line);
        if (Trimmed.empty()) continue;
        if (Trimmed[0] == ';') continue;

        if (IsLabel(Trimmed)) {
            std::string Label = GetLabelName(Trimmed);
            if (Labels.find(Label) != Labels.end()) {
                std::cerr << "Error: Duplicate label of " << Label << " found" << std::endl;
            }
            Labels[Label] = Address;
            continue;
        }

        size_t Space = Trimmed.find(' ');
        std::string OperationCode = (Space == std::string::npos) ? Trimmed : Trimmed.substr(0, Space);

        if (OperationCode == "HALT") Address += 1;
        else if (OperationCode == "RETURN") Address += 1;
        else if (OperationCode == "NOT") Address += 1;
        else if (OperationCode == "OUTPUT") Address += 2;
        else if (OperationCode == "READ_KEY") Address += 2;
        else if (OperationCode == "PUSH" || OperationCode == "POP") Address += 2;
        else if (OperationCode == "DB") {
            std::string Values = (Space == std::string::npos) ? "" : Trim(Trimmed.substr(Space + 1));
    
            int ByteCount = 0;
            if (!Values.empty()) {
                size_t Position = 0;
                while ((Position = Values.find(',')) != std::string::npos) {
                    ByteCount++;
                    Values = Values.substr(Position + 1);
                }
                ByteCount++;
            }
            
            Address += 2 + ByteCount;
        } else if (OperationCode == "MOV8") Address += 3;
        else if (OperationCode == "ADD" || OperationCode == "SUB") Address += 3;
        else if (OperationCode == "CMP") Address += 3;
        else if (OperationCode == "ADDI") Address += 3;
        else if (OperationCode == "CALL") Address += 5;
        else if (OperationCode == "JUMP") Address += 5;
        else if (OperationCode == "JUMP_IF_EQ" || OperationCode == "JUMP_IF_LT" || OperationCode == "JUMP_IF_GT") Address += 5;
        else if (OperationCode == "JUMP_IF_ZERO") Address += 6;
        else if (OperationCode == "READ" || OperationCode == "WRITE") Address += 6;
        else if (OperationCode == "MOV32") Address += 6;
        else if (OperationCode == "LEA") Address += 6;
    }
}

void AssembleLine(const std::string& Line, uint32_t& PC) {
    std::string ReadLine = Line;

    size_t CommentPosition = ReadLine.find(';');
    if (CommentPosition != std::string::npos) {
        ReadLine = ReadLine.substr(0, CommentPosition);
    }

    ReadLine = Trim(ReadLine);
    if (ReadLine.empty()) return;

    if (ReadLine.empty()) return;
    if (ReadLine[0] == ';') return;

    if (IsLabel(ReadLine)) return;

    size_t Space = ReadLine.find(' ');
    std::string OperationCode = (Space == std::string::npos) ? ReadLine : ReadLine.substr(0, Space);
    std::string Rest = (Space == std::string::npos) ? "" : Trim(ReadLine.substr(Space + 1));

    if (OperationCode == "HALT") {
        WriteByte(OperationCodes["HALT"]);
        PC += 1;
    } else if (OperationCode == "RETURN") {
        WriteByte(OperationCodes["RETURN"]);
        PC += 1;
    } else if (OperationCode == "NOT") {
        WriteByte(OperationCodes["NOT"]);
        PC += 1;
    } else if (OperationCode == "OUTPUT") {
        int Register = ParseRegister(Rest);
        WriteByte(OperationCodes["OUTPUT"]);
        WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "READ_KEY") {
        int Register = ParseRegister(Rest);
        WriteByte(OperationCodes["READ_KEY"]);
        WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "MOV8") {
        size_t Comma = Rest.find(',');

        if (Comma == std::string::npos) {
            std::cerr << "Error: syntax requires <register>, <value>" << std::endl;
            return;
        }

        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string ValueString = Trim(Rest.substr(Comma + 1));

        int Register = ParseRegister(RegisterString);
        int Value = ParseValue(ValueString);

        if (Value < 0 || Value > 255) {
            std::cerr << "Error: value found as " << Value << " exceeds MOV8 limits (minimum value = 0, maxmium value = 255)" << std::endl;
            return; 
        }

        WriteByte(OperationCodes["MOV8"]);
        WriteByte(Register);
        WriteByte(Value);
        PC += 3;
    } else if (OperationCode == "CMP") {
        size_t Comma = Rest.find(',');
        if (Comma == std::string::npos) {
            std::cerr << "Error: CMP requires syntax <destination>, <source>" << std::endl;
            return;
        }
        
        std::string DestinationString = Trim(Rest.substr(0, Comma));
        std::string SourceString = Trim(Rest.substr(Comma + 1));
        
        int Destination = ParseRegister(DestinationString);
        int Source = ParseRegister(SourceString);
        
        WriteByte(OperationCodes["CMP"]);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "MOV32") {
        size_t Comma = Rest.find(',');

        if (Comma == std::string::npos) {
            std::cerr << "Error: syntax requires <register>, <value>" << std::endl;
            return;
        }

        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string ValueString = Trim(Rest.substr(Comma + 1));

        int Register = ParseRegister(RegisterString);
        int Value = ParseValue(ValueString);

        WriteByte(OperationCodes["MOV32"]);
        WriteByte(Register);
        WriteByte(Value);
        PC += 6;
    } else if (OperationCode == "ADD" || OperationCode == "SUB") {
        size_t Comma = Rest.find(',');

        if (Comma == std::string::npos) {
            std::cerr << "Error: " << OperationCode << " requires syntax <destination>, <source>" << std::endl;
            return;
        }

        std::string DestinationString = Trim(Rest.substr(0, Comma));
        std::string SourceString = Trim(Rest.substr(Comma + 1));

        int Destination = ParseRegister(DestinationString);
        int Source = ParseRegister(SourceString);

        WriteByte(OperationCodes[OperationCode]);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "ADDI") {
        size_t Comma = Rest.find(',');
        std::string DestinationString = Trim(Rest.substr(0, Comma));
        std::string ValueString = Trim(Rest.substr(Comma + 1));
        
        int Destination = ParseRegister(DestinationString);
        int Value = ParseValue(ValueString);
        
        WriteByte(OperationCodes["ADDI"]);
        WriteByte(Destination);
        WriteByte(Value);
        PC += 3;
    } else if (OperationCode == "PUSH" || OperationCode == "POP") {
        int Register = ParseRegister(Rest);
        WriteByte(OperationCodes[OperationCode]);
        WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "DB") {
        std::vector<int> Bytes;
        std::string Values = Rest;
        size_t Position = 0;
        
        while ((Position = Values.find(',')) != std::string::npos) {
            std::string Value = Trim(Values.substr(0, Position));
            Bytes.push_back(ParseValue(Value));
            Values = Values.substr(Position + 1);
        }

        std::string ValueTryParse = Trim(Values);

        if (!Values.empty()) {
            Bytes.push_back(ParseValue(ValueTryParse));
        }
        
        WriteByte(OperationCodes["DB"]);
        WriteByte(Bytes.size());
        for (int Byte : Bytes) {
            WriteByte(Byte & 0xFF);
        }
        PC += 2 + Bytes.size();
    } else if (OperationCode == "JUMP" || OperationCode == "JUMP_IF_EQ" || OperationCode == "JUMP_IF_LT" || OperationCode == "JUMP_IF_GT") {
        std::string AddressString = Trim(Rest);

        size_t CommentPosition = AddressString.find(';');
        if (CommentPosition != std::string::npos) {
            AddressString = Trim(AddressString.substr(0, CommentPosition));
        }

        uint32_t Address;
        if (AddressString.find_first_not_of("0123456789xXabcdefABCDEF") != std::string::npos) {
            if (Labels.find(AddressString) == Labels.end()) {
                std::cerr << "Error: unknown label of " << AddressString << std::endl;
                return;
            }
            Address = Labels[AddressString];
        } else {
            Address = ParseValue(AddressString);
        }

        WriteByte(OperationCodes[OperationCode]);
        WriteByte32(Address);
        PC += 5;
    } else if (OperationCode == "CALL") {
        std::string AddressString = Trim(Rest);
        uint32_t Address = (Labels.find(AddressString) != Labels.end()) ? Labels[AddressString] : ParseValue(AddressString);

        WriteByte(OperationCodes["CALL"]);
        WriteByte32(Address);
        PC += 5;
    } else if (OperationCode == "JUMP_IF_ZERO") {
        size_t CommentPosition = Rest.find(';');
        if (CommentPosition != std::string::npos) {
            Rest = Trim(Rest.substr(0, CommentPosition));
        }

        size_t Comma = Rest.find(',');

        if (Comma == std::string::npos) {
            std::cerr << "Error: syntax requires <register>, <address>" << std::endl;
            return;
        }

        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string AddressString = Trim(Rest.substr(Comma + 1));

        int Register = ParseRegister(RegisterString);

        uint32_t Address;
        if (AddressString.find_first_not_of("0123456789xXabcdefABCDEF") != std::string::npos) {
            if (Labels.find(AddressString) == Labels.end()) {
                std::cerr << "Error: unknown label of " << AddressString << std::endl;
                return;
            }
            Address = Labels[AddressString];
        } else {
            Address = ParseValue(AddressString);
        }

        WriteByte(OperationCodes["JUMP_IF_ZERO"]);
        WriteByte(Register);
        WriteByte32(Address);
        PC += 6;
    } else if (OperationCode == "WRITE" || OperationCode == "READ") {
        size_t Comma = Rest.find(',');

        if (Comma == std::string::npos) {
            std::cerr << "Error: " << OperationCode << "requires syntax <register>, <address>" << std::endl;
            return;
        }

        std::string AddressString = Trim(Rest.substr(0, Comma));
        std::string RegisterString = Trim(Rest.substr(Comma + 1));

        uint32_t Address = ParseValue(AddressString);
        int Register = ParseRegister(RegisterString);

        WriteByte(OperationCodes[OperationCode]);
        WriteByte32(Address);
        WriteByte(Register);
        PC += 6;
    } else if (OperationCode == "LEA") {
        size_t Comma = Rest.find(',');
        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string AddressString = Trim(Rest.substr(Comma + 1));

        int Register = ParseRegister(RegisterString);
        uint32_t Address = (Labels.find(AddressString) != Labels.end()) ? Labels[AddressString] : ParseValue(AddressString);

        WriteByte(OperationCodes["LEA"]);
        WriteByte(Register);
        WriteByte32(Address);
        PC += 6;
    }
}

int main(int ArguementC, char* ArguementV[]) {
    if (ArguementC < 2) {
        std::cout << "Syntax: jasm Input.jasm [output.bin]" << std::endl;
    }

    std::ifstream Input(ArguementV[1]);

    if (!Input) {
        std::cout << "Error: Cannot open " << ArguementV[1] << std::endl;
    }

    std::string Line;
    
    while (getline(Input, Line)) {
        SourceLines.push_back(Line);
    }
    Input.close();

    FirstPass(SourceLines);

    uint32_t PC = 0;
    Bytecode.clear();

    for (auto& Line : SourceLines) {
        AssembleLine(Line, PC);
    }

    std::string Outfile = (ArguementC > 2) ? ArguementV[2] : "kernel.jinx";
    std::ofstream Output(Outfile, std::ios::binary);
    if (!Bytecode.empty()) Output.write(reinterpret_cast<char*>(&Bytecode[0]), Bytecode.size());
    Output.close();

    std::cout << "Assembled " << Bytecode.size() << " bytes to " << Outfile << std::endl;
}