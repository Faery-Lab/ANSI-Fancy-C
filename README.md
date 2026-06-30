# 🎨 ANSI Fancy C

A header-only C library for Windows to create colorized formatted text in the command-line interface.

---

## 📖 Overview

This project provides a simple way to format terminal output with box-drawing characters and color support. It includes automatic word-wrapping and structural management, perfect for CLI tools that need to present hierarchical data or stepwise logs.

### ✨ Features
* **Flows:** Create grouped, connected lists of text with customizable indentation.

---

## 📂 File Structure

| File | Description |
| :--- | :--- |
| `ANSI_Fancy_Config.h` | Contains size settings, ANSI escape sequence definitions, and console initialization. |
| `ANSI_Fancy.h` | The core library containing the Flow logic and rendering engine. |
| `main.c` | A demonstration program showing how to use the library's features. |

---

## 🚀 Quick Start

1.  Include `ANSI_Fancy.h` in your project.
2.  Call `InitializeConsole()` at the very beginning of your `main` function.
3.  Use the `Flow` functions to structure your output:

```c
#include "ANSI_Fancy.h"

int main() {
    // Enable ANSI escape sequences and UTF-8 output
    InitializeConsole();
    
    // Start a blue flow with 5 units of indent
    FlowStart(ANSI_BLUE, 5);
    
    FlowAdd("This is the first entry.");
    FlowAdd("This entry is longer and will automatically wrap if it exceeds the CONSOLE_WIDTH.");
    
    // Close the flow with the bottom connector
    FlowFinish();
    
    return 0;
}
```
Output (with CONSOLE_WIDTH set to 52):

<pre>
    <span style="color:blue">┌─────</span>This is the first entry.<br/>
    <span style="color:blue">└─────</span>This entry is longer and will automatically<br/>
    <span style="color:blue">      </span>wrap if it exceeds the CONSOLE_WIDTH.
</pre>
