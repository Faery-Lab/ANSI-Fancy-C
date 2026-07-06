#ifndef ANSI_FANCY_H_INCLUDED
#define ANSI_FANCY_H_INCLUDED

#include "ANSI_Fancy_Config.h"

// Global trackers for flows
static int flowRow = 0;                     // Counts how many entries have been added to the current flow.
static int flowWidth = 0;                   // Tracks how wide the connectors/indents of the current flow are.
static int insideFlow = 0;                  // Boolean to track if a flow is being built.
static int lastLineCount = 0;               // Tracks how many terminal lines the last entry occupied.
static char currentFlowColor[32];           // Stores the ANSI color code to be used for the connectors.
static char *flowContent[FLOW_LEN] = {0};   // Stores the text strings so they can be re-drawn if they are the final entry.

// Signal that the following prints to the screen are to be grouped into a flow.
static inline void FlowStart(char *flowColor, int width) {
    flowWidth = width;                                                  // Set the connector/indent width.
    insideFlow = 1;                                                     // Signal that a flow is active.
    flowRow = 0;                                                        // Reset the row counter.
    lastLineCount = 0;                                                  // Reset the line counter.
    strncpy(currentFlowColor, flowColor, sizeof(currentFlowColor) - 1); // Save the connector color.
}

// Add a row to the flow (with whitespace-aware wrapping).
static inline void FlowAdd(char *content) {
    // If flow is not started, disregard attempts to add.
    if (!insideFlow) return;

    // Save the text to the buffer (in case this is the final entry) so the bottom connector can be re-written when the flow ends.
    if (flowRow < 100) {
        if (flowContent[flowRow]) free(flowContent[flowRow]);
        flowContent[flowRow] = strdup(content);
    }

    int effectiveWidth = CONSOLE_WIDTH - (1 + flowWidth);   // Calculate space left for actual text (next to the connectors).
    int contentLength = strlen(content);                    // Tracks the overall length of the content.
    int cursor = 0;                                         // Track the current position while scanning through content.
    int linesUsed = 0;                                      // Tracks lines used if the text wraps (too long for one row).
    char lastFont[32] = "";                                 // Tracks the last used ANSI escape sequence for font.
    char lastColor[32] = "";                                // Tracks the last used ANSI escape sequence for color.

    // Loop through the content to handle word wrapping.
    while (cursor < contentLength) {
        int regularCharacters = 0;      // Track the number of regular (not used for ANSI escape sequence) characters.
        int chunkCursor = cursor;       // Track the position in the current content.
        int breakPoint = -1;            // Track the position of the last used break point character.

        // Try to find a space to break the line along, rather than cutting a word in half.
        while (chunkCursor < contentLength && regularCharacters < effectiveWidth) {
            // Skip over ANSI escape sequences while determining the characters to print.
            if (content[chunkCursor] == ANSI_PREFIX[0]) {
                while (chunkCursor < contentLength && content[chunkCursor] != 'm' && content[chunkCursor] != 'A' && content[chunkCursor] != 'K') chunkCursor++;
                chunkCursor++;
            // Count this character as not being an escape code.
            } else {
                // Save the last used break point character.
                if (IsBreakPointCharacter(content[chunkCursor])) breakPoint = chunkCursor;
                regularCharacters++;
                chunkCursor++;
            }
        }

        int chunkLength = chunkCursor - cursor;                                                         // Determine how long the current chunk of text is.
        if (chunkCursor < contentLength && breakPoint > cursor) chunkLength = breakPoint - cursor;      // Shorten to the break point if the line needs to be wrapped.

        // Print the start of the line with the correct connector (top-left-corner or t-bar).
        if (linesUsed == 0) {
            const char *firstSymbol = (flowRow == 0) ? "┌" : "├";       // Determine if this is the start of a new flow or the middle of an existing one.
            printf("%s%s", currentFlowColor, firstSymbol);              // Apply the color to the connector.
            for(int i = 0; i < flowWidth; i++) printf("─");             // Print the horizontal connector lines.
        // Print a continuation line to indicate wrapping text (no horizontal connectors, just a vertical bar).
        } else printf("%s│%*s", currentFlowColor, flowWidth, "");

        if (lastFont[0] != '\0') printf("%s", lastFont);        // Apply the last used font settings from the previous row.
        if (lastColor[0] != '\0') printf("%s", lastColor);      // Apply the last used color settings from the previous row.

        // Process chunk and update style buffers
        int printCursor = 0;
        while(printCursor < chunkLength) {
            if (content[cursor + printCursor] == ANSI_PREFIX[0]) {
                int start = cursor + printCursor;
                // Skip past ANSI escape codes so they are not counted as printed to the screen.
                while (printCursor < chunkLength && content[cursor + printCursor] != 'm' && content[cursor + printCursor] != 'A' && content[cursor + printCursor] != 'K') printCursor++;
                int escapeLength = printCursor - (start - cursor) + 1;      // Determine how long the ANSI escape code was (if there was one).

                // If no font or color settings were detected, set them as empty.
                if (strncmp(content + start, ANSI_RESET, escapeLength) == 0) {
                    lastFont[0] = '\0';
                    lastColor[0] = '\0';
                // If the last character of the escape sequence is 'm', than it is either a font or color code.
                } else if (content[cursor + printCursor] == 'm') {
                    // If the escape sequence has two characters following '[', it is a color code.
                    if (printCursor > 1 && content[cursor + printCursor - 2] == '[') { strncpy(lastColor, content + start, escapeLength); lastColor[escapeLength] = '\0'; }
                    // If the escape sequence only has one character following '[', it is a font code.
                    else { strncpy(lastFont, content + start, escapeLength); lastFont[escapeLength] = '\0'; }
                }
                printf("%.*s", escapeLength, content + start);      // Print the ANSI escape sequence to apply the font or color.
                printCursor++;                                      // Move to the next character in the current text.
            // Print the current character and move to the next.
            } else {
                printf("%c", content[cursor + printCursor]);
                printCursor++;
            }
        }

        printf(ANSI_CLEAR "\n");    // End the line and clear the font and color for the next row.

        cursor += chunkLength;                                                                      // Move the cursor to the beginning of the next chunk of text.
        if (cursor < contentLength && IsBreakPointCharacterRemoved(content[cursor])) cursor++;      // If the break point character gets removed, skip past it.
        linesUsed++;                                                                                // Increment the number of lines used by the content.
    }

    lastLineCount = linesUsed;  // Store how many lines this entry took up.
    flowRow++;                  // Move to the next flow slot.
}

// Signal that the flow has ended.
static inline void FlowFinish() {
    // If nothing was added to the flow, turn flow back off and do nothing.
    if (flowRow == 0) {
            insideFlow = 0;
            return;
    }

    // Move cursor back up while erasing the last printed entry so the connector can be replaced with the right type.
    for (int i = 0; i < lastLineCount; i++) {
        printf(ANSI_RETURN ANSI_ERASE);     // Move up one line and erase the whole line.
    }

    char *lastContent = flowContent[flowRow - 1];
    int indentLength = 1 + flowWidth;
    int effectiveWidth = CONSOLE_WIDTH - (1 + flowWidth);   // Calculate space left for actual text (next to the connectors).
    int contentLength = strlen(lastContent);                // Tracks the overall length of the content.
    int cursor = 0;                                         // Track the current position while scanning through content.
    int linesUsed = 0;                                      // Tracks lines used if the text wraps (too long for one row).
    char lastFont[32] = "";                                 // Tracks the last used ANSI escape sequence for font.
    char lastColor[32] = "";                                // Tracks the last used ANSI escape sequence for color.

    // Repeat the adding process to re-write the bottom of the flow.
    while (cursor < contentLength) {
        int regularCharacters = 0;      // Track the number of regular (not used for ANSI escape sequence) characters.
        int chunkCursor = cursor;       // Track the position in the current content.
        int breakPoint = -1;            // Track the position of the last used break point character.

        // Try to find a space to break the line along, rather than cutting a word in half.
        while (chunkCursor < contentLength && regularCharacters < effectiveWidth) {
            // Skip over ANSI escape sequences while determining the characters to print.
            if (lastContent[chunkCursor] == ANSI_PREFIX[0]) {
                while (chunkCursor < contentLength && lastContent[chunkCursor] != 'm' && lastContent[chunkCursor] != 'A' && lastContent[chunkCursor] != 'K') chunkCursor++;
                chunkCursor++;
            // Count this character as not being an escape code.
            } else {
                // Save the last used break point character.
                if (IsBreakPointCharacter(lastContent[chunkCursor])) breakPoint = chunkCursor;
                regularCharacters++;
                chunkCursor++;
            }
        }

        int chunkLength = chunkCursor - cursor;                                                         // Determine how long the current chunk of text is.
        if (chunkCursor < contentLength && breakPoint > cursor) chunkLength = breakPoint - cursor;      // Shorten to the break point if the line needs to be wrapped.

        // If only one entry was added to the flow, draw its connector as a horizontal line.
        if (linesUsed == 0 && flowRow == 1) {
            printf("%s─", currentFlowColor);
            for(int i = 0; i < flowWidth; i++) printf("─");
        // Print the final of multiple entries with the correct connector (bottom-left-corner).
        } else if (linesUsed == 0) {
            printf("%s└", currentFlowColor);
            for(int i = 0; i < flowWidth; i++) printf("─");
        // Print an indent to indicate wrapping text (no horizontal connectors, or vertical bar).
        } else printf("%s%*s", currentFlowColor, flowWidth + 1, "");

        if (lastFont[0] != '\0') printf("%s", lastFont);        // Apply the last used font settings from the previous row.
        if (lastColor[0] != '\0') printf("%s", lastColor);      // Apply the last used color settings from the previous row.

        int printCursor = 0;
        while(printCursor < chunkLength) {
            if (lastContent[cursor + printCursor] == ANSI_PREFIX[0]) {
                int start = cursor + printCursor;
                // Skip past ANSI escape codes so they are not counted as printed to the screen.
                while (printCursor < chunkLength && lastContent[cursor + printCursor] != 'm' && lastContent[cursor + printCursor] != 'A' && lastContent[cursor + printCursor] != 'K') printCursor++;
                int escapeLength = printCursor - (start - cursor) + 1;      // Determine how long the ANSI escape code was (if there was one).

                // If no font or color settings were detected, set them as empty.
                if (strncmp(lastContent + start, ANSI_RESET, escapeLength) == 0) {
                    lastFont[0] = '\0';
                    lastColor[0] = '\0';
                // If the last character of the escape sequence is 'm', than it is either a font or color code.
                } else if (lastContent[cursor + printCursor] == 'm') {
                    // If the escape sequence has two characters following '[', it is a color code.
                    if (printCursor > 1 && lastContent[cursor + printCursor - 2] == '[') { strncpy(lastColor, lastContent + start, escapeLength); lastColor[escapeLength] = '\0'; }
                    // If the escape sequence only has one character following '[', it is a font code.
                    else { strncpy(lastFont, lastContent + start, escapeLength); lastFont[escapeLength] = '\0'; }
                }
                printf("%.*s", escapeLength, lastContent + start);      // Print the ANSI escape sequence to apply the font or color.
                printCursor++;                                          // Move to the next character in the current text.
            // Print the current character and move to the next.
            } else {
                printf("%c", lastContent[cursor + printCursor]);
                printCursor++;
            }
        }
        printf(ANSI_CLEAR "\n");    // End the line and clear the font and color for the next row.

        cursor += chunkLength;                                                                          // Move the cursor to the beginning of the next chunk of text.
        if (cursor < contentLength && IsBreakPointCharacterRemoved(lastContent[cursor])) cursor++;      // If the break point character gets removed, skip past it.
        linesUsed++;                                                                                    // Increment the number of lines used by the content.
    }

    // Free up the used memory and reset the global flow tracker variables.
    for (int i = 0; i < flowRow; i++) {
        free(flowContent[i]);
        flowContent[i] = NULL;
    }
    insideFlow = 0;             // Exit the flow.
    flowRow++;                  // Move to the next flow slot.
}

/*

Examples of flows (with CONSOLE_WIDTH set to 52):

    FlowStart(ANSI_RED, 0);
    FlowAdd("Entry 1 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowFinish();

─Entry 1 has some really long text which causes it
 to wrap around in this example quite substantially
 by two rows.

    FlowStart(ANSI_GREEN, 3);
    FlowAdd("Entry 1");
    FlowAdd("Entry 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowFinish();

┌───Entry 1
└───Entry 2 has some really long text which causes
    it to wrap around in this example quite
    substantially by two rows.

    FlowStart(ANSI_BLUE, 5);
    FlowAdd("Entry 1");
    FlowAdd("Entry 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowAdd("Entry 3");
    FlowFinish();

┌─────Entry 1
├─────Entry 2 has some really long text which
│     causes it to wrap around in this example
│     quite substantially by two rows.
└─────Entry 3

*/

// Displays a stylized box with a header and listed content items.
static inline void MenuBox(char *boxColor, int boxWidth, int boxHeight, char *header, char **content) {
    int effectiveWidth = (boxWidth < CONSOLE_WIDTH) ? boxWidth : CONSOLE_WIDTH;             // Determine the width of the box, capped at a max of CONSOLE_WIDTH.
    int horizontalOffset = (boxWidth < CONSOLE_WIDTH) ? (CONSOLE_WIDTH - boxWidth) / 2 : 0; // Determine how many spaces to print before the box are needed to center it horizontally.

    // Internal helper function for printing the leading spaces for centering.
    void printOffset() { for (int i = 0; i < horizontalOffset; i++) printf(" "); }

    char lastColor[32] = "";        // Track the last color used so it can be re-applied if the text wraps.
    char lastFont[32] = "";         // Track the last font used so it can be re-applied if the text wraps.

    printOffset();                                              // Offset the row if needed.
    printf(ANSI_RESET "%s┌", boxColor);                         // Print the top-left corner.
    for (int i = 0; i < effectiveWidth - 2; i++) printf("─");   // Print the top border.
    printf("┐\n");                                              // Print the top-right corner.

    int maxWidth = effectiveWidth - 2;                          // Determine max space available for text inside the box (subtracting borders).
    int cursor = 0;                                             // Track the current position in the entire header string.
    int totalLength = strlen(header);                           // Track the total length of the header (including ANSI escape codes).

    // Process the header, breaking it into smaller chunks if it exceeds the width of the box.
    while (cursor < totalLength) {
        int regularCharacters = 0;      // Track the number of regular characters (not including ANSI escape sequences).
        int chunkCursor = cursor;       // Track the position in the current chunk of text.
        int breakPoint = -1;            // Track the position of the latest-used break point character.

        // Determine which characters are printed and identify the break point.
        while (chunkCursor < totalLength && regularCharacters < maxWidth) {
            if (header[chunkCursor] == ANSI_PREFIX[0]) {
                while (chunkCursor < totalLength && header[chunkCursor] != 'm'      // The end of the escape sequence (formatting and color).
                                                 && header[chunkCursor] != 'A'      // The end of the escape sequence (return to previous line).
                                                 && header[chunkCursor] != 'K'      // The end of the escape sequence (clear and erase).
                      ) chunkCursor++;      // Skip past the current character of the escape sequence.
                chunkCursor++;              // Skip past the last character of the escape sequence.
            } else {
                if (IsBreakPointCharacter(header[chunkCursor])) breakPoint = chunkCursor;   // Save where the break point is.
                regularCharacters++;                                                        // Add the character to the count of regular characters.
                chunkCursor++;                                                              // Skip to the next character.
            }
        }
        int chunkSize = chunkCursor - cursor;                                                       // Determine the size of this chunk of text.
        if (chunkCursor < totalLength && breakPoint > cursor) chunkSize = breakPoint - cursor;      // Try to find a space to break the line along, rather than cutting a word in half.

        printOffset();      // Print the offset if needed.
        printf("│");        // Print the left-side border of the header.

        int printCursor = 0;                                    // Track the current position in the header while printing characters.
        regularCharacters = 0;                                  // Clear the 'regularCharacters' counter for reuse.
        char chunk[512];                                        // Save the current chunk of header text.
        memcpy(chunk, header + cursor, chunkSize);              // Copy the header text into the chunk.
        chunk[chunkSize] = '\0';                                // Append a null terminator to the end of the chunk text.
        regularCharacters = GetRegularCharacters(chunk);        // Determine the number of regular characters (not including ANSI escape sequences) in the chunk.

        int pad = (maxWidth - regularCharacters) / 2;           // Determine the padding needed in order to center the header text.
        for(int i = 0; i < pad; i++) printf(" ");               // Print the left-side padding for centering the header text.

        // Loop through the chunk while printing characters to the console.
        while(printCursor < chunkSize) {
            // Determine which characters are printed and which are escape sequences.
            if (header[cursor + printCursor] == ANSI_PREFIX[0]) {
                int start = cursor + printCursor;
                while (printCursor < chunkSize && header[cursor + printCursor] != 'm'    // The end of the escape sequence (formatting and color).
                                               && header[cursor + printCursor] != 'A'    // The end of the escape sequence (return to previous line).
                                               && header[cursor + printCursor] != 'K'    // The end of the escape sequence (clear and erase).
                      ) printCursor++;
                int escapeLength = printCursor - (start - cursor) + 1;
                // if this is a reset code, clear the saved color and font.
                if (strncmp(header + start, ANSI_RESET, escapeLength) == 0) {
                    lastColor[0] = '\0';
                    lastFont[0] = '\0';
                // If this is a color or font escape code, update the last used.
                } else if (header[cursor + printCursor] == 'm') {
                    // If the escape code has two characters following '[', it is a color code.
                    if (printCursor > 1 && header[cursor + printCursor - 2] == '[') {
                            strncpy(lastColor, header + start, escapeLength);
                            lastColor[escapeLength] = '\0';
                    }
                    // If the escape code has one character following '[', it is a color code.
                    else {
                            strncpy(lastFont, header + start, escapeLength);
                            lastFont[escapeLength] = '\0';
                    }
                }
                printf("%.*s", escapeLength, header + start);           // Print the ANSI escape sequence to apply it to the current text.
                printCursor++;                                          // Increment to the next position in the header text.
            } else printf("%c", header[cursor + printCursor++]);        // Print the regular character and increment to the next.
        }

        for(int i = 0; i < (maxWidth - regularCharacters - pad); i++) printf(" ");              // Print the right-side padding for centering the header text.
        printf(ANSI_RESET "%s│\n", boxColor);                                                   // Re-apply the box color and print the right-side border.
        cursor += chunkSize;                                                                    // Increment the main cursor past the current chunk.
        if (cursor < totalLength && IsBreakPointCharacterRemoved(header[cursor])) cursor++;     // If the break point character is included among the list of removed characters, skip over it.
    }

    printOffset();                                              // Offset the row if needed.
    printf("├");                                                // Print the left divider border between the header and the content areas.
    for (int i = 0; i < effectiveWidth - 2; i++) printf("─");   // Print the divider.
    printf("┤\n");                                              // Print the right divider border.

    int rowsPrinted = 0;                            // Keep track of how many lines of content have been drawn.

    // Loop through the content array until hitting a NULL.
    for (int i = 0; content[i] != NULL; i++) {
        lastColor[0] = '\0';                                    // Reset the color.
        lastFont[0] = '\0';                                     // Reset the font.
        maxWidth = effectiveWidth - 3;                          // Determine max space available for text inside the box (subtracting borders and left-indent).
        char *text = content[i];                                // Save the current text item.
        totalLength = strlen(text);                             // Track the total length of the current text item (including ANSI escape codes).
        cursor = 0;                                             // Track the current position in the entire text string.

        // Process the text, breaking it into smaller chunks if it exceeds the width of the box.
        while (cursor < totalLength) {
            int regularCharacters = 0;      // Track the number of regular characters (not including ANSI escape sequences).
            int chunkCursor = cursor;       // Track the position in the current chunk of text.
            int breakPoint = -1;            // Track the position of the latest-used break point character.

            // Determine which characters are printed and identify the break point.
            while (chunkCursor < totalLength && regularCharacters < maxWidth) {
                if (text[chunkCursor] == ANSI_PREFIX[0]) {
                    while (chunkCursor < totalLength && text[chunkCursor] != 'm'    // The end of the escape sequence (formatting and color).
                                                     && text[chunkCursor] != 'A'    // The end of the escape sequence (return to previous line).
                                                     && text[chunkCursor] != 'K'    // The end of the escape sequence (clear and erase).
                          ) chunkCursor++;      // Skip past the current character of the escape sequence.
                    chunkCursor++;              // Skip past the last character of the escape sequence.
                } else {
                    if (IsBreakPointCharacter(text[chunkCursor])) breakPoint = chunkCursor;     // Save where the break point is.
                    regularCharacters++;                                                        // Add the character to the count of regular characters.
                    chunkCursor++;                                                              // Skip to the next character.
                }
            }

            int chunkSize = chunkCursor - cursor;                                                       // Determine the size of this chunk of text.
            if (chunkCursor < totalLength && breakPoint > cursor) chunkSize = breakPoint - cursor;      // Try to find a space to break the line along, rather than cutting a word in half.

            printOffset();                                          // Print the offset if needed.
            printf("│ ");                                           // Print the left-side border.
            if (lastFont[0] != '\0') printf("%s", lastFont);        // Carry over the previous font.
            if (lastColor[0] != '\0') printf("%s", lastColor);      // Carry over the previous color.

            int printCursor = 0;                                        // Track the current position in the header while printing characters.

            // Loop through the chunk while printing characters to the console.
            while(printCursor < chunkSize) {
                // Determine which characters are printed and which are escape sequences.
                if (text[cursor + printCursor] == ANSI_PREFIX[0]) {
                    int start = cursor + printCursor;
                    while (printCursor < chunkSize && text[cursor + printCursor] != 'm'     // The end of the escape sequence (formatting and color).
                                                   && text[cursor + printCursor] != 'A'     // The end of the escape sequence (return to previous line).
                                                   && text[cursor + printCursor] != 'K'     // The end of the escape sequence (clear and erase).
                          ) printCursor++;                                                  // Skip past the current character of the escape sequence.
                    int escapeLength = printCursor - (start - cursor) + 1;                  // Track the length of the current ANSI escape code.
                    // if this is a reset code, clear the saved color and font.
                    if (strncmp(text + start, ANSI_RESET, escapeLength) == 0) {
                        lastColor[0] = '\0';
                        lastFont[0] = '\0';
                    // If this is a color or font escape code, update the last used.
                    } else if (text[cursor + printCursor] == 'm') {
                        // If the code is two digits, it is a color code, otherwise, it is for font.
                        if (printCursor > 1 && text[cursor + printCursor - 2] == '[') { strncpy(lastColor, text + start, escapeLength); lastColor[escapeLength] = '\0'; }
                        else { strncpy(lastFont, text + start, escapeLength); lastFont[escapeLength] = '\0'; }
                    }
                    printf("%.*s", escapeLength, text + start);     // Print the ANSI escape sequence to apply it to the current text.
                    printCursor++;                                  // Increment to the next position in the header text.
                } else {
                    printf("%c", text[cursor + printCursor++]);       // Print the regular character and increment to the next.
                }
            }

            char regularCharactersPrinted[chunkSize + 1];                                   // Determine the number of characters printed to the line.
            memcpy(regularCharactersPrinted, text + cursor, chunkSize);                     // Copy the the characters in the current chunk (including ANSI escape codes).
            regularCharactersPrinted[chunkSize] = '\0';                                     // Null terminate the text.
            int regularCharactersChunk = GetRegularCharacters(regularCharactersPrinted);    // Count the number of printed characters (without counting escape codes).

            for (int spaces = 0; spaces < (maxWidth - regularCharactersChunk); spaces++) printf(" ");           // Pad out the remaining space to the right of the breakpoint.
            printf("%s│\n", boxColor);                                                                          // Print the right-side border.

            cursor += chunkSize;                                                                    // Reposition the primary text cursor.
            if (cursor < totalLength && IsBreakPointCharacterRemoved(text[cursor])) cursor++;       // If the break point character is included among the list of removed characters, skip over it.
            rowsPrinted++;                                                                          // Increment the number of rows which have been printed.
        }
    }

    // If the content is shorter than the box height, draw empty rows.
    for (int i = rowsPrinted; i < boxHeight; i++) {
        printOffset();                                                                              // Offset the row if needed.
        printf("│");                                                                                // Print the left-side border.
        for (int chunkCursor = 0; chunkCursor < effectiveWidth - 2; chunkCursor++) printf(" ");     // Print the empty space.
        printf("│\n");                                                                              // Print the right-side border.
    }

    printOffset();                                              // Offset the row if needed.
    printf("└");                                                // Print the bottom-left corner.
    for (int i = 0; i < effectiveWidth - 2; i++) printf("─");   // Print the bottom border.
    printf("┘%s\n", ANSI_RESET);                                // Print the bottom-right corner, then reset console formatting to default.
}

/*
    Examples of menu boxes (with CONSOLE_WIDTH set to 52):

    Declare the content items as an array of character pointers:

    char *menuBoxContent[] = {
    ANSI_RED "Layer 1" ANSI_RESET,
    ANSI_GREEN "Layer 2 has some " ANSI_SLANT ANSI_BLUE "really" ANSI_RESET ANSI_GREEN " long text which causes it to wrap around in this example quite substantially by multiple rows." ANSI_RESET,
    "Layer 3",
    NULL
    };

    Then the array can be passed to the MenuBox function:

    MenuBox(ANSI_CYAN, 25, 3, "HEADER", menuBoxContent);

             ┌───────────────────────┐
             │        HEADER         │
             ├───────────────────────┤
             │ Layer 1               │
             │ Layer 2 has some      │
             │ really long text      │
             │ which causes it to    │
             │ wrap around in this   │
             │ example quite         │
             │ substantially by      │
             │ multiple rows.        │
             │ Layer 3               │
             └───────────────────────┘

    MenuBox(ANSI_MAGENTA, 100, 3, "HEADER", menuBoxContent);

┌──────────────────────────────────────────────────┐
│                      HEADER                      │
├──────────────────────────────────────────────────┤
│ Layer 1                                          │
│ Layer 2 has some really long text which causes   │
│ it to wrap around in this example quite          │
│ substantially by multiple rows.                  │
│ Layer 3                                          │
└──────────────────────────────────────────────────┘


*/

// Displays a stylized box with a header and listed content items, prompts the user for input, then reformats the box to include the submission encapsulated.
static inline char *MenuBoxPrompt(char *boxColor, int boxWidth, int boxHeight, char *header, char **content, char *prompt) {
    int horizontalOffset = (boxWidth < CONSOLE_WIDTH) ? (CONSOLE_WIDTH - boxWidth) / 2 : 0; // Determine how many spaces to print before the box are needed to center it horizontally.
    static char userInput[256];                                                             // Track the user's response.

    // Internal helper function for printing the leading spaces for centering.
    void printOffset() { for (int i = 0; i < horizontalOffset; i++) printf(" "); }

    MenuBox(boxColor, boxWidth, boxHeight, header, content);                    // Create the top section of the prompt.
    printf("%s " PROMPT_MARK " ", prompt);                                      // Print the prompt message.
    if (fgets(userInput, sizeof(userInput), stdin) == NULL) return NULL;        // Get the user's input, return NULL if nothing was entered (such as CTRL+C).
    // If no '\n' (enter key) is found , read any characters out of the (fgets) input stream to prevent them from auto-populating the next input.
    if (strchr(userInput, '\n') == NULL) {
        int overflow;                                                   // Save any extra characters into a temporary variable to consume them from input memory.
        while ((overflow = getchar()) != '\n' && overflow != EOF);      // Use getchar() to read the stream until the null terminator (or the end of the stream) is found.
    }
    userInput[strcspn(userInput, "\r\n")] = '\0';   // Replace the symbol applied by submitting the prompt with a null terminator.

    printf(ANSI_RETURN ANSI_RETURN);                        // The user has submitter the prompt, move upwards by two lines to redraw the bottom of the MenuBox.
    printOffset();                                          // Print the offset if needed.
    printf("%s├", boxColor);                                // Print the left divider border between the MenuBox and the prompt areas.
    for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the divider.
    printf("┤\n");                                          // Print the right divider border.

    int promptWidth = boxWidth - 5 - strlen(PROMPT_MARK) - GetRegularCharacters(prompt);    // Determine the width available for displaying the submitted prompt.
    int inputLength = strlen(userInput);                                                    // Determine the width of the submitted prompt.
    int maxPrint = (inputLength > promptWidth) ? promptWidth : inputLength;                 // Determine whether to show the whole submission or truncate.

    printOffset();                                                              // Print the offset if needed.
    printf("│ %s " PROMPT_MARK " %.*s%*s" ANSI_RESET "%s│" ANSI_CLEAR "\n",     // Print the left border, the prompt  and marker, the submission (right-padded with spaces), then the right border.
            prompt,                                                             // (%s) Print the (entire) prompt (s - as a string of text).
            maxPrint, userInput,                                                // (%.*s) Print the userInput up to a maxPrint number of characters.
            (int)(promptWidth - maxPrint), "",                                  // (%*s) Print a (promptWidth - maxPrint) (the remaining space on the line) number of space characters.
            boxColor);                                                          // Return to the color of the box.

    printOffset();                                          // Print the offset if needed.
    printf("└");                                            // Print the bottom-left corner.
    for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the bottom border.
    printf("┘%s\n", ANSI_RESET);                            // Print the bottom-right corner, then reset console formatting to default.

    return userInput;       // Return the user's input for external use.
}

// Displays a stylized box with a header and listed content items, prompts the user for input, then reformats the box to include confirmation only.
static inline char *MenuBoxPromptSecure(char *boxColor, int boxWidth, int boxHeight, char *header, char **content, char *prompt) {
    void failure(char *reason) {
        char errorMessage[256];
        snprintf(errorMessage, sizeof(errorMessage), ANSI_RED "%s" ANSI_RESET, reason);
        char *errorBoxContent[] = {
            errorMessage,
            ANSI_RED "Error reading secure password. Terminating." ANSI_RESET,
            NULL
        };
        MenuBox(ANSI_RED, 25, 2, ANSI_BLINK "ERROR" ANSI_RESET, errorBoxContent);
    }

    int horizontalOffset = (boxWidth < CONSOLE_WIDTH) ? (CONSOLE_WIDTH - boxWidth) / 2 : 0; // Determine how many spaces to print before the box are needed to center it horizontally.
    static char userInput[256];                                                             // Track the user's response.

    // Internal helper function for printing the leading spaces for centering.
    void printOffset() { for (int i = 0; i < horizontalOffset; i++) printf(" "); }

    MenuBox(boxColor, boxWidth, boxHeight, header, content);                        // Create the top section of the prompt.
    printf("%s " PROMPT_MARK " ", prompt);                                          // Print the prompt message.

    // Get the handle which listens to user input.
    HANDLE userInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (userInputHandle == INVALID_HANDLE_VALUE) failure("Could not retrieve standard input handle.");

    // Save the current console mode (the default) so it can be restored later.
    DWORD defaultMode;
    if (!GetConsoleMode(userInputHandle, &defaultMode)) failure("Could not read console mode.");

    // Turn off input echoing (ENABLE_ECHO_INPUT) for an invisible password.
    DWORD secureMode = defaultMode & ~ENABLE_ECHO_INPUT;
    if (!SetConsoleMode(userInputHandle, secureMode)) failure("Could not disable echo flags.");

    if (fgets(userInput, sizeof(userInput), stdin) == NULL) {
        SetConsoleMode(userInputHandle, defaultMode);           // Restore the default terminal settings (turn echo back on).
        printf("\n");                                           // Simulate an enter key press from the user submitting.
        memset(userInput, 0, sizeof(userInput));                // Clear any junk data left in the userInput stream.
        printf(ANSI_RETURN ANSI_RETURN);                        // The user has submitter the prompt, move upwards by two lines to redraw the bottom of the MenuBox.
        printOffset();                                          // Print the offset if needed.
        printf("%s├", boxColor);                                // Print the left divider border between the MenuBox and the prompt areas.
        for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the divider.
        printf("┤\n");                                          // Print the right divider border.

        int promptWidth = boxWidth - 5 - strlen(PROMPT_MARK) - GetRegularCharacters(prompt);    // Determine the width available for displaying the submitted prompt.
        int inputLength = strlen(userInput);                                                    // Determine the width of the submitted prompt.
        int maxPrint = (inputLength > promptWidth) ? promptWidth : inputLength;                 // Determine whether to show the whole submission or truncate.

        printOffset();                                                              // Print the offset if needed.
        printf("│ %s " PROMPT_MARK " %.*s%*s" ANSI_RESET "%s│" ANSI_CLEAR "\n",     // Print the left border, the prompt  and marker, the submission (right-padded with spaces), then the right border.
            prompt,                                                                 // (%s) Print the (entire) prompt (s - as a string of text).
            strlen(PROMPT_DECLINE), PROMPT_DECLINE,                                 // (%.*s) Print the PROMPT_DECLINE symbol(s) up to a PROMPT_DECLINE-length number of characters.
            (int)(promptWidth - strlen(PROMPT_DECLINE)), "",                        // (%*s) Print a (promptWidth - PROMPT_DECLINE length) (the remaining space on the line) number of space characters.
            boxColor);                                                              // Return to the color of the box.

        printOffset();                                          // Print the offset if needed.
        printf("└");                                            // Print the bottom-left corner.
        for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the bottom border.
        printf("┘%s\n", ANSI_RESET);                            // Print the bottom-right corner, then reset console formatting to default.
        return NULL;                                            // Return NULL if nothing was entered (such as CTRL+C).
    }
    // If no '\n' (enter key) is found , read any characters out of the (fgets) input stream to prevent them from auto-populating the next input.
    if (strchr(userInput, '\n') == NULL) {
        int overflow;                                                   // Save any extra characters into a temporary variable to consume them from input memory.
        while ((overflow = getchar()) != '\n' && overflow != EOF);      // Use getchar() to read the stream until the null terminator (or the end of the stream) is found.
    }
    userInput[strcspn(userInput, "\r\n")] = '\0';   // Replace the symbol applied by submitting the prompt with a null terminator.

    printf(ANSI_RETURN ANSI_RETURN);                        // The user has submitter the prompt, move upwards by two lines to redraw the bottom of the MenuBox.
    printOffset();                                          // Print the offset if needed.
    printf("%s├", boxColor);                                // Print the left divider border between the MenuBox and the prompt areas.
    for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the divider.
    printf("┤\n");                                          // Print the right divider border.

    int promptWidth = boxWidth - 5 - strlen(PROMPT_MARK) - GetRegularCharacters(prompt);    // Determine the width available for displaying the submitted prompt.
    int inputLength = strlen(userInput);                                                    // Determine the width of the submitted prompt.
    int maxPrint = (inputLength > promptWidth) ? promptWidth : inputLength;                 // Determine whether to show the whole submission or truncate.

    printOffset();                                                              // Print the offset if needed.
    printf("│ %s " PROMPT_MARK " %.*s%*s" ANSI_RESET "%s│" ANSI_CLEAR "\n",     // Print the left border, the prompt  and marker, the submission (right-padded with spaces), then the right border.
        prompt,                                                                 // (%s) Print the (entire) prompt (s - as a string of text).
        strlen(PROMPT_DECLINE), PROMPT_ACCEPT,                                  // (%.*s) Print the PROMPT_ACCEPT symbol(s) up to a PROMPT_ACCEPT-length number of characters.
        (int)(promptWidth - strlen(PROMPT_ACCEPT)), "",                         // (%*s) Print a (promptWidth - PROMPT_ACCEPT length) (the remaining space on the line) number of space characters.
        boxColor);                                                              // Return to the color of the box.

    printOffset();                                          // Print the offset if needed.
    printf("└");                                            // Print the bottom-left corner.
    for (int i = 0; i < boxWidth - 2; i++) printf("─");     // Print the bottom border.
    printf("┘%s\n", ANSI_RESET);                            // Print the bottom-right corner, then reset console formatting to default.
    SetConsoleMode(userInputHandle, defaultMode);           // Restore the default terminal settings (turn echo back on).

    return userInput;       // Return the user's input for external use.
}

// Displays a stylized loading bar with a label and a percentage readout (compatible with flows).
static inline void ProgressBar(char *barColor, int maxBarWidth, const char *label, long current, long total, int isUpdate) {
    if (total <= 0) return;                         // If there is no total (no bar to print), return to avoid division by zero when determining percentage.
    if (isUpdate) printf(ANSI_RETURN);              // If updating the existing bar, move the cursor back up and erase the line to overwrite it.

    int percentage = (int)((current * 100LL) / total);      // Determine the percentage of the loading bar to fill.
    if (percentage > 100) percentage = 100;                 // Cap the percentage at 100%.

    int indent = 8;                                                 // Account for spaces and percentage.
    int labelLength = GetRegularCharacters(label);                  // Determine the length of the label.
    int flowOffset = insideFlow ? (1 + flowWidth) : 0;              // Determine whether or not to offset (if a flow is active).
    int availableWidth = CONSOLE_WIDTH - indent - flowOffset;       // Determine the width available for printing the loading bar.

    int minBarWidth = 4;                                                              // Set the minimum amount of loading bar ticks (25% increments).
    int maxLabelWidth = availableWidth - minBarWidth;                                 // Determine the maximum amount of room available for the label.
    int textRoom = (labelLength > maxLabelWidth) ? maxLabelWidth : labelLength;       // Determine whether to truncate the label or not based on available space.

    int remainingSpace = availableWidth - textRoom;                                         // Determine the amount of leftover space available to print the bar.
    int activeBarWidth = (maxBarWidth < remainingSpace) ? maxBarWidth : remainingSpace;     // Determine whether to truncate the bar or not based on available space.

    char adjustedText[256] = {0};       // Store the label string in case it needs to be shortened.
    //  If the label is truncated, add an ellipsis (...) to indicate.
    if (labelLength > maxLabelWidth && maxLabelWidth > 3) {
        strncpy(adjustedText, label, maxLabelWidth - 3);
        strcat(adjustedText, "...");
    // Copy the label to the adjusted text, then add a null terminator.
    } else {
        strncpy(adjustedText, label, textRoom);
        adjustedText[textRoom] = '\0';
    }

    int filledLength = (percentage * activeBarWidth) / 100;     // Determine how many ticks should be colored (filled) and how many are unfilled (empty).

    if (insideFlow) printf("%s│%*s" ANSI_RESET, currentFlowColor, flowWidth, "");       // If a flow is active, print the vertical separator line first and indent.

    // Print the label, the opening bracket for the bar, and set the color for the progress part
    printf("%s%s" ANSI_RESET ANSI_DIM " [" ANSI_RESET, barColor, adjustedText);

    // Loop through the bar's width, printing a filled tick for progress or an empty tick for remaining work.
    for (int i = 0; i < activeBarWidth; i++) {
        if (i < filledLength) printf("%s" BAR_FILLED ANSI_RESET, barColor);
        else printf(ANSI_DIM BAR_EMPTY ANSI_RESET);
    }

    printf(ANSI_DIM "]" ANSI_RESET " %s%3d%%" ANSI_RESET "  \n", barColor, percentage);   // Print the closing bracket and the percentage number.

    fflush(stdout); // Force the output to appear on the screen immediately.
}

/*
    Examples of progress bars inside of flows (with CONSOLE_WIDTH set to 52):

    FlowStart(ANSI_BLUE, 2); // Start a blue flow.
    FlowAdd("Initializing progress bar...");

    ProgressBar(GetRGBColor(64, 255, 64), 256, "PROGRESS: ", 0, 100, 0); // Initial creation of the bar (isUpdate = 0).

    // Simulate progress updates
    for (int i = 1; i <= 100; i ++) {
        Sleep(50);                                                              // Wait 50 milliseconds (0.05 seconds) to simulate processing.
        ProgressBar(GetRGBColor(64, 255, 64), 256, "PROGRESS: ", i, 100, 1);     // Update the loading bar (isUpdate = 1).
    }
    ProgressBar(GetRGBColor(128, 255, 128), 256, "PROGRESS: ", 100, 100, 1);     // Update the loading bar with a brighter color once it completes.

    FlowAdd("Bar complete!");   // Add a completion message.
    FlowFinish();               // Finish the flow.

┌──Initializing progress bar...
│  PROGRESS:  [●●●●●●●●●●●●●●●●●●●●●●●●●●●●●●●] 100%
└──Bar complete!

*/

#endif // ANSI_FANCY_H_INCLUDED
