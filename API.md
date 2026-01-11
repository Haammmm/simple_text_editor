# Plugin API Documentation / 插件 API 文档

## Table of Contents
- [Overview](#overview)
- [EditorAPI Interface](#editorapi-interface)
- [Plugin Initialization](#plugin-initialization)
- [Command Registration](#command-registration)
- [Buffer Operations](#buffer-operations)
- [File Operations](#file-operations)
- [Network Operations](#network-operations)
- [UI Operations](#ui-operations)
- [Plugin Examples](#plugin-examples)
- [Best Practices](#best-practices)

## Overview

The SimpleTextEditor plugin system allows developers to extend the editor's functionality through DLL plugins. Plugins interact with the editor via the `EditorAPI` interface, which provides access to the text buffer, file I/O, HTTP requests, and UI utilities.

### Key Features
- **Dynamic Loading**: Plugins are loaded at runtime from the `./plugins` directory
- **Command Registration**: Register custom commands with descriptions
- **Buffer Access**: Read, modify, insert, and delete lines in the text buffer
- **File I/O**: Read and write files from within plugins
- **HTTP Support**: Make HTTP GET requests
- **UI Integration**: Print messages and control screen output

## EditorAPI Interface

The `EditorAPI` structure provides the following function pointers:

```c
typedef struct EditorAPI {
    int (*get_line_count)(void);
    const char* (*get_line)(int line_num);
    int (*insert_line)(int line_num, const char* text);
    int (*delete_line)(int line_num);
    int (*replace_line)(int line_num, const char* text);
    void (*print_msg)(const char* msg);
    void (*clear_screen)(void);
    void (*register_command)(const char* name, void (*func)(void), const char* desc);
    int (*read_file)(const char* path, char* out, size_t out_sz);
    int (*write_file)(const char* path, const char* data);
    int (*http_get)(const char* url, char* out, size_t out_sz);
} EditorAPI;
```

## Plugin Initialization

### Required Export Function

Every plugin DLL must export a `PluginInit` function with the following signature:

```c
extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api);
}
```

### Parameters
- `api`: Pointer to the EditorAPI interface

### Return Value
- `0`: Success
- `-1`: Failure (plugin will not be loaded)

### Example

```c
static EditorAPI* g_api = nullptr;

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    
    // Store the API pointer for later use
    g_api = api;
    
    // Register plugin commands
    api->register_command("MyCommand", my_command_function, "Description of my command");
    
    return 0;
}
}
```

## Command Registration

### Function: register_command

```c
void register_command(const char* name, void (*func)(void), const char* desc);
```

Registers a new command that users can execute from the plugin management menu.

#### Parameters
- `name`: Command name (max 31 characters)
- `func`: Function pointer to the command implementation
- `desc`: Command description (max 63 characters)

#### Notes
- Command names must be unique
- Commands are executed when the user selects them from the plugin menu
- Command functions take no parameters and return void

#### Example

```c
void my_custom_command(void) {
    if (!g_api) return;
    g_api->print_msg("Executing my custom command!\n");
    
    int lines = g_api->get_line_count();
    char msg[128];
    snprintf(msg, sizeof(msg), "Buffer has %d lines\n", lines);
    g_api->print_msg(msg);
}

// In PluginInit:
api->register_command("MyCustomCommand", my_custom_command, "Execute my custom command");
```

## Buffer Operations

### Function: get_line_count

```c
int get_line_count(void);
```

Returns the current number of lines in the text buffer.

#### Return Value
- Number of lines (0 to MAX_LINES)

#### Example
```c
int count = g_api->get_line_count();
printf("Buffer contains %d lines\n", count);
```

---

### Function: get_line

```c
const char* get_line(int line_num);
```

Retrieves a specific line from the text buffer.

#### Parameters
- `line_num`: Zero-based line index (0 to line_count-1)

#### Return Value
- Pointer to the line content (read-only)
- `NULL` if line_num is out of bounds

#### Notes
- The returned pointer is valid until the buffer is modified
- Lines are null-terminated strings
- Maximum line length is MAX_LINE_LENGTH (4096 bytes)

#### Example
```c
int count = g_api->get_line_count();
for (int i = 0; i < count; i++) {
    const char* line = g_api->get_line(i);
    if (line) {
        printf("Line %d: %s\n", i + 1, line);
    }
}
```

---

### Function: insert_line

```c
int insert_line(int line_num, const char* text);
```

Inserts a new line at the specified position.

#### Parameters
- `line_num`: Zero-based position where the line will be inserted (0 to line_count)
- `text`: Content of the new line (null-terminated string)

#### Return Value
- `0`: Success
- `-1`: Failure (invalid parameters, buffer full, or line too long)

#### Notes
- Existing lines at and after `line_num` are shifted down
- Maximum buffer capacity is MAX_LINES (1000 lines)
- Maximum line length is MAX_LINE_LENGTH (4096 bytes)
- Text exceeding the limit will be truncated

#### Example
```c
// Insert at the beginning
if (g_api->insert_line(0, "First line") == 0) {
    g_api->print_msg("Line inserted successfully\n");
}

// Append at the end
int count = g_api->get_line_count();
g_api->insert_line(count, "Last line");
```

---

### Function: delete_line

```c
int delete_line(int line_num);
```

Deletes a line from the text buffer.

#### Parameters
- `line_num`: Zero-based line index to delete (0 to line_count-1)

#### Return Value
- `0`: Success
- `-1`: Failure (invalid line number)

#### Notes
- Lines after the deleted line are shifted up
- The buffer's modified flag is set

#### Example
```c
// Delete the first line
if (g_api->delete_line(0) == 0) {
    g_api->print_msg("First line deleted\n");
}

// Delete the last line
int count = g_api->get_line_count();
if (count > 0) {
    g_api->delete_line(count - 1);
}
```

---

### Function: replace_line

```c
int replace_line(int line_num, const char* text);
```

Replaces the content of an existing line.

#### Parameters
- `line_num`: Zero-based line index (0 to line_count-1)
- `text`: New content for the line

#### Return Value
- `0`: Success
- `-1`: Failure (invalid parameters or line too long)

#### Notes
- More efficient than delete + insert
- Maximum line length is MAX_LINE_LENGTH (4096 bytes)
- Text exceeding the limit will be truncated

#### Example
```c
if (g_api->replace_line(0, "New first line content") == 0) {
    g_api->print_msg("Line replaced successfully\n");
}
```

## File Operations

### Function: read_file

```c
int read_file(const char* path, char* out, size_t out_sz);
```

Reads the entire content of a file into a buffer.

#### Parameters
- `path`: File path (relative or absolute)
- `out`: Output buffer
- `out_sz`: Size of the output buffer

#### Return Value
- Number of bytes read (>= 0): Success
- `-1`: Failure (file not found, access denied, or buffer too small)

#### Notes
- Binary mode read
- File content is null-terminated if space permits
- Caller must provide sufficient buffer space

#### Example
```c
char buffer[8192];
int bytes = g_api->read_file("config.txt", buffer, sizeof(buffer));
if (bytes > 0) {
    char msg[128];
    snprintf(msg, sizeof(msg), "Read %d bytes from file\n", bytes);
    g_api->print_msg(msg);
}
```

---

### Function: write_file

```c
int write_file(const char* path, const char* data);
```

Writes data to a file, creating or overwriting it.

#### Parameters
- `path`: File path (relative or absolute)
- `data`: Null-terminated string to write

#### Return Value
- Number of bytes written (>= 0): Success
- `-1`: Failure (access denied, disk full, etc.)

#### Notes
- Binary mode write
- Creates the file if it doesn't exist
- Overwrites existing files

#### Example
```c
const char* content = "Hello, World!";
if (g_api->write_file("output.txt", content) >= 0) {
    g_api->print_msg("File written successfully\n");
}
```

## Network Operations

### Function: http_get

```c
int http_get(const char* url, char* out, size_t out_sz);
```

Performs a synchronous HTTP GET request.

#### Parameters
- `url`: Full URL (must start with `http://` or `https://`)
- `out`: Output buffer for the response body
- `out_sz`: Size of the output buffer

#### Return Value
- Number of bytes received (>= 0): Success
- `-1`: Failure (network error, invalid URL, or buffer too small)

#### Notes
- Synchronous operation (blocks until complete)
- Uses WinHTTP (Windows only)
- Response is null-terminated if space permits
- Supports both HTTP and HTTPS
- No proxy authentication support

#### Example
```c
char response[4096];
int bytes = g_api->http_get("https://api.example.com/data", response, sizeof(response));
if (bytes > 0) {
    g_api->print_msg("HTTP request successful\n");
    g_api->print_msg(response);
}
```

## UI Operations

### Function: print_msg

```c
void print_msg(const char* msg);
```

Prints a message to the console.

#### Parameters
- `msg`: Message string to display

#### Notes
- Output goes to stdout
- No automatic newline appended (include `\n` if needed)
- UTF-8 strings are supported

#### Example
```c
g_api->print_msg("Processing...\n");
g_api->print_msg("Done!\n");
```

---

### Function: clear_screen

```c
void clear_screen(void);
```

Clears the console screen.

#### Notes
- Uses `system("cls")` on Windows
- Use sparingly to avoid flickering

#### Example
```c
g_api->clear_screen();
g_api->print_msg("Screen cleared\n");
```

## Plugin Examples

### Example 1: Line Counter Plugin

```c
#include "plugin.h"

static EditorAPI* g_api = nullptr;

void count_lines_command(void) {
    if (!g_api) return;
    
    int count = g_api->get_line_count();
    char msg[128];
    snprintf(msg, sizeof(msg), "Total lines: %d\n", count);
    g_api->print_msg(msg);
}

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("CountLines", count_lines_command, "Count total lines in buffer");
    return 0;
}
}
```

### Example 2: Text Reverser Plugin

```c
#include "plugin.h"
#include <string.h>
#include <algorithm>

static EditorAPI* g_api = nullptr;

void reverse_lines_command(void) {
    if (!g_api) return;
    
    int count = g_api->get_line_count();
    if (count == 0) {
        g_api->print_msg("Buffer is empty\n");
        return;
    }
    
    for (int i = 0; i < count; i++) {
        const char* line = g_api->get_line(i);
        if (line) {
            std::string reversed(line);
            std::reverse(reversed.begin(), reversed.end());
            g_api->replace_line(i, reversed.c_str());
        }
    }
    
    g_api->print_msg("All lines reversed!\n");
}

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("ReverseLines", reverse_lines_command, "Reverse each line in buffer");
    return 0;
}
}
```

### Example 3: File Append Plugin

```c
#include "plugin.h"
#include <iostream>

static EditorAPI* g_api = nullptr;

void append_file_command(void) {
    if (!g_api) return;
    
    std::cout << "Enter filename to append: ";
    char filename[256];
    if (!std::cin.getline(filename, sizeof(filename))) return;
    
    char content[8192];
    int bytes = g_api->read_file(filename, content, sizeof(content));
    if (bytes < 0) {
        g_api->print_msg("Failed to read file\n");
        return;
    }
    
    // Parse lines and append to buffer
    const char* p = content;
    const char* line_start = p;
    int count = g_api->get_line_count();
    
    while (*p) {
        if (*p == '\n') {
            size_t len = p - line_start;
            char line[4096];
            if (len < sizeof(line)) {
                memcpy(line, line_start, len);
                line[len] = '\0';
                g_api->insert_line(count++, line);
            }
            line_start = p + 1;
        }
        p++;
    }
    
    g_api->print_msg("File content appended to buffer\n");
}

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("AppendFile", append_file_command, "Append file content to buffer");
    return 0;
}
}
```

## Best Practices

### Memory Management
- **Never free** pointers returned by `get_line()` - they are managed by the editor
- Store the EditorAPI pointer globally for command functions to access
- Be cautious with large allocations - plugins share memory space with the editor

### Error Handling
- Always check return values from API functions
- Validate parameters before passing to API functions
- Provide informative error messages to users via `print_msg()`

### Performance
- Minimize buffer modifications in loops
- Use `replace_line()` instead of `delete_line()` + `insert_line()`
- Cache `get_line_count()` results if the value won't change

### User Interaction
- Use `std::cout` or `printf` for user prompts
- Use `print_msg()` for status messages
- Consider using `clear_screen()` judiciously

### UTF-8 Support
- All text in the buffer is UTF-8 encoded
- Use UTF-8 aware string functions when manipulating text
- Line and column indices in the buffer are character-based, not byte-based

### Command Design
- Keep command names short and descriptive
- Provide clear, concise descriptions
- Commands should be self-contained and not rely on external state

### Testing
- Test plugins with empty buffers
- Test with maximum buffer size (1000 lines)
- Test with long lines (near 4096 character limit)
- Test with UTF-8 content (Chinese, emoji, etc.)

### Security
- Validate all user input
- Be cautious with file operations - check paths
- Limit network operations to trusted endpoints
- Avoid executing arbitrary code from user input

## Constants and Limits

```c
#define MAX_LINE_LENGTH  4096   // Maximum bytes per line
#define MAX_LINES        1000   // Maximum number of lines
#define MAX_FILENAME     256    // Maximum filename length
#define BUFFER_SIZE      4096   // General buffer size
```

## Compilation

### Visual Studio Settings
1. **Configuration Type**: Dynamic Library (.dll)
2. **Additional Dependencies**: `winhttp.lib` (if using HTTP functions)
3. **C++ Language Standard**: C++20 (for C++ plugins)
4. **C Language Standard**: C17 (for C plugins)

### Example Build Command
```cmd
cl /LD /EHsc /std:c++20 my_plugin.cpp /link winhttp.lib
```

## Deployment

1. Compile your plugin to a `.dll` file
2. Copy the DLL to the `./plugins` directory (relative to the executable)
3. Launch the editor
4. Select **Plugin Management** from the main menu
5. Choose **Scan and load plugins from plugins directory**
6. Your commands will appear in the plugin command list

## Troubleshooting

### Plugin Not Loading
- Verify the DLL exports `PluginInit` function
- Check that the DLL architecture (x86/x64) matches the executable
- Ensure the plugin returns 0 from `PluginInit`

### Commands Not Appearing
- Verify `register_command()` is called in `PluginInit`
- Check command name length (max 31 characters)
- Ensure command names are unique

### Buffer Operations Failing
- Check line indices are within valid range (0 to line_count-1)
- Verify line content doesn't exceed MAX_LINE_LENGTH
- Ensure buffer has space for new lines (< MAX_LINES)

### HTTP Requests Failing
- Verify URL starts with `http://` or `https://`
- Check network connectivity
- Ensure output buffer is large enough for response

## API Version

- **Current Version**: 1.0
- **Compatibility**: Windows only (WinHTTP dependency)
- **Threading**: Single-threaded (plugins must not create threads)

## Future API Enhancements (Planned)

The following features may be added in future versions:
- Undo/redo support
- Search and replace from API
- Character statistics API
- Event callbacks (on text change, file save, etc.)
- Multi-line operations (insert/delete ranges)
- Cursor position tracking
- Configuration file access

---

**Note**: This API is subject to change. Always check the latest documentation and header files for the most current information.
