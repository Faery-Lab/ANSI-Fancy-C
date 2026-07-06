#ifndef ANSI_FANCY_CONFIG_H_INCLUDED
#define ANSI_FANCY_CONFIG_H_INCLUDED

#include <windows.h>                // Core Win32 API Header (Handles process mechanics & console)

#define CONSOLE_WIDTH 52            // The width of the display area (for consistent scaling).

#define FLOW_LEN 256                // The max number of characters allowed for flow content.

#define PROMPT_MARK "[>]"           // The symbol(s) used to indicate a user input.
#define PROMPT_DECLINE "-"
#define PROMPT_ACCEPT "+"

#define BAR_FILLED "●"              // The symbol used to indicate a filled progress bar tick.
#define BAR_EMPTY "○"               // The symbol used to indicate an empty progress bar tick.


// Define ANSI escape codes for ease of terminal formatting
#define ANSI_PREFIX "\x1b"
//#define ANSI_PREFIX "\033"

#define ANSI_RESET   ANSI_PREFIX "[0m"
#define ANSI_BOLD    ANSI_PREFIX "[1m"
#define ANSI_DIM     ANSI_PREFIX "[2m"
#define ANSI_SLANT   ANSI_PREFIX "[3m"
#define ANSI_UNDER   ANSI_PREFIX "[4m"
#define ANSI_BLINK   ANSI_PREFIX "[5m"
#define ANSI_STRIKE  ANSI_PREFIX "[9m"

#define ANSI_BLACK   ANSI_PREFIX "[30m"
#define ANSI_RED     ANSI_PREFIX "[31m"
#define ANSI_GREEN   ANSI_PREFIX "[32m"
#define ANSI_YELLOW  ANSI_PREFIX "[33m"
#define ANSI_BLUE    ANSI_PREFIX "[34m"
#define ANSI_MAGENTA ANSI_PREFIX "[35m"
#define ANSI_CYAN    ANSI_PREFIX "[36m"
#define ANSI_WHITE   ANSI_PREFIX "[37m"

#define ANSI_RETURN  ANSI_PREFIX "[A"
#define ANSI_CLEAR   ANSI_PREFIX "[K"
#define ANSI_ERASE   ANSI_PREFIX "[2K"

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

// Convert RGB colors into ANSI escape sequences using 8-bit notation (ANSI_ESCAPE 38;5;COLORm) for 256 total colors.
static inline const char* Get8BitColor(int color) {
    static char colorCode[32];                                                      // Stores the text for the ANSI escape code.
    snprintf(colorCode, sizeof(colorCode), ANSI_PREFIX "[38;5;%dm", color);         // Read the chosen colors into the escape code.
    return colorCode;                                                               // Return the color code to the user.
}

// Convert RGB colors into ANSI escape sequences using 24-bit notation (ANSI_ESCAPE R;G;Bm) for 16,777,216 total colors.
static inline const char* GetRGBColor(int r, int g, int b) {
    static char colorCode[32];                                                           // Stores the text for the ANSI escape code.
    snprintf(colorCode, sizeof(colorCode), ANSI_PREFIX "[38;2;%d;%d;%dm", r, g, b);      // Read the chosen colors into the escape code.
    return colorCode;                                                                    // Return the color code to the user.
}

// Characters which can be used as break points are specified here.
static inline int IsBreakPointCharacter(char character) {
    if (character == ' ' ||
        character == '\n' ||
        character == '\r' ||
        character == '\t'
        /*
        // Custom symbols to allow as a break point can be specified here:
        || character == '.' ||
        character == ',' ||
        character == ';'
        */
        ) return 1;
    return 0;
}
// Characters which are to be removed once used as a break point are specified here.
static inline int IsBreakPointCharacterRemoved(char character) {
    if (character == ' ' ||
        character == '\n' ||
        character == '\r' ||
        character == '\t'
        /*
        // Custom symbols which are to be removed once used as a break point can be specified here:
        || character == '.' ||
        character == ',' ||
        character == ';'
        */
        ) return 1;
    return 0;
}

// Get the number of regular characters in a string of text (the length without ANSI escape codes counted).
static inline int GetRegularCharacters(char *text) {
    int textLength = strlen(text);
    int regularCharacters = 0;
    // Loop through the text, skipping ANSI escape sequences.
    for(int cursor = 0; cursor < textLength; cursor++) {
        if (text[cursor] != ANSI_PREFIX[0]) {
            regularCharacters++;
        } else {
            while (cursor < textLength && (
                                           text[cursor] != 'm' &&     // The end of the escape sequence (formatting and color).
                                           text[cursor] != 'A' &&     // The end of the escape sequence (return to previous line).
                                           text[cursor] != 'K'        // The end of the escape sequence (clear and erase).
                                          )) cursor++;
        }
    }
    return regularCharacters;
}

#endif // ANSI_FANCY_CONFIG_H_INCLUDED
