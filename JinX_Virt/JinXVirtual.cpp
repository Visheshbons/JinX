#include "JinXVirtual.h"
#include <fstream>
#include <iostream>

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
    }
}

void JinXVM::Run() {
    Running = true;

    while (Running) {
        unsigned char OperationCode = Memory[ProgramCounter];
        ProgramCounter++;

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
            case 0x12: { //
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
            case 0x22: {
                IsInverted = true;
                break;
            }
            case 0x30: { // OUTPUT
                IsInverted = false;

                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Registers[RegisterIndex];
                char OutputCharacter = char(Value & 0xFF);
                std::cout << OutputCharacter;
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
            case 0x91: {
                int Count = Memory[ProgramCounter++];
                ProgramCounter += Count;
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