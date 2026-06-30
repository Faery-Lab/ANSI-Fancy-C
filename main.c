#include <stdio.h>
#include <stdlib.h>

#include "ANSI_Fancy.h"

// Demo for ANSI_Fancy commands
int main()
{
    InitializeConsole();

    printf(ANSI_YELLOW "FlowStart(ANSI_RED, 0);" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Entry 1 has some really long text which causes it to wrap around in this example quite substantially by two rows.\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowFinish();" ANSI_RESET "\n\n");

    FlowStart(ANSI_RED, 0);
    FlowAdd("Entry 1 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowFinish();

    printf("\n");

    printf(ANSI_YELLOW "FlowStart(ANSI_GREEN, 3);" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Layer 1\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Layer 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowFinish();" ANSI_RESET "\n\n");

    FlowStart(ANSI_GREEN, 3);
    FlowAdd("Layer 1");
    FlowAdd("Layer 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowFinish();

    printf("\n");

    printf(ANSI_YELLOW "FlowStart(ANSI_BLUE, 5);" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Layer 1\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Layer 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowAdd(\"Layer 3\");" ANSI_RESET "\n"
           ANSI_YELLOW "FlowFinish();" ANSI_RESET "\n\n");

    FlowStart(ANSI_BLUE, 5);
    FlowAdd("Layer 1");
    FlowAdd("Layer 2 has some really long text which causes it to wrap around in this example quite substantially by two rows.");
    FlowAdd("Layer 3");
    FlowFinish();
}
