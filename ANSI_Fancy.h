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

    int effectiveWidth = CONSOLE_WIDTH - (1 + flowWidth); // Calculate space left for actual text (next to the connectors).
    int contentLength = (int)strlen(content);             // Tracks the overall length of the content.
    int cursor = 0;                                       // Track the current position while scanning through content.
    int linesUsed = 0;                                    // Tracks lines used if the text wraps (too long for one row).

    // Loop through the content to handle word wrapping.
    while (cursor < contentLength) {
        int remaining = contentLength - cursor;                                         // Track the number of remaining characters.
        int chunkLength = (remaining <= effectiveWidth) ? remaining : effectiveWidth;   // Expect the length of the current chunk to be either the max allowed or less.

        // Try to find a space to break the line along, rather than cutting a word in half.
        if (remaining > effectiveWidth) {
            int breakPoint = -1;                            // Initialize the break point outside of bounds, in case it is not used.
            // Scan from the end of the chunk backwards looking for a whitespace character to use as a break point.
            for (int i = chunkLength - 1; i >= 0; i--) {
                if (content[cursor + i] == ' ' || content[cursor + i] == '\n' || content[cursor + i] == '\r' || content[cursor + i] == '\t') {
                    breakPoint = i;
                    break;
                }
            }
            /*
            // Custom symbols to allow as break points can be specified here:
            // Note that when the symbol is used as a break point, it can removed from the content or included.
            for (int i = chunkLength - 1; i >= 0; i--) {
                if (content[cursor + i] == '.' || content[cursor + i] == ',' || content[cursor + i] == ';') {
                    breakPoint = i;
                    break;
                }
            }
            */
            if (breakPoint != -1) chunkLength = breakPoint; // If no whitespace is found, break at the max length.
        }

        // Print the start of the line with the correct connector (top-left-corner or t-bar).
        if (linesUsed == 0) {
            const char *firstSymbol = (flowRow == 0) ? "┌" : "├";                       // Determine if this is the start of a new flow or the middle of an existing one.
            printf("%s%s", currentFlowColor, firstSymbol);                              // Apply the color to the connector.
            for(int i = 0; i < flowWidth; i++) printf("─");                             // Print the horizontal connector lines.
            printf(ANSI_RESET "%.*s" ANSI_CLEAR "\n", chunkLength, content + cursor);   // Reset color, print the text (truncated at chunkLength), and clear the rest of the line.
        // Print a continuation line to indicate wrapping text (no horizontal connectors, just a vertical bar).
        } else {
            printf("%s│%*s", currentFlowColor, flowWidth, "");                          // Print the vertical connector and indent.
            printf(ANSI_RESET "%.*s" ANSI_CLEAR "\n", chunkLength, content + cursor);   // Reset color, print the text (truncated at chunkLength), and clear the rest of the line.
        }

        cursor += chunkLength;
        if (
            (cursor < contentLength && content[cursor] == ' ') ||
            (cursor < contentLength && content[cursor] == '\n') ||
            (cursor < contentLength && content[cursor] == '\r') ||
            (cursor < contentLength && content[cursor] == '\t')
            /*
            // Custom symbols can be specified here to remove them after using as a breakpoint.
            || (cursor < contentLength && content[cursor] == '.')
            */
        ) cursor++;     // Skip the whitepsace used as a breakpoint.
        linesUsed++;    // Increment the number of lines used by the content.
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
        printf(ANSI_RETURN ANSI_ERASE); // Move up one line and erase the whole line.
    }

    char *lastContent = flowContent[flowRow - 1];
    int indentLength = 1 + flowWidth;
    int effectiveWidth = CONSOLE_WIDTH - (1 + flowWidth); // Calculate space left for actual text (next to the connectors).
    int contentLength = (int)strlen(lastContent);         // Tracks the overall length of the content.
    int cursor = 0;                                       // Track the current position while scanning through content.
    int linesUsed = 0;                                    // Tracks lines used if the text wraps (too long for one row).

    // Repeat the adding process to re-write the bottom of the flow.
    while (cursor < contentLength) {
        int remaining = contentLength - cursor;                                         // Track the number of remaining characters.
        int chunkLength = (remaining <= effectiveWidth) ? remaining : effectiveWidth;   // Expect the length of the current chunk to be either the max allowed or less.

        // Try to find a space to break the line along, rather than cutting a word in half.
        if (remaining > effectiveWidth) {
            int breakPoint = -1;                            // Initialize the break point outside of bounds, in case it is not used.
            // Scan from the end of the chunk backwards looking for a whitespace character to use as a break point.
            for (int i = chunkLength - 1; i >= 0; i--) {
                if (lastContent[cursor + i] == ' ' || lastContent[cursor + i] == '\n' || lastContent[cursor + i] == '\r' || lastContent[cursor + i] == '\t') {
                    breakPoint = i;
                    break;
                }
            }
            /*
            // Custom symbols to allow as break points can be specified here:
            // Note that when the symbol is used as a break point, it can removed from the content or included.
            for (int i = chunkLength - 1; i >= 0; i--) {
                if (content[cursor + i] == '.' || content[cursor + i] == ',' || content[cursor + i] == ';') {
                    breakPoint = i;
                    break;
                }
            }
            */
            if (breakPoint != -1) chunkLength = breakPoint; // If no whitespace is found, break at the max length.
        }

        // If only one entry was added to the flow, draw its connector as a horizontal line.
        if (linesUsed == 0 && flowRow == 1) {
            printf("%s─", currentFlowColor);                                        // Use the horizontal line character.
            for(int i = 0; i < flowWidth; i++) printf("─");
            printf(ANSI_RESET "%.*s" ANSI_CLEAR "\n", chunkLength, lastContent + cursor);
        }
        // Print the final of multiple entries with the correct connector (bottom-left-corner).
        else if (linesUsed == 0) {
            printf("%s└", currentFlowColor);                                                // Use the bottom-left-corner character.
            for(int i = 0; i < flowWidth; i++) printf("─");                                 // Print the horizontal connector lines.
            printf(ANSI_RESET "%.*s" ANSI_CLEAR "\n", chunkLength, lastContent + cursor);   // Reset color, print the text (truncated at chunkLength), and clear the rest of the line.
        // Print an indent to indicate wrapping text (no horizontal connectors, or vertical bar).
        } else {
            printf("%s%*s", currentFlowColor, indentLength, "");                            // Print the indent.
            printf(ANSI_RESET "%.*s" ANSI_CLEAR "\n", chunkLength, lastContent + cursor);   // Reset color, print the text (truncated at chunkLength), and clear the rest of the line.
        }

        cursor += chunkLength;
        if (
            (cursor < contentLength && lastContent[cursor] == ' ') ||
            (cursor < contentLength && lastContent[cursor] == '\n') ||
            (cursor < contentLength && lastContent[cursor] == '\r') ||
            (cursor < contentLength && lastContent[cursor] == '\t')
            /*
            // Custom symbols can be specified here to remove them after using as a breakpoint.
            || (cursor < contentLength && lastContent[cursor] == '.')
            */
        ) cursor++;     // Skip the whitepsace used as a breakpoint.
        linesUsed++;    // Increment the number of lines used by the content.
    }

    // Free up the used memory and reset the global flow tracker variables.
    for (int i = 0; i < flowRow; i++) {
        free(flowContent[i]);
        flowContent[i] = NULL;
    }
    insideFlow = 0;
    flowRow = 0;
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

#endif // ANSI_FANCY_H_INCLUDED
