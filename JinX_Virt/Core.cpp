#include "JinXVirtual.h"
#include <iostream>

#if defined(__APPLE__) || defined(__linux__)
    #include <termios.h>
    #include <unistd.h>
    
    static struct termios OriginalTerminal;
    
    void SetupTerminal() {
        tcgetattr(STDIN_FILENO, &OriginalTerminal);
        struct termios NewTerminal = OriginalTerminal;
        NewTerminal.c_lflag &= ~ICANON;
        NewTerminal.c_lflag &= ~ECHO;
        NewTerminal.c_cc[VMIN] = 1;
        NewTerminal.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &NewTerminal);

        system("stty raw -echo");
    }
    
    void RestoreTerminal() {
        tcsetattr(STDIN_FILENO, TCSANOW, &OriginalTerminal);

        system("stty cooked echo");
    }
#elif defined(_WIN32)
    #include <windows.h>
    #include <conio.h>
    
    static HANDLE hStdin;
    static DWORD original_mode;
    
    void SetupTerminal() {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hStdin, &original_mode);
        DWORD new_mode = original_mode;
        new_mode &= ~ENABLE_LINE_INPUT;  // Disable line buffering
        new_mode &= ~ENABLE_ECHO_INPUT;  // Disable echo
        SetConsoleMode(hStdin, new_mode);
    }
    
    void RestoreTerminal() {
        SetConsoleMode(hStdin, original_mode);
    }
#else
    void SetupTerminal() {}
    void RestoreTerminal() {}
#endif

int main() {
    SetupTerminal();
    
    JinXVM Simula(1024 * 1024);
    
    if (!Simula.LoadFromFile("kernel.jinx")) {
        std::cout << "Error: Failed to load kernel.jinx" << std::endl;
        RestoreTerminal();
        return 1;
    }
    
    Simula.Run();
    Simula.Stop();
    
    RestoreTerminal();
    return 0;
}