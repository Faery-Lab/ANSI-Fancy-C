#ifndef ANSI_FANCY_CONFIG_H_INCLUDED
#define ANSI_FANCY_CONFIG_H_INCLUDED

#include <windows.h>            // Core Win32 API Header (Handles process mechanics & console)

#define CONSOLE_WIDTH 52        // The width of the display area (for consistent scaling).

#define FLOW_LEN 256            // The max number of characters allowed for flow content.

// Define ANSI escape codes for ease of terminal formatting

#define ANSI_BOLD    "\x1b[22m"
#define ANSI_SLANT   "\x1b[23m"
#define ANSI_UNDER   "\x1b[24m"
#define ANSI_BLINK   "\x1b[25m"
#define ANSI_STRIKE  "\x1b[29m"

#define ANSI_BLACK   "\x1b[30m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"

#define ANSI_RESET   "\x1b[0m"
#define ANSI_RETURN  "\x1b[A"
#define ANSI_CLEAR   "\x1b[K"
#define ANSI_ERASE   "\x1b[2K"

static inline void InitializeConsole() {
    // Ensure stdout prints immediately on Windows consoles without buffer lag.
    // Default is '_IOLBF' (I-O Line Buffering), requiring a '/n' before showing text.
    // '_IONBF' (I-O No Buffering) prints text to the device character by character.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // This allows for terminal colors to render properly (ANSI processing).
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    // This ensures the characters render properly.
    SetConsoleOutputCP(CP_UTF8);
}

#endif // ANSI_FANCY_CONFIG_H_INCLUDED
