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



    printf("\n");

    ProgressBar(GetRGBColor(64, 255, 64), 256, "PROGRESS: ", 0, 100, 0); // Initial creation of the bar (isUpdate = 0).

    // Simulate progress updates.
    for (int i = 1; i <= 100; i ++) {
        Sleep(5);                                                              // Wait 5 milliseconds (0.005 seconds) to simulate processing.
        ProgressBar(GetRGBColor(64, 255, 64), 256, "PROGRESS: ", i, 100, 1);     // Update the loading bar (isUpdate = 1).
    }
    ProgressBar(GetRGBColor(128, 255, 128), 256, "PROGRESS: ", 100, 100, 1);     // Update the loading bar with a brighter color once it completes.

    printf("\n");




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
            FlowAdd(ANSI_RED " OPERATION A BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(128, 64, 64), 256, "OPERATION A: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(128, 64, 64), 256, "OPERATION A: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(255, 128, 128), 256, "OPERATION A: ", 100, 100, 1);

            FlowAdd(ANSI_YELLOW " OPERATION B BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(128, 128, 64), 256, "OPERATION B: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(128, 128, 64), 256, "OPERATION B: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(255, 255, 128), 256, "OPERATION B: ", 100, 100, 1);

            FlowAdd(ANSI_GREEN " OPERATION C BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(64, 128, 64), 256, "OPERATION C: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(64, 128, 64), 256, "OPERATION C: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(128, 255, 128), 256, "OPERATION C: ", 100, 100, 1);

            FlowAdd(ANSI_SLANT ANSI_CYAN " OPERATIONS FINISHED!" ANSI_RESET);
            FlowFinish();
            choosing = 0;
        } else if (atoi(response) == 2) {
            FlowStart(ANSI_BLUE, 5);
            FlowAdd(ANSI_GREEN " OPERATION C BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(64, 128, 64), 256, "OPERATION C: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(64, 128, 64), 256, "OPERATION C: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(128, 255, 128), 256, "OPERATION C: ", 100, 100, 1);

            FlowAdd(ANSI_YELLOW " OPERATION B BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(128, 128, 64), 256, "OPERATION B: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(128, 128, 64), 256, "OPERATION B: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(255, 255, 128), 256, "OPERATION B: ", 100, 100, 1);

            FlowAdd(ANSI_RED " OPERATION A BEGINNING..." ANSI_RESET);
            ProgressBar(GetRGBColor(128, 64, 64), 256, "OPERATION A: ", 0, 100, 0);
            for (int i = 1; i <= 100; i ++) {
            Sleep(5);
            ProgressBar(GetRGBColor(128, 64, 64), 256, "OPERATION A: ", i, 100, 1);
            }
            ProgressBar(GetRGBColor(255, 128, 128), 256, "OPERATION A: ", 100, 100, 1);

            FlowAdd(ANSI_SLANT ANSI_CYAN " OPERATIONS FINISHED!" ANSI_RESET);
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
