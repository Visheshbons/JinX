#include "JinXVirtual.h"
#include <fstream>
#include <iostream>

#if defined(__APPLE__) || defined(__linux__)
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

JinXVM::JinXVM(int Size) {
    MemorySize = Size;
    Memory = new unsigned char[MemorySize];
    for (int i = 0; i < MemorySize; i++) {
        Memory[i] = 0;
    }

    for (int _ = 0; _ < 8; _++) {
        Registers[_] = 0;
    }
    ProgramCounter = 0;
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
                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[RegisterIndex] = Value;
                break;
            }
            case 0x02: { // MOV_IMM32
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
                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Source = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] += Registers[Source];
                break;
            }
            case 0x11: { // SUB
                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Source = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] -= Registers[Source];
                break;
            }
            case 0x12: { // ADDI
                int Destination = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Memory[ProgramCounter];
                ProgramCounter++;
                Registers[Destination] += Value;
                break;
            }
            case 0x20: { // JUMP
                int Address = 0;
                Address |= Memory[ProgramCounter++] << 0;
                Address |= Memory[ProgramCounter++] << 8;
                Address |= Memory[ProgramCounter++] << 16;
                Address |= Memory[ProgramCounter++] << 24;
                ProgramCounter = Address;
                break;
            }
            case 0x21: { // JUMP_IF_0
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
            case 0x30: { // OUTPUT
                int RegisterIndex = Memory[ProgramCounter];
                ProgramCounter++;
                int Value = Registers[RegisterIndex];
                char OutputCharacter = char(Value & 0xFF);
                std::cout << OutputCharacter;
                break;
            }
            case 0x40: { // WRITE_MEM
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