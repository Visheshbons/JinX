#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cctype>
#include <sstream>
#include <optional>

bool DebugFlag = false;
bool HexFlag = false;

/* For reference

enum class OperationCode : uint8_t {
    //HALT = 0x00,
    //MOV8 = 0x01,
    //MOV32 = 0x02,
    //MOV = 0x03,
    //ADD = 0x10,
    //SUB = 0x11,
    //ADDI = 0x12,
    //JUMP = 0x20,
    //JUMP_IF_ZERO = 0x21,
    //NOT = 0x22,
    //INTOUT = 0x30,
    //CHAROUT = 0x31,
    //OB = 0x32,
    //REALOUT = 0x33,
    //WRITE_MEM = 0x40,
    //READ_MEM = 0x41,
    //READ_REG = 0x42,
    //WRITE_REG = 0x43,
    //READ_KEY = 0x60,
    //PUSH = 0x70,
    //POP = 0x71,
    //CALL = 0x72,
    //RETURN = 0x73,
    //CMP = 0x80,
    //JUMP_IF_EQ = 0x81,
    //JUMP_IF_LT = 0x82,
    //JUMP_IF_GT = 0x83,
    LEA = 0x90,
    MUL = 0xA0,
    DIV = 0xA1,
    MOD = 0xA2,
    BOOLMOV = 0xB0,
    BOOLAND = 0xB1,
    BOOLOR = 0xB2,
    BOOLNOT = 0xB3,
    BOOLXOR = 0xB4,
    AND = 0xC0,
    OR = 0xC1,
    XOR = 0xC2,
    BITNOT = 0xC3,
    SHL = 0xC4,
    SHR = 0xC5,
    FMOV = 0xD0,
    FADD = 0xD1,
    FSUB = 0xD2,
    FMUL = 0xD3,
    FDIV = 0xD4,
    DMOV = 0xE0,
    DADD = 0xE1,
    DSUB = 0xE2,
    DMUL = 0xE3,
    DDIV = 0xE4,
    STORE = 0xF0,
    LOAD = 0xF1
};
*/

// For reference now while I work towards the final assembler (V1.0)
const std::unordered_map<std::string_view, int> InstanceSize = {
    {"DB", 0},
    {"HALT", 1}, {"RETURN", 1}, {"NOT", 1},
    {"INTOUT", 2}, {"CHAROUT", 2}, {"READ_KEY", 2}, {"PUSH", 2}, {"POP", 2}, {"BOOLNOT", 2}, {"BITNOT", 2},
    {"MOV8", 3}, {"ADD", 3}, {"SUB", 3}, {"CMP", 3}, {"ADDI", 3}, {"READ_REG", 3}, {"WRITE_REG", 3}, {"MUL", 3}, {"DIV", 3}, {"MOD", 3}, {"BOOLMOV", 3}, {"BOOLAND", 3}, {"BOOLOR", 3}, {"BOOLXOR", 3}, {"AND", 3}, {"OR", 3}, {"XOR", 3}, {"SHL", 3}, {"SHR", 3},
    {"DADD", 3}, {"DSUB", 3}, {"DMUL", 3}, {"DDIV", 3}, {"MOV", 3}, {"FADD", 3}, {"FSUB", 3}, {"FMUL", 3}, {"FDIV", 3}, {"REALOUT", 3},
    {"STORE", 3}, {"LOAD", 3},
    {"JUMP", 5}, {"CALL", 5},
    {"JUMP_IF_EQ", 5}, {"JUMP_IF_LT", 5}, {"JUMP_IF_GT", 5}, {"OB", 5},
    {"JUMP_IF_ZERO", 6},
    {"READ_MEM", 6}, {"WRITE_MEM", 6}, {"MOV32", 6}, {"LEA", 6}, {"FMOV", 6},
    {"DMOV", 10}
};

std::vector<uint8_t> Bytecode;
std::unordered_map<std::string, uint32_t> Labels;
std::vector<std::string> SourceLines;

void WriteByte(uint8_t Byte) {
    Bytecode.push_back(Byte);
}

/*
// Superseded by WriteByte32LE, BE and WriteByte64 for future use
void WriteByte32(uint32_t Value) {
    WriteByte(Value & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 24) & 0xFF);
}*/

void WriteByte32LE(uint32_t Value) {
    WriteByte(Value & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 24) & 0xFF);
}

void WriteByte32BE(uint32_t Value) {
    WriteByte((Value >> 24) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte(Value & 0xFF);
}

void WriteByte64BE(uint64_t Value) {
    WriteByte((Value >> 56) & 0xFF);
    WriteByte((Value >> 48) & 0xFF);
    WriteByte((Value >> 40) & 0xFF);
    WriteByte((Value >> 32) & 0xFF);
    WriteByte((Value >> 24) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte(Value & 0xFF);
}

std::string Trim(std::string_view String) {
    size_t Start = String.find_first_not_of(" \t");
    if (Start == std::string::npos) return "";
    size_t End = String.find_last_not_of(" \t");
    return std::string(String.substr(Start, End - Start + 1));
}

std::string StripComment(std::string_view Line) {
    size_t CommentPos = Line.find(';');
    if (CommentPos != std::string::npos) {
        return std::string(Line.substr(0, CommentPos));
    }
    return std::string(Line);
}

bool IsLabel(std::string_view Line) {
    return Line.size() >= 2 && Line.back() == ':';
}

std::string GetLabelName(std::string_view Line) {
    return std::string(Line.substr(0, Line.size() - 1));
}

auto GetLabelAddress(std::string_view Label) {
    auto It = Labels.find(std::string(Label));
    if (It != Labels.end()) {
        return std::optional<uint32_t>{It->second};
    }
    return std::optional<uint32_t> {};
}

int ParseRegister(std::string_view String) {
    std::string Trimmed = Trim(String);
    if (Trimmed.empty() || Trimmed[0] != 'R') {
        std::cerr << "Error: Expected register R0-R7, got '" << Trimmed << "'" << std::endl;
        return 0;
    }
    return std::stoi(Trimmed.substr(1));
}

std::optional<int> ParseValue(std::string_view String) {
    std::string Trimmed = Trim(String);
    if (Trimmed.empty()) {
        return std::nullopt;
    }

    if (Trimmed.size() >= 3 && Trimmed.front() == '\'' && Trimmed.back() == '\'') {
        if (Trimmed.size() == 3) return (unsigned char)Trimmed[1];
        if (Trimmed[1] == '\\') {
            switch (Trimmed[2]) {
                case 'n': return '\n';
                case 'r': return '\r';
                case 't': return '\t';
                case '\\': return '\\';
                case '\'': return '\'';
                default: return Trimmed[2];
            }
        }
        return (unsigned char)Trimmed[1];
    }
    
    if (Trimmed.size() >= 3 && Trimmed[0] == '0' && Trimmed[1] == 'x') {
        try {
            return std::stoul(Trimmed, nullptr, 16);
        } catch (...) {
            return std::nullopt;
        }
    }
    
    try {
        return std::stoi(Trimmed);
    } catch (const std::invalid_argument& InvalidError) {
        std::cerr << "Error: invalid number '" << Trimmed << "'" << std::endl;
        return std::nullopt;
    } catch (const std::out_of_range& RangeError) {
        std::cerr << "Error: number out of range '" << Trimmed << "'" << std::endl;
        return std::nullopt;
    }
}

void FirstPass() {
    uint32_t Address = 0;
    
    for (const auto& Line : SourceLines) {
        std::string Trimmed = Trim(Line);
        if (Trimmed.empty() || Trimmed[0] == ';') continue;

        if (IsLabel(Trimmed)) {
            std::string Label = GetLabelName(Trimmed);
            if (Labels.find(Label) != Labels.end()) {
                std::cerr << "Warning: duplicate label '" << Label << "'" << std::endl;
            }
            Labels[Label] = Address;
            continue;
        }
        
        size_t Space = Trimmed.find(' ');
        std::string OperationCode = (Space == std::string::npos) ? Trimmed : Trimmed.substr(0, Space);
        std::string Rest = (Space == std::string::npos) ? "" : Trim(Trimmed.substr(Space + 1));

        if (OperationCode == "DB") {
            std::stringstream StringReader(Rest);
            std::string Item;
            int Count = 0;
            while (getline(StringReader, Item, ',')) {
                if (!Trim(Item).empty()) Count++;
            }
            Address += Count;
            continue;
        }
        
        auto Index = InstanceSize.find(OperationCode);
        if (Index != InstanceSize.end()) {
            Address += Index->second;
        }
    }
}

int ParseRegisterPro(const std::string& String) {
    if (String[0] == 'X') return std::stoi(String.substr(1));
    if (String[0] == 'R') return 8 + std::stoi(String.substr(1));
    if (String[0] == 'F') return 16 + std::stoi(String.substr(1));
    if (String[0] == 'D') return 24 + std::stoi(String.substr(1));
    if (String[0] == 'H') return 32 + std::stoi(String.substr(1));
    return 0;
}

void AssembleLine(std::string_view Line, uint32_t& PC) {
    std::string Trimmed = Trim(Line);
    Trimmed = StripComment(Trimmed);
    Trimmed = Trim(Trimmed);

    if (Trimmed.empty() || Trimmed[0] == ';') return;
    if (IsLabel(Trimmed)) return;
    
    size_t Space = Trimmed.find(' ');
    std::string OperationCode = (Space == std::string::npos) ? Trimmed : Trimmed.substr(0, Space);
    std::string Rest = (Space == std::string::npos) ? "" : Trim(Trimmed.substr(Space + 1));
   
    if (DebugFlag) {
        std::cerr << "Line: " << Line << std::endl;
        std::cerr << "Trimmed: " << Trimmed << std::endl;
        std::cerr << "Rest: " << Rest << std::endl;
        std::cerr << "OperationCode: " << OperationCode << std::endl;
    }

    if (OperationCode == "HALT") { WriteByte(0x00); PC += 1; }
    else if (OperationCode == "RETURN") { WriteByte(0x73); PC += 1; }
    else if (OperationCode == "NOT") { WriteByte(0x22); PC += 1; }
    else if (OperationCode == "INTOUT" || OperationCode == "CHAROUT") {
        int Register = ParseRegisterPro(Rest);
        WriteByte(OperationCode == "INTOUT" ? 0x30 : 0x31); WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "READ_KEY") {
        int Register = ParseRegisterPro(Rest);
        WriteByte(0x60); WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "PUSH" || OperationCode == "POP") {
        int Register = ParseRegisterPro(Rest);
        WriteByte(OperationCode == "PUSH" ? 0x70 : 0x71);
        WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "MOV8") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegisterPro(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for MOV8" << std::endl;
            return;
        }
        WriteByte(0x01); WriteByte(Register); WriteByte(Value.value());
        PC += 3;
    } else if (OperationCode == "MUL" || OperationCode == "DIV" || OperationCode == "MOD") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        int Source = ParseRegister(Rest.substr(Comma + 1));
        
        uint8_t Operation = (OperationCode == "MUL") ? 0xA0 : ((OperationCode == "DIV") ? 0xA1 : 0xA2);
        WriteByte(Operation);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "FADD" || OperationCode == "FSUB" || OperationCode == "FMUL" || OperationCode == "FDIV") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Trim(Rest.substr(0, Comma)));
        int Source = ParseRegisterPro(Trim(Rest.substr(Comma + 1)));

        uint8_t Operation;
        if (OperationCode == "FADD") Operation = 0xD1;
        else if (OperationCode == "FSUB") Operation = 0xD2;
        else if (OperationCode == "FMUL") Operation = 0xD3;
        else if (OperationCode == "FDIV") Operation = 0xD4;

        if (DebugFlag) {
            std::cout << "Destination: " << Destination << std::endl;
            std::cout << "Source: " << Source << std::endl;
            std::cout << "Operation: " << Operation << std::endl;
        }

        WriteByte(Operation);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "DADD" || OperationCode == "DSUB" || OperationCode == "DMUL" || OperationCode == "DDIV") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Trim(Rest.substr(0, Comma)));
        int Source = ParseRegisterPro(Trim(Rest.substr(Comma + 1)));

        std::cout << "Destination: " << Destination << std::endl;
        std::cout << "Source: " << Source << std::endl;

        uint8_t Operation;
        if (OperationCode == "DADD") Operation = 0xE1;
        else if (OperationCode == "DSUB") Operation = 0xE2;
        else if (OperationCode == "DMUL") Operation = 0xE3;
        else if (OperationCode == "DDIV") Operation = 0xE4;

        if (DebugFlag) {
            std::cout << "Destination: " << Destination << std::endl;
            std::cout << "Source: " << Source << std::endl;
            std::cout << "Operation: " << Operation << std::endl;
        }

        WriteByte(Operation);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "BOOLMOV") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegisterPro(Rest.substr(0, Comma));
        std::string Value = Trim(Rest.substr(Comma + 1));
        uint8_t BooleanValue = (Value == "TRUE" || Value == "true") ? 1 : 0;

        WriteByte(0xB0);
        WriteByte(Register);
        WriteByte(BooleanValue);
        PC += 3;
    } else if (OperationCode == "BOOLAND" || OperationCode == "BOOLOR" || OperationCode == "BOOLXOR") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        int Source = ParseRegister(Rest.substr(Comma + 1));
        
        uint8_t Operation = (OperationCode == "BOOLAND") ? 0xB1 : ((OperationCode == "BOOLOR") ? 0xB2 : 0xB4);
        WriteByte(Operation);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "REALOUT") {
        size_t Comma = Rest.find(',');
        std::string TypeString = Trim(Rest.substr(0, Comma));
        std::string RegisterString = Trim(Rest.substr(Comma + 1));
        
        int Type = (TypeString == "F") ? 0 : 1;
        int Register = ParseRegisterPro(RegisterString);
        
        WriteByte(0x33);
        WriteByte(Type);
        WriteByte(Register);
        PC += 3;
    } else if (OperationCode == "BOOLNOT") {
        int Destination = ParseRegisterPro(Rest);
        WriteByte(0xB3);
        WriteByte(Destination);
        PC += 2;
    } else if (OperationCode == "AND" || OperationCode == "OR" || OperationCode == "XOR" || OperationCode == "SHL" || OperationCode == "SHR") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        int Source = ParseRegister(Rest.substr(Comma + 1));

        uint8_t Operation = 0xC0;
        if (OperationCode == "AND") Operation = 0xC0;
        else if (OperationCode == "OR") Operation = 0xC1;
        else if (OperationCode == "XOR") Operation = 0xC2;
        else if (OperationCode == "SHL") Operation = 0xC4;
        else if (OperationCode == "SHR") Operation = 0xC5;
        
        WriteByte(Operation);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "BITNOT") {
        int Destination = ParseRegisterPro(Rest);
        WriteByte(0xC3);
        WriteByte(Destination);
        PC += 2;
    } else if (OperationCode == "MOV32") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegister(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for MOV32" << std::endl;
            return;
        }
        WriteByte(0x02); WriteByte(Register); WriteByte32LE(Value.value());
        PC += 6;
    } else if (OperationCode == "ADD" || OperationCode == "SUB") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        int Source = ParseRegisterPro(Rest.substr(Comma + 1));
        WriteByte(OperationCode == "ADD" ? 0x10 : 0x11);
        WriteByte(Destination); WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "ADDI") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for ADDI" << std::endl;
            return;
        }
        WriteByte(0x12); WriteByte(Destination); WriteByte(Value.value());
        PC += 3;
    } else if (OperationCode == "CMP") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegisterPro(Rest.substr(0, Comma));
        int Source = ParseRegisterPro(Rest.substr(Comma + 1));
        WriteByte(0x80); WriteByte(Destination); WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "READ_REG" || OperationCode == "WRITE_REG") {
        size_t Comma = Rest.find(',');
        std::string AddressRegisterString = Trim(Rest.substr(0, Comma));
        std::string DataRegisterString = Trim(Rest.substr(Comma + 1));
        int AddressRegister = ParseRegisterPro(AddressRegisterString);
        int DataRegister = ParseRegisterPro(DataRegisterString);
        WriteByte(OperationCode == "READ_REG" ? 0x42 : 0x43);
        WriteByte(AddressRegister);
        WriteByte(DataRegister);
        PC += 3;
    } else if (OperationCode == "JUMP") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x20); WriteByte32LE(Address.value());
        PC += 5;
    } else if (OperationCode == "JUMP_IF_ZERO") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegister(Rest.substr(0, Comma));
        std::string Target = Trim(Rest.substr(Comma + 1));
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x21); WriteByte(Register); WriteByte32LE(Address.value());
        PC += 6;
    } else if (OperationCode == "JUMP_IF_EQ" || OperationCode == "JUMP_IF_LT" || OperationCode == "JUMP_IF_GT") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        uint8_t Byte = (OperationCode == "JUMP_IF_EQ") ? 0x81 : ((OperationCode == "JUMP_IF_LT") ? 0x82 : 0x83);
        WriteByte(Byte); WriteByte32LE(Address.value());
        PC += 5;
    } else if (OperationCode == "CALL") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x72); WriteByte32LE(Address.value());
        PC += 5;
    } else if (OperationCode == "OB") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = Labels.count(Target) ? Labels[Target] : ParseValue(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
        }
        WriteByte(0x32); WriteByte32LE(Address.value());
        PC += 5;
    } else if (OperationCode == "READ_MEM" || OperationCode == "WRITE_MEM") {
        size_t Comma = Rest.find(',');
        std::string AddressString = Trim(Rest.substr(0, Comma));
        std::string RegisterString = Trim(Rest.substr(Comma + 1));
        std::optional<uint32_t> Address = ParseValue(AddressString);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address from address parameter '" << AddressString<< "'" << std::endl;
            return;
        }
        int Register = ParseRegisterPro(RegisterString);
        WriteByte(OperationCode == "READ_MEM" ? 0x41 : 0x40);
        WriteByte32LE(Address.value()); WriteByte(Register);
        PC += 6;
    } else if (OperationCode == "LEA") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegisterPro(Rest.substr(0, Comma));
        std::string Target = Trim(Rest.substr(Comma + 1));
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address from label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x90); WriteByte(Register); WriteByte32LE(Address.value());
        PC += 6;
    } else if (OperationCode == "DB") {
        std::stringstream StringReader(Rest);
        std::string Item;
        std::vector<int> Bytes;
        
        while (getline(StringReader, Item, ',')) {
            Item = Trim(Item);
            if (Item.empty()) continue;
            
            if (Item.size() >= 3 && Item.front() == '\'' && Item.back() == '\'') {
                if (Item.size() == 3) {
                    Bytes.push_back((unsigned char)Item[1]);
                } else if (Item[1] == '\\') {
                    switch (Item[2]) {
                        case 'n': Bytes.push_back('\n'); break;
                        case 'r': Bytes.push_back('\r'); break;
                        case 't': Bytes.push_back('\t'); break;
                        case '\\': Bytes.push_back('\\'); break;
                        case '\'': Bytes.push_back('\''); break;
                        default: Bytes.push_back(Item[2]);
                    }
                }
            } else {
                try {
                    int Value = std::stoi(Item);
                    Bytes.push_back(Value & 0xFF);
                } catch (...) {
                    std::cerr << "Error: Invalid DB value '" << Item << "'" << std::endl;
                }
            }
        }
        
        for (int Byte : Bytes) {
            WriteByte(Byte & 0xFF);
        }

        PC += Bytes.size();
    } else if (OperationCode == "MOV") {
        size_t Comma = Rest.find(',');
        std::string DestinationString = Trim(Rest.substr(0, Comma));
        std::string SourceString = Trim(Rest.substr(Comma + 1));
        
        int Destination = ParseRegisterPro(DestinationString);
        int Source = ParseRegisterPro(SourceString);
        
        WriteByte(0x03);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "LOAD" || OperationCode == "STORE") {
        size_t Comma = Rest.find(',');
        std::string DestinationString = Trim(Rest.substr(0, Comma));
        std::string SourceString = Trim(Rest.substr(Comma + 1));

        int Destination = ParseRegisterPro(DestinationString);
        int Source = ParseRegisterPro(SourceString);

        WriteByte((OperationCode == "STORE") ? 0xF0 : 0xF1);
        WriteByte(Destination);
        WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "FMOV") {
        size_t Comma = Rest.find(',');
        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string ValueString = Trim(Rest.substr(Comma + 1));
        
        int Register = ParseRegisterPro(RegisterString);
        float Value = std::stof(ValueString);
        uint32_t Bits;
        memcpy(&Bits, &Value, 4);
        
        WriteByte(0xD0);
        WriteByte(Register);
        WriteByte32BE(Bits);
        PC += 6;
    } else if (OperationCode == "DMOV") {
        size_t Comma = Rest.find(',');
        std::string RegisterString = Trim(Rest.substr(0, Comma));
        std::string ValueString = Trim(Rest.substr(Comma + 1));
        
        int Register = ParseRegisterPro(RegisterString);
        double Value = std::stod(ValueString);
        uint64_t Bits;
        memcpy(&Bits, &Value, 8);
        
        WriteByte(0xE0);
        WriteByte(Register);
        //WriteByte32BE(Bits & 0xFFFFFFFF);
        //WriteByte32BE((Bits >> 32) & 0xFFFFFFFF);
        WriteByte64BE(Bits);
        PC += 10;
    } else {
        std::cerr << "Warning: unknown instruction '" << OperationCode << "'" << std::endl;
    }
}

int main(int ArgumentC, char* ArgumentV[]) {
    if (ArgumentC < 2) {
        std::cout << "Usage: ./ja input.jbin [output.bin] [-debug]" << std::endl;
        return 1;
    }
    
    for (int i = 1; i < ArgumentC; i++) {
        std::string Arguements = ArgumentV[i];
        if (Arguements == "-debug") {
            DebugFlag = true;
        }
        if (Arguements == "-hex") {
            HexFlag = true;
        }
    }
    
    int FileIndex = 1;
    while (FileIndex < ArgumentC && std::string(ArgumentV[FileIndex])[0] == '-') {
        FileIndex++;
    }
    
    if (FileIndex >= ArgumentC) {
        std::cerr << "Error: No input file specified" << std::endl;
        return 1;
    }
    
    std::ifstream Input(ArgumentV[FileIndex]);
    if (!Input) {
        std::cerr << "Error: Cannot open " << ArgumentV[FileIndex] << std::endl;
        return 1;
    }
    
    std::string Outfile = "kernel.jbin";
    for (int i = FileIndex + 1; i < ArgumentC; i++) {
        std::string Arguements = ArgumentV[i];
        if (Arguements[0] != '-') {
            Outfile = Arguements;
            break;
        }
    }
    
    std::string Line;
    SourceLines.clear();
    while (getline(Input, Line)) {
        SourceLines.push_back(Line);
    }
    Input.close();
    
    Labels.clear();
    FirstPass();
    
    Bytecode.clear();
    uint32_t PC = 0;
    for (const auto& Line : SourceLines) {
        AssembleLine(Line, PC);
    }
    
    std::ofstream Output(Outfile, std::ios::binary);
    if (!Bytecode.empty()) {
        Output.write(reinterpret_cast<char*>(&Bytecode[0]), Bytecode.size());
    }
    Output.close();

    if (HexFlag) system("xxd kernel.jbin");
    
    std::cout << "Assembled " << Bytecode.size() << " bytes to " << Outfile << std::endl;
    return 0;
}