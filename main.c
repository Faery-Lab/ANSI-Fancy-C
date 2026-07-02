#include <stdio.h>
#include <stdlib.h>
#include "ANSI_Fancy.h"

int main() {
    // Enable ANSI escape sequences and UTF-8 output.
    InitializeConsole();

    printf("           " ANSI_YELLOW ANSI_SLANT "======== BASIC USAGE ========" ANSI_RESET "\n");

    // Add some data to an array for content.
    char *menuBoxContent[] = {
    ANSI_RED "This is some data." ANSI_RESET,
    ANSI_GREEN "This is some " ANSI_SLANT ANSI_BLUE "longer" ANSI_RESET ANSI_GREEN " data which causes it to wrap around in this example quite substantially by multiple rows." ANSI_RESET,
    NULL
    };

    // Display a cyan box with the data inside.
    MenuBox(ANSI_CYAN, 25, 3, "HEADER", menuBoxContent);



    // Start a blue flow with 5 units of indent.
    FlowStart(ANSI_BLUE, 5);

    FlowAdd("This is the first entry.");
    FlowAdd("This entry is longer and will automatically wrap if it exceeds the CONSOLE_WIDTH.");

    // Close the flow with the bottom connector.
    FlowFinish();




    printf("\n         " ANSI_YELLOW ANSI_SLANT "======== ADVANCED USAGE ========" ANSI_RESET "\n");

    char *menuBoxPromptContent[] = {
    ANSI_BOLD ANSI_WHITE "Which order do you want to run operations in?" ANSI_RESET,
    ANSI_UNDER ANSI_BLUE "1" ANSI_RESET ": " ANSI_RED "A " ANSI_MAGENTA "> " ANSI_YELLOW "B " ANSI_MAGENTA "> " ANSI_GREEN "C" ANSI_RESET,
    ANSI_UNDER ANSI_BLUE "2" ANSI_RESET ": " ANSI_GREEN "C " ANSI_MAGENTA "> " ANSI_YELLOW "B " ANSI_MAGENTA "> " ANSI_RED "A" ANSI_RESET,
    NULL
    };

    int choosing = 1;
    while (choosing) {
        char *response = MenuBoxPrompt(ANSI_CYAN, 50, 3, "ANSI FANCY DEMO", menuBoxPromptContent, ANSI_DIM ANSI_SLANT ANSI_WHITE "Select Option: " ANSI_RESET ANSI_DIM "(" ANSI_BLUE "1 " ANSI_RESET ANSI_DIM "or " ANSI_DIM ANSI_BLUE "2" ANSI_RESET ANSI_DIM ")" ANSI_RESET);
        if (atoi(response) == 1) {
            FlowStart(ANSI_BLUE, 5);
            FlowAdd(ANSI_RED " OPERATION A" ANSI_RESET);
            FlowAdd(ANSI_YELLOW " OPERATION B" ANSI_RESET);
            FlowAdd(ANSI_GREEN " OPERATION C" ANSI_RESET);
            FlowFinish();
            choosing = 0;
        } else if (atoi(response) == 2) {
            FlowStart(ANSI_BLUE, 5);
            FlowAdd(ANSI_GREEN " OPERATION C" ANSI_RESET);
            FlowAdd(ANSI_YELLOW " OPERATION B" ANSI_RESET);
            FlowAdd(ANSI_RED " OPERATION A" ANSI_RESET);
            FlowFinish();
            choosing = 0;
        } else {
            char errorMsg[256];
            snprintf(errorMsg, sizeof(errorMsg), ANSI_RED "Invalid choice: %s" ANSI_RESET, response);
            char *errorBoxContent[] = {
                errorMsg,
                ANSI_RED "Please enter 1 or 2" ANSI_RESET,
                NULL
            };
            MenuBox(ANSI_RED, 25, 2, ANSI_BLINK "ERROR" ANSI_RESET, errorBoxContent);
        }
    }
}
