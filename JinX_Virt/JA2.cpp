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

enum class OperationCode : uint8_t {
    HALT = 0x00,
    MOV8 = 0x01,
    MOV32 = 0x02,
    ADD = 0x10,
    SUB = 0x11,
    ADDI = 0x12,
    JUMP = 0x20,
    JUMP_IF_ZERO = 0x21,
    NOT = 0x22,
    OUTPUT = 0x30,
    OB = 0x31,
    WRITE_MEM = 0x40,
    READ_MEM = 0x41,
    READ_REG = 0x42,
    WRITE_REG = 0x43,
    READ_KEY = 0x60,
    PUSH = 0x70,
    POP = 0x71,
    CALL = 0x72,
    RETURN = 0x73,
    CMP = 0x80,
    JUMP_IF_EQ = 0x81,
    JUMP_IF_LT = 0x82,
    JUMP_IF_GT = 0x83,
    LEA = 0x90
};

const std::unordered_map<std::string_view, int> InstanceSize = {
    {"DB", 0},
    {"HALT", 1}, {"RETURN", 1}, {"NOT", 1},
    {"OUTPUT", 2}, {"READ_KEY", 2}, {"PUSH", 2}, {"POP", 2},
    {"MOV8", 3}, {"ADD", 3}, {"SUB", 3}, {"CMP", 3}, {"ADDI", 3}, {"READ_REG", 3}, {"WRITE_REG", 3},
    {"JUMP", 5}, {"CALL", 5},
    {"JUMP_IF_EQ", 5}, {"JUMP_IF_LT", 5}, {"JUMP_IF_GT", 5}, {"OB", 5},
    {"JUMP_IF_ZERO", 6},
    {"READ_MEM", 6}, {"WRITE_MEM", 6}, {"MOV32", 6}, {"LEA", 6}
};

std::vector<uint8_t> Bytecode;
std::unordered_map<std::string, uint32_t> Labels;
std::vector<std::string> SourceLines;

void WriteByte(uint8_t Byte) {
    Bytecode.push_back(Byte);
}

void WriteByte32(uint32_t Value) {
    WriteByte(Value & 0xFF);
    WriteByte((Value >> 8) & 0xFF);
    WriteByte((Value >> 16) & 0xFF);
    WriteByte((Value >> 24) & 0xFF);
}

std::string Trim(std::string_view String) {
    size_t Start = String.find_first_not_of(" \t");
    if (Start == std::string::npos) return "";
    size_t End = String.find_last_not_of(" \t");
    return std::string(String.substr(Start, End - Start + 1));
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
                std::cerr << "WARNING: duplicate label '" << Label << "'" << std::endl;
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
        
        auto It = InstanceSize.find(OperationCode);
        if (It != InstanceSize.end()) {
            Address += It->second;
        }
    }
}

void AssembleLine(std::string_view Line, uint32_t& PC) {
    std::string Trimmed = Trim(Line);
    if (Trimmed.empty() || Trimmed[0] == ';') return;
    if (IsLabel(Trimmed)) return;
    
    size_t Space = Trimmed.find(' ');
    std::string OperationCode = (Space == std::string::npos) ? Trimmed : Trimmed.substr(0, Space);
    std::string Rest = (Space == std::string::npos) ? "" : Trim(Trimmed.substr(Space + 1));
    
    if (OperationCode == "HALT") { WriteByte(0x00); PC += 1; }
    else if (OperationCode == "RETURN") { WriteByte(0x73); PC += 1; }
    else if (OperationCode == "NOT") { WriteByte(0x22); PC += 1; }
    else if (OperationCode == "OUTPUT") {
        int Register = ParseRegister(Rest);
        WriteByte(0x30); WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "READ_KEY") {
        int Register = ParseRegister(Rest);
        WriteByte(0x60); WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "PUSH" || OperationCode == "POP") {
        int Register = ParseRegister(Rest);
        WriteByte(OperationCode == "PUSH" ? 0x70 : 0x71);
        WriteByte(Register);
        PC += 2;
    } else if (OperationCode == "MOV8") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegister(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for MOV8" << std::endl;
            return;
        }
        WriteByte(0x01); WriteByte(Register); WriteByte(Value.value());
        PC += 3;
    } else if (OperationCode == "MOV32") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegister(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for MOV32" << std::endl;
            return;
        }
        WriteByte(0x02); WriteByte(Register); WriteByte32(Value.value());
        PC += 6;
    } else if (OperationCode == "ADD" || OperationCode == "SUB") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegister(Rest.substr(0, Comma));
        int Source = ParseRegister(Rest.substr(Comma + 1));
        WriteByte(OperationCode == "ADD" ? 0x10 : 0x11);
        WriteByte(Destination); WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "ADDI") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegister(Rest.substr(0, Comma));
        std::optional<int> Value = ParseValue(Rest.substr(Comma + 1));
        if (!Value.has_value()) {
            std::cerr << "Error: invalid value for ADDI" << std::endl;
            return;
        }
        WriteByte(0x12); WriteByte(Destination); WriteByte(Value.value());
        PC += 3;
    } else if (OperationCode == "CMP") {
        size_t Comma = Rest.find(',');
        int Destination = ParseRegister(Rest.substr(0, Comma));
        int Source = ParseRegister(Rest.substr(Comma + 1));
        WriteByte(0x80); WriteByte(Destination); WriteByte(Source);
        PC += 3;
    } else if (OperationCode == "READ_REG" || OperationCode == "WRITE_REG") {
        size_t Comma = Rest.find(',');
        std::string AddressRegisterString = Trim(Rest.substr(0, Comma));
        std::string DataRegisterString = Trim(Rest.substr(Comma + 1));
        int AddressRegister = ParseRegister(AddressRegisterString);
        int DataRegister = ParseRegister(DataRegisterString);
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
        WriteByte(0x20); WriteByte32(Address.value());
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
        WriteByte(0x21); WriteByte(Register); WriteByte32(Address.value());
        PC += 6;
    } else if (OperationCode == "JUMP_IF_EQ" || OperationCode == "JUMP_IF_LT" || OperationCode == "JUMP_IF_GT") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        uint8_t Byte = (OperationCode == "JUMP_IF_EQ") ? 0x81 : ((OperationCode == "JUMP_IF_LT") ? 0x82 : 0x83);
        WriteByte(Byte); WriteByte32(Address.value());
        PC += 5;
    } else if (OperationCode == "CALL") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x72); WriteByte32(Address.value());
        PC += 5;
    } else if (OperationCode == "OB") {
        std::string Target = Trim(Rest);
        std::optional<uint32_t> Address = Labels.count(Target) ? Labels[Target] : ParseValue(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address of label '" << Target << "'" << std::endl;
        }
        WriteByte(0x31); WriteByte32(Address.value());
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
        int Register = ParseRegister(RegisterString);
        WriteByte(OperationCode == "READ_MEM" ? 0x41 : 0x40);
        WriteByte32(Address.value()); WriteByte(Register);
        PC += 6;
    } else if (OperationCode == "LEA") {
        size_t Comma = Rest.find(',');
        int Register = ParseRegister(Rest.substr(0, Comma));
        std::string Target = Trim(Rest.substr(Comma + 1));
        std::optional<uint32_t> Address = GetLabelAddress(Target);
        if (!Address.has_value()) {
            std::cerr << "Error: could not get address from label '" << Target << "'" << std::endl;
            return;
        }
        WriteByte(0x90); WriteByte(Register); WriteByte32(Address.value());
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
    } else {
        std::cerr << "Warning: Unknown instruction '" << OperationCode << "'" << std::endl;
    }
}

int main(int ArgumentC, char* ArgumentV[]) {
    if (ArgumentC < 2) {
        std::cout << "Usage: jasm input.ja [output.bin]" << std::endl;
        return 1;
    }
    
    std::ifstream Input(ArgumentV[1]);
    if (!Input) {
        std::cerr << "Error: Cannot open " << ArgumentV[1] << std::endl;
        return 1;
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
    
    std::string Outfile = (ArgumentC > 2) ? ArgumentV[2] : "kernel.jbin";
    std::ofstream Output(Outfile, std::ios::binary);
    if (!Bytecode.empty()) {
        Output.write(reinterpret_cast<char*>(&Bytecode[0]), Bytecode.size());
    }
    Output.close();
    
    std::cout << "Assembled " << Bytecode.size() << " bytes to " << Outfile << std::endl;
    return 0;
}