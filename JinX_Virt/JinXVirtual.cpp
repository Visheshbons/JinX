#include "JinXVirtual.h"
#include <fstream>
#include <iostream>
#include <iomanip>

#if defined(__APPLE__) || defined(__linux__)
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

struct Flags {
    bool IsZero;
    bool IsLess;
    bool IsGreater;
} Condition;

JinXVM::JinXVM(int Size) {
    MemorySize = Size;
    Memory = new unsigned char[MemorySize];
    for (int i = 0; i < MemorySize; i++) {
        Memory[i] = 0;
    }

    StackPointer = MemorySize - 4;

    for (int _ = 0; _ < 8; _++) {
        Registers[_] = 0;
        FloatRegisters[_] = 0.0;
        DoubleRegisters[_] = 0.0;
        TemporaryRegisters[_] = 0;
    }

    ProgramCounter = 0;
    Condition = {false, false, false};
    IsInverted = false;
    Running = false;
}

JinXVM::~JinXVM() {
    delete[] Memory;

    for (int _ = 0; _ < 8; _++) {
        Registers[_] = 0;
        FloatRegisters[_] = 0.0;
        DoubleRegisters[_] = 0.0;
        TemporaryRegisters[_] = 0;
    }
}

void JinXVM::Run() {
    Running = true;

    while (Running) {
        unsigned char OperationCode = Memory[ProgramCounter];
        ProgramCounter++;

        // std::cerr << "Program Counter = " << ProgramCounter << ", OperationCode = " << (int)OperationCode << std::endl;

        switch (OperationCode) {
            case 0x00: // HALT 
                Running = false;
                break;
            case 0x01: { // MOV_IMM8
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[RegisterIndex] = Value;
                break;
            }
            case 0x02: { // MOV_IMM32
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;

                int Value = 0;
                Value |= Memory[ProgramCounter++] << 0;
                Value |= Memory[ProgramCounter++] << 8;
                Value |= Memory[ProgramCounter++] << 16;
                Value |= Memory[ProgramCounter++] << 24;
                Registers[RegisterIndex] = Value;
                break;
            }
            case 0x03: { // MOV
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];

                int32_t SourceValue = 0;
                if (Source >= 0 && Source <= 7) {
                    SourceValue = TemporaryRegisters[Source];
                    TemporaryRegisters[Source] = 0;
                } else if (Source >= 8 && Source <= 15) {
                    SourceValue = Registers[Source - 8];
                } else if (Source >= 16 && Source <= 23) {
                    SourceValue = (int32_t)FloatRegisters[Source - 16];
                } else if (Source >= 24 && Source <= 31) {
                    SourceValue = (int32_t)DoubleRegisters[Source - 24];
                }

                if (Destination >= 0 && Destination <= 7) {
                    TemporaryRegisters[Destination] = SourceValue;
                } else if (Destination >= 8 && Destination <= 15) {
                    Registers[Destination - 8] = SourceValue;
                } else if (Destination >= 16 && Destination <= 23) {
                    FloatRegisters[Destination - 16] = (float)SourceValue;
                } else if (Destination >= 24 && Destination <= 31) {
                    DoubleRegisters[Destination - 24] = (double)SourceValue;
                }

                break;
            }
            case 0x10: { // ADD 
                IsInverted = false;

                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Source = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] += Registers[Source];
                break;
            }
            case 0x11: { // SUB
                IsInverted = false;

                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Source = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] -= Registers[Source];
                break;
            }
            case 0x12: { // ADDI
                IsInverted = false;

                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] += Value;
                break;
            }
            case 0x20: { // JUMP
                IsInverted = false;

                int Address = 0;
                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                ProgramCounter = Address;
                break;
            }
            case 0x21: { // JUMP_IF_0
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter];
                int Address = 0;
                ProgramCounter++;

                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                if (Registers[RegisterIndex] == 0) ProgramCounter = Address;
                break;
            }
            case 0x22: { // NOT
                IsInverted = true;
                break;
            }
            case 0x30: { // INTOUT
                IsInverted = false;
                int RegisterIndex = Memory[ProgramCounter++];
                int Value = Registers[RegisterIndex];
                std::cout << Value;
                break;
            }
            case 0x31: { // CHAROUT
                IsInverted = false;
                int RegisterIndex = Memory[ProgramCounter++];
                char Character = char(Registers[RegisterIndex] & 0xFF);
                std::cout << Character;
                break;
            }
            case 0x32: { // OB (CUSTOM COMMAND named OUTPUT BYTE)
                IsInverted = false;
                
                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                
                while (Memory[Address] != 0) {
                    std::cout << (char)Memory[Address];
                    Address++;
                }
                break;
            }
            case 0x33: { // REALOUT (OUTPUT for FLOAT or DOUBLE)
                IsInverted = false;
                int Type = Memory[ProgramCounter++];  // Codes 0 for float and 1 for double
                int Register = Memory[ProgramCounter++];

                if (Type == 0) std::cout << FloatRegisters[Register - 16];
                else std::cout << std::setprecision(15) << DoubleRegisters[Register - 24];
                break;
            }
            case 0x40: { // WRITE_MEM
                IsInverted = false;

                int Address = 0;
                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                
                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;

                Memory[Address] = Registers[RegisterIndex] & 0xFF;
                break;
            }
            case 0x41: { // READ_MEM
                IsInverted = false;

                int Address = 0;
                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                
                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;

                Registers[RegisterIndex] = Memory[Address];
                break;
            }
            case 0x42: { // READ_REG
                int AddressRegister = Memory[ProgramCounter++];
                int DestinationRegister = Memory[ProgramCounter++];
                Registers[DestinationRegister] = Memory[Registers[AddressRegister]];
                break;
            }
            case 0x43: { // WRITE_REG
                int AddressRegister = Memory[ProgramCounter++];
                int SourceRegister = Memory[ProgramCounter++];
                Memory[Registers[AddressRegister]] = Registers[SourceRegister] & 0xFF;
                break;
            }
            case 0x60: { // READ_KEY
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;
                
                int Key = 0;
                
                #if defined(__APPLE__) || defined(__linux__)
                    Key = getchar();
                #elif defined(_WIN32)
                    if (_kbhit()) {
                        Key = _getch();
                    }
                #endif
                
                Registers[RegisterIndex] = Key;
                break;
            }
            case 0x70: { // PUSH
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter++];
                uint32_t Value = Registers[RegisterIndex];
                
                StackPointer -= 4;
                Memory[StackPointer] = (Value >> 24) & 0xFF;
                Memory[StackPointer + 1] = (Value >> 16) & 0xFF;
                Memory[StackPointer + 2] = (Value >> 8) & 0xFF;
                Memory[StackPointer + 3] = Value & 0xFF;
                break;
            }
            case 0x71: { // POP
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter++];

                uint32_t Value = 0;
                Value |= Memory[StackPointer] << 24;
                Value |= Memory[StackPointer + 1] << 16;
                Value |= Memory[StackPointer + 2] << 8;
                Value |= Memory[StackPointer + 3];

                Registers[RegisterIndex] = Value;
                StackPointer += 4;
                break;
            }
            case 0x72: { // CALL
                IsInverted = false;

                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++];
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;

                uint32_t ReturnAddress = ProgramCounter;
                StackPointer -= 4;

                Memory[StackPointer] = (ReturnAddress >> 24) & 0xFF;
                Memory[StackPointer + 1] = (ReturnAddress >> 16) & 0xFF;
                Memory[StackPointer + 2] = (ReturnAddress >> 8) & 0xFF;
                Memory[StackPointer + 3] = ReturnAddress & 0xFF;

                ProgramCounter = Address;
                break;
            }
            case 0x73: { // RETURN
                IsInverted = false;

                uint32_t ReturnAddress = 0;
                ReturnAddress |= Memory[StackPointer] << 24;
                ReturnAddress |= Memory[StackPointer + 1] << 16;
                ReturnAddress |= Memory[StackPointer + 2] << 8;
                ReturnAddress |= Memory[StackPointer + 3];

                StackPointer += 4;
                ProgramCounter = ReturnAddress;
                break;
            }
            case 0x80: { // CMP
                IsInverted = false;

                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];

                int Result = Registers[Destination] - Registers[Source];
                Condition.IsZero = (Result == 0);
                Condition.IsLess = (Registers[Destination] < Registers[Source]);
                Condition.IsGreater = (Registers[Destination] > Registers[Source]);
                break;
            }
            case 0x81: { // JUMP_IF_EQ
                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++];
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;

                bool ShouldJump = Condition.IsZero;
                if (IsInverted) ShouldJump = !ShouldJump;
                IsInverted = false;

                if (ShouldJump) ProgramCounter = Address;
                break;
            }
            case 0x82: { // JUMP_IF_LT
                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++];
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;

                bool ShouldJump = Condition.IsLess;
                if (IsInverted) ShouldJump = !ShouldJump;
                IsInverted = false;

                if (ShouldJump) ProgramCounter = Address;
                break;
            }
            case 0x83: { // JUMP_IF_GT
                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++];
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;

                bool ShouldJump = Condition.IsGreater;
                if (IsInverted) ShouldJump = !ShouldJump;
                IsInverted = false;

                if (ShouldJump) ProgramCounter = Address;
                break;
            }
            case 0x90: { // LEA
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter++];
                uint32_t Address = 0;
                Address |= Memory[ProgramCounter++];
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;

                Registers[RegisterIndex] = Address;
                break;
            }
            case 0xA0: { // MUL
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] *= Registers[Source];
                break;
            }
            case 0xA1: { // DIV
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                if (Registers[Source] != 0) {
                    Registers[Destination] /= Registers[Source];
                } else {
                    std::cerr << "Error: division by zero" << std::endl;
                    Running = false;
                }
                break;
            }
            case 0xA2: { // MOD
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                if (Registers[Source] != 0) {
                    Registers[Destination] %= Registers[Source];
                } else {
                    std::cerr << "Error: modulo by zero" << std::endl;
                    Running = false;
                }
                break;
            }
            case 0xB0: { // BOOLMOV
                int Register = Memory[ProgramCounter++];
                uint8_t Value = Memory[ProgramCounter++];
                Registers[Register] = (Value != 0) ? 1 : 0;
                break;
            }
            case 0xB1: { // BOOLAND
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] = (Registers[Destination] && Registers[Source]) ? 1 : 0;
                break;
            }
            case 0xB2: { // BOOLOR
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] = (Registers[Destination] || Registers[Source]) ? 1 : 0;
                break;
            }
            case 0xB3: { // BOOLNOT
                int Destination = Memory[ProgramCounter++];
                Registers[Destination] = (Registers[Destination] == 0) ? 1 : 0;
                break;
            }
            case 0xB4: { // BOOLXOR
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] = (Registers[Destination] != Registers[Source]) ? 1 : 0;
                break;
            }
            case 0xC0: { // AND
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] &= Registers[Source];
                break;
            }
            case 0xC1: { // OR
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] |= Registers[Source];
                break;
            }
            case 0xC2: { // XOR
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                Registers[Destination] ^= Registers[Source];
                break;
            }
            case 0xC3: { // BITNOT
                int Destination = Memory[ProgramCounter++];
                Registers[Destination] = ~Registers[Destination];
                break;
            }
            case 0xC4: { // SHL
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                int Amount = Registers[Source];
                Registers[Destination] <<= Amount;
                break;
            }
            case 0xC5: { // SHR
                int Destination = Memory[ProgramCounter++];
                int Source = Memory[ProgramCounter++];
                int Amount = Registers[Source];
                Registers[Destination] >>= Amount;
                break;
            }
            case 0xD0: { // FMOV
                // Updated to Big Endian:
                int Register = Memory[ProgramCounter++] - 16; // -16 is to avoid writing into the adjacent registers (this may be double or temporary)
                uint32_t Bits = 0;
                Bits |= (uint32_t)Memory[ProgramCounter++] << 24;
                Bits |= (uint32_t)Memory[ProgramCounter++] << 16;
                Bits |= (uint32_t)Memory[ProgramCounter++] << 8;
                Bits |= (uint32_t)Memory[ProgramCounter++] << 0;
                memcpy(&FloatRegisters[Register], &Bits, 4);
                break;
            }
            case 0xD1: { // FADD 
                int Destination = Memory[ProgramCounter++] - 16;
                int Source = Memory[ProgramCounter++] - 16;
                FloatRegisters[Destination] += FloatRegisters[Source];
                break;
            }
            case 0xD2: { // FSUB
                int Destination = Memory[ProgramCounter++] - 16;
                int Source = Memory[ProgramCounter++] - 16;
                FloatRegisters[Destination] -= FloatRegisters[Source];
                break;
            }
            case 0xD3: { // FMUL
                int Destination = Memory[ProgramCounter++] - 16;
                int Source = Memory[ProgramCounter++] - 16;
                FloatRegisters[Destination] *= FloatRegisters[Source];
                break;
            }
            case 0xD4: { // FDIV
                int Destination = Memory[ProgramCounter++] - 16;
                int Source = Memory[ProgramCounter++] - 16;
                if (FloatRegisters[Source] != 0.0f) FloatRegisters[Destination] /= FloatRegisters[Source];
                else std::cerr << "Error: float division by zero" << std::endl;
                break;
            }
            case 0xE0: { // DMOV
                // Updated to Big Endian:
                int Register = Memory[ProgramCounter++] - 24;
                uint64_t Bits = 0;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 56;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 48;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 40;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 32;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 24;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 16;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 8;
                Bits |= (uint64_t)Memory[ProgramCounter++] << 0;
                memcpy(&DoubleRegisters[Register], &Bits, 8);
                break;
            }
            case 0xE1: { // DADD
                int Destination = Memory[ProgramCounter++] - 24;
                int Source = Memory[ProgramCounter++] - 24;
                DoubleRegisters[Destination] += DoubleRegisters[Source];
                break;
            }
            case 0xE2: { // DSUB
                int Destination = Memory[ProgramCounter++] - 24;
                int Source = Memory[ProgramCounter++] - 24;
                DoubleRegisters[Destination] -= DoubleRegisters[Source];
                break;
            }
            case 0xE3: { // DMUL
                int Destination = Memory[ProgramCounter++] - 24;
                int Source = Memory[ProgramCounter++] - 24;
                DoubleRegisters[Destination] *= DoubleRegisters[Source];
                break;
            }
            case 0xE4: { // DDIV
                int Destination = Memory[ProgramCounter++] - 24;
                int Source = Memory[ProgramCounter++] - 24;
                if (DoubleRegisters[Source] != 0.0) DoubleRegisters[Destination] /= DoubleRegisters[Source];
                else std::cerr << "Error: double division by zero" << std::endl;
                break;
            }
            default: {
                IsInverted = false;
                break;
            }
        }
    }
}

void JinXVM::Stop() {
    Running = false;
}

void JinXVM::WriteByte(int Address, unsigned char Value) {
    if (Address >= 0 && Address < MemorySize) {
        Memory[Address] = Value;
    }
}

void JinXVM::SetProgramCounter(int Address) {
    if (Address >= 0 && Address < MemorySize) {
        ProgramCounter = Address;
    }
}

bool JinXVM::LoadFromFile(const char* Filename) {
    std::ifstream FileLoaded(Filename, std::ios::binary);
    if (!FileLoaded.is_open()) {
        return false;
    }

    FileLoaded.read(reinterpret_cast<char*>(Memory), MemorySize);
    FileLoaded.close();

    ProgramCounter = 0;
    return true;
}

int JinXVM::DecodeRegister(int Encoded) {
    // Encoded registers:
    // X0 -> X7 found as 0 -> 7, 
    // R0 -> R7 found as 8 -> 15, 
    // F0 -> F7 found as 16 -> 23,
    // D0 -> D7 found as 24 -> 31

    if (Encoded >= 0 && Encoded <= 7) {
        return Encoded;
    } else if (Encoded >= 8 && Encoded <= 15) {
        return Encoded - 8;
    } else if (Encoded >= 16 && Encoded <= 23) {
        return Encoded - 16;
    } else if (Encoded >= 24 && Encoded <= 31) {
        return Encoded - 24;
    }

    return 0;
}
