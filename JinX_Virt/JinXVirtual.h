#ifndef JINX_VIRTUAL_H
#define JINX_VIRTUAL_H

#if defined(__APPLE__) || defined(__linux__)
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <conio.h>
    #include <windows.h>
#endif

class JinXVM {
    private:
        unsigned char* Memory;
        int MemorySize;
        int Registers[8];
        int ProgramCounter;
        bool Running;
    public:
        JinXVM(int Size);
        ~JinXVM();

        void Run();
        void Stop();
        void WriteByte(int Address, unsigned char Value);
        void SetProgramCounter(int Address);
        bool LoadFromFile(const char* Filename);
};

#endif