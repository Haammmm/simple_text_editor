# Architecture and Design Documentation

## Table of Contents
- [System Overview](#system-overview)
- [Architecture Diagram](#architecture-diagram)
- [Core Components](#core-components)
- [Design Patterns](#design-patterns)
- [Data Structures](#data-structures)
- [Algorithm Choices](#algorithm-choices)
- [Memory Management](#memory-management)
- [Plugin Architecture](#plugin-architecture)
- [Error Handling Strategy](#error-handling-strategy)
- [Security Considerations](#security-considerations)

## System Overview

SimpleTextEditor is designed as a modular, extensible console-based text editor written in C/C++. The architecture follows a layered approach with clear separation of concerns:

1. **Presentation Layer**: Console UI and menu system
2. **Business Logic Layer**: Text editing operations and algorithms
3. **Data Layer**: Text buffer management
4. **Plugin Layer**: Dynamic extensibility through DLL plugins

### Design Philosophy

- **Simplicity**: Keep the core functionality focused and minimal
- **Safety**: Use secure CRT functions and bounds checking
- **Extensibility**: Allow feature additions without modifying core code
- **Performance**: Efficient algorithms (KMP search, UTF-8 handling)
- **Portability**: Core logic is portable; platform-specific code is isolated

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                        User Interface                       │
│                         (main.c)                            │
│  ┌─────────┬─────────┬─────────┬─────────┬──────────┐     │
│  │  Input  │  Open   │  Save   │  Stats  │  Plugin  │     │
│  │  Text   │  File   │  File   │  Char   │  Mgmt    │ ... │
│  └────┬────┴────┬────┴────┬────┴────┬────┴────┬─────┘     │
└───────┼─────────┼─────────┼─────────┼─────────┼───────────┘
        │         │         │         │         │
        ▼         ▼         ▼         ▼         ▼
┌───────────────────────────────────────────────────────────┐
│              Text Editor Core (text_editor.c/h)           │
│  ┌──────────────────────────────────────────────────┐    │
│  │              Text Buffer (TextBuffer)            │    │
│  │  - lines[MAX_LINES][MAX_LINE_LENGTH]            │    │
│  │  - line_count, modified, filename                │    │
│  └──────────────────────────────────────────────────┘    │
│                                                           │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │   Buffer    │  │    File      │  │   Search &   │   │
│  │ Operations  │  │  Operations  │  │   Modify     │   │
│  │             │  │              │  │              │   │
│  │ - insert    │  │ - open       │  │ - find       │   │
│  │ - delete    │  │ - save       │  │ - replace    │   │
│  │ - replace   │  │              │  │ - delete     │   │
│  └─────────────┘  └──────────────┘  └──────────────┘   │
│                                                           │
│  ┌─────────────┐  ┌──────────────┐                      │
│  │   UTF-8     │  │  Character   │                      │
│  │  Handling   │  │  Statistics  │                      │
│  │             │  │              │                      │
│  │ - strlen    │  │ - count      │                      │
│  │ - offset    │  │ - classify   │                      │
│  └─────────────┘  └──────────────┘                      │
└───────────────────────────────────────────────────────────┘
        │                                   │
        │                                   │
        ▼                                   ▼
┌──────────────────────────┐     ┌─────────────────────────┐
│   Plugin Manager         │◄────┤   Plugin System         │
│  (plugin_manager.c/h)    │     │    (plugin.h)           │
│                          │     │                         │
│ - Load DLLs              │     │  EditorAPI Interface:   │
│ - Register commands      │     │  - get_line_count()     │
│ - Execute commands       │     │  - get_line()           │
│ - API implementation     │     │  - insert_line()        │
│                          │     │  - delete_line()        │
│ ┌──────────────────┐    │     │  - replace_line()       │
│ │  HTTP Client     │    │     │  - print_msg()          │
│ │  (WinHTTP)       │    │     │  - file I/O             │
│ └──────────────────┘    │     │  - http_get()           │
└──────────────────────────┘     └─────────────────────────┘
        │                                   │
        │                                   │
        ▼                                   ▼
┌──────────────────────────┐     ┌─────────────────────────┐
│   Plugin DLLs            │     │  Example: LLM Plugin    │
│   (./plugins/*.dll)      │     │  (Dll1/openai_agent.cpp)│
│                          │     │                         │
│ - Custom commands        │     │ - LLMDialog command     │
│ - Extended features      │     │ - OpenAI API calls      │
│ - Domain-specific tools  │     │ - Buffer manipulation   │
└──────────────────────────┘     └─────────────────────────┘
```

## Core Components

### 1. Text Buffer (`TextBuffer`)

**Location**: `text_editor.h`, `text_editor.c`

**Responsibilities**:
- Store text content as an array of lines
- Track modification state
- Maintain current filename

**Structure**:
```c
typedef struct {
    char lines[MAX_LINES][MAX_LINE_LENGTH + 1];  // Fixed-size 2D array
    int line_count;                               // Current number of lines
    int modified;                                 // Dirty flag
    char filename[MAX_FILENAME];                  // Associated file
} TextBuffer;
```

**Design Rationale**:
- Fixed-size arrays for predictable memory usage
- No dynamic allocation for simplicity and safety
- Maximum capacity is known at compile time

### 2. Text Editor Core

**Location**: `text_editor.c`

**Key Functions**:
- **Buffer Management**: `buffer_init()`, `buffer_clear()`
- **File I/O**: `file_open()`, `file_save()`, `file_save_current()`
- **Search**: `find_substring_count()`, `find_all_occurrences()`
- **Modification**: `insert_substring()`, `replace_at_position()`, `delete_substring()`
- **Statistics**: `count_characters()`

**Design Principles**:
- Each function has a single, well-defined responsibility
- Extensive parameter validation
- Consistent error return codes (0 = success, -1 = failure)
- UTF-8 awareness throughout

### 3. Plugin Manager

**Location**: `plugin_manager.c`, `plugin_manager.h`

**Responsibilities**:
- Scan and load DLL plugins from the plugins directory
- Maintain a registry of plugin commands
- Execute plugin commands
- Provide EditorAPI implementation
- Manage plugin lifecycle

**Key Components**:
- **DLL Loader**: Uses Windows LoadLibrary/GetProcAddress
- **Command Registry**: Linked list of PluginCommand structures
- **API Bridge**: Implements EditorAPI interface functions
- **HTTP Client**: Provides HTTP GET functionality via WinHTTP

### 4. User Interface

**Location**: `main.c`

**Responsibilities**:
- Display menus and prompts
- Process user input
- Coordinate operations between modules
- Display results and feedback

**Design Features**:
- Menu-driven interface for simplicity
- Input validation and error handling
- Clear visual feedback (borders, separators)
- Confirmation dialogs for destructive operations

### 5. Plugin Interface

**Location**: `plugin.h`

**Responsibilities**:
- Define the contract between editor and plugins
- Specify EditorAPI structure
- Define plugin initialization signature

**Key Types**:
- `EditorAPI`: Function pointer table for plugin operations
- `PluginCommand`: Command metadata and execution
- `PluginInitFunc`: Standard plugin entry point

## Design Patterns

### 1. Plugin Pattern (Strategy + Factory)

**Purpose**: Allow runtime extensibility without modifying core code

**Implementation**:
- Plugins implement a standard initialization function
- EditorAPI acts as a service locator
- Commands are registered dynamically
- Plugin manager acts as a factory for command execution

**Benefits**:
- Open/Closed Principle: Open for extension, closed for modification
- Loose coupling between core and extensions
- Runtime flexibility

### 2. Function Pointer Table (Virtual Method Table)

**Purpose**: Provide polymorphic behavior in C

**Implementation**:
```c
typedef struct EditorAPI {
    int (*get_line_count)(void);
    const char* (*get_line)(int line_num);
    // ... more function pointers
} EditorAPI;
```

**Benefits**:
- Similar to C++ virtual functions
- Allows plugins to access core functionality
- Versioning flexibility (can add functions without breaking existing plugins)

### 3. Callback Pattern

**Purpose**: Decouple UI operations from plugin logic

**Implementation**:
```c
typedef void (*PluginLogFunc)(const char* msg);
```

**Benefits**:
- Plugins can output messages without knowing about console I/O
- Future UI changes don't affect plugins
- Testability (mock log functions)

### 4. Command Pattern

**Purpose**: Encapsulate requests as objects

**Implementation**:
```c
typedef struct PluginCommand {
    char name[32];
    void (*func)(void);
    char description[64];
    // ...
    struct PluginCommand* next;
} PluginCommand;
```

**Benefits**:
- Commands are first-class objects
- Easy to list, filter, and execute
- Extensible through linked list

## Data Structures

### 1. Fixed-Size Text Buffer

**Choice**: 2D fixed-size array

**Trade-offs**:
- ✓ Simple memory management
- ✓ Predictable memory usage
- ✓ Fast random access
- ✗ Fixed capacity limits
- ✗ Memory waste for small documents

**Alternatives Considered**:
- Dynamic arrays (std::vector): More flexible but requires C++
- Linked list: O(n) access time
- Rope data structure: Complex for this use case

### 2. Search Results Array

**Choice**: Dynamically allocated array

**Rationale**:
- Size unknown until search completes
- Temporary data structure (freed after use)
- Two-pass approach: count first, allocate, then collect

```c
SearchResult* results = malloc(sizeof(SearchResult) * count);
```

### 3. Plugin Command Linked List

**Choice**: Singly-linked list

**Rationale**:
- Unknown number of plugins
- Infrequent access (user-driven)
- Simple insertion
- No need for random access

```c
typedef struct PluginCommand {
    // ... data ...
    struct PluginCommand* next;
} PluginCommand;
```

### 4. Character Statistics Structure

**Choice**: Simple struct with counters

```c
typedef struct {
    int letter_count;
    int digit_count;
    int space_count;
    int total_count;
    int punctuation_count;
    int other_count;
    int chinese_count;
} CharStatistics;
```

**Rationale**:
- Plain old data (POD) for simplicity
- Return by value (small structure)
- Clear, self-documenting fields

## Algorithm Choices

### 1. KMP String Matching

**Purpose**: Efficient substring search

**Complexity**: O(n + m) where n = text length, m = pattern length

**Why KMP**:
- Linear time complexity
- No backtracking in the text
- Preprocesses pattern only once
- Suitable for multiple searches with the same pattern

**Implementation Highlights**:
```c
static void build_lps(const char *pattern, size_t m, int *lps);
static int kmp_count_line(const char *text, const char *pattern, const int *lps, size_t m);
```

**Alternative Considered**:
- Boyer-Moore: Better average case but more complex
- Simple brute force: O(n*m), too slow for large texts

### 2. UTF-8 Character Handling

**Purpose**: Correctly handle multi-byte characters

**Approach**:
- Detect byte sequence length from first byte
- Calculate character count vs. byte offset
- Maintain character-based indexing for user operations

**Key Functions**:
```c
static int utf8_char_length(unsigned char c);
int utf8_strlen_chars(const char *s);
static int utf8_byte_offset(const char *s, int char_index);
```

**Challenges Addressed**:
- Chinese characters (3 bytes in UTF-8)
- Emoji and symbols (4 bytes)
- Column/line positioning
- Insert/delete/replace at character boundaries

### 3. Character Classification

**Purpose**: Accurately count different character types

**Approach**:
- Decode UTF-8 to Unicode code points
- Apply range checks for CJK, fullwidth, etc.
- Handle both ASCII and multibyte characters

**Classification Ranges**:
```c
- CJK Unified Ideographs: 0x4E00-0x9FFF
- CJK Extensions: 0x3400-0x4DBF, 0x20000-0x2CEAF
- Fullwidth digits: 0xFF10-0xFF19
- Fullwidth letters: 0xFF21-0xFF3A, 0xFF41-0xFF5A
- CJK punctuation: 0x3000-0x303F, 0xFE30-0xFE4F
```

## Memory Management

### Static vs. Dynamic Allocation

**Static (Fixed-size) Allocations**:
- Text buffer (`TextBuffer`)
- Plugin module handles array
- Plugin directory path

**Dynamic Allocations**:
- Search results (freed after use)
- KMP preprocessing array (freed after use)
- Temporary buffers in plugins

### Safety Measures

1. **Bounds Checking**:
   - All array accesses validated
   - Line and column indices checked
   - Buffer sizes verified

2. **Safe CRT Functions**:
   ```c
   strcpy_s()     // Instead of strcpy
   strncpy_s()    // Instead of strncpy
   strcat_s()     // Instead of strcat
   sprintf_s()    // Instead of sprintf
   fopen_s()      // Instead of fopen
   ```

3. **Truncation Handling**:
   ```c
   strncpy_s(dest, sizeof(dest), src, _TRUNCATE);
   ```

4. **Null Termination**:
   - All strings explicitly null-terminated
   - Size calculations include null terminator

### Memory Ownership

- **Text Buffer**: Owned by main program
- **Plugin API**: Plugins receive read-only pointers; modifications through API
- **Search Results**: Caller responsible for freeing
- **Temporary Buffers**: Function-local, stack-allocated when possible

## Plugin Architecture

### Loading Mechanism

1. **Discovery**: Scan `./plugins` directory for `*.dll` files
2. **Loading**: `LoadLibrary()` each DLL
3. **Symbol Resolution**: `GetProcAddress(hModule, "PluginInit")`
4. **Initialization**: Call `PluginInit(EditorAPI*)` with API pointer
5. **Command Registration**: Plugin calls `register_command()` multiple times
6. **Tracking**: Store module handles for cleanup

### API Design Principles

1. **Minimalism**: Only essential functions exposed
2. **Stability**: Avoid frequent API changes
3. **Versioning**: Future version could add version field to EditorAPI
4. **Safety**: Plugins can't directly access global state
5. **Convenience**: Higher-level operations (not just byte manipulation)

### Command Execution Flow

```
User selects "Execute plugin command"
  └─→ Display list of registered commands
      └─→ User enters command name
          └─→ Lookup command in registry (linked list)
              └─→ If found: call command->func()
                  └─→ Command function executes
                      └─→ May call EditorAPI functions
                          └─→ May prompt user for input
                              └─→ May modify buffer
                                  └─→ Return to plugin menu
```

### Security Model

**Trust-based**:
- Plugins run with same privileges as main program
- No sandboxing or permission system
- Plugins can read/write files, make network calls
- Users responsible for plugin provenance

**Mitigation**:
- Load only from designated directory
- User explicitly triggers plugin loading
- Could add digital signature verification (future)

## Error Handling Strategy

### Return Code Convention

```c
// Success/failure
0  = Success
-1 = Failure

// Counts/sizes
>= 0 = Success (count or size)
-1   = Failure
```

### Error Propagation

```c
int result = operation();
if (result != 0) {
    print_error_message();
    return -1;  // Propagate failure
}
```

### User Feedback

- Error messages displayed immediately
- Context-specific messages (not generic "Error")
- Suggestions for resolution when possible

### Defensive Programming

- **Null checks**: All pointer parameters validated
- **Range checks**: Array indices and sizes verified
- **State validation**: Check buffer state before operations
- **Graceful degradation**: Continue operation when possible

### No Exception Handling

- C doesn't have exceptions
- All errors communicated via return codes
- Plugins should follow same convention

## Security Considerations

### Input Validation

1. **File Paths**:
   - Max length checked (MAX_FILENAME)
   - No sanitization (Windows file system handles most cases)
   - Future: Could add path traversal checks

2. **User Text Input**:
   - Length limits enforced (MAX_LINE_LENGTH)
   - No script injection concerns (not web-based)
   - UTF-8 validation implicit in processing

3. **Network Input**:
   - HTTP responses bounded by buffer size
   - No execution of downloaded content
   - Future: HTTPS certificate validation

### Buffer Overflow Protection

1. **Fixed-size buffers**: Compile-time bounds
2. **Safe CRT functions**: Runtime bounds checking
3. **Truncation**: Graceful handling of oversize content
4. **Array indexing**: Validated before access

### Plugin Safety

**Current State**:
- No privilege separation
- Plugins can do anything the main program can

**Future Enhancements**:
- Permission system (file access, network access)
- API subset selection
- Plugin signing/verification
- Sandboxing (e.g., using Windows AppContainer)

### Safe CRT Benefits

- Runtime buffer overrun detection
- Automatic null termination
- Debug assertions in debug builds
- Minimize undefined behavior

### Potential Vulnerabilities

1. **Path Traversal**: File operations accept any path
   - Mitigation: User must explicitly initiate file operations

2. **Plugin Malice**: Plugins are fully trusted
   - Mitigation: Load only from local directory, verify source

3. **Network Attacks**: HTTP client doesn't validate certificates
   - Mitigation: Use for trusted endpoints only

4. **Resource Exhaustion**: No limits on plugin memory usage
   - Mitigation: Process-level OS limits

## Future Architecture Enhancements

### Planned Improvements

1. **Undo/Redo System**:
   - Command pattern for operations
   - Operation history stack
   - Memento pattern for state snapshots

2. **Configuration System**:
   - INI file or JSON configuration
   - Plugin-specific settings
   - User preferences

3. **Syntax Highlighting**:
   - Lexer/tokenizer module
   - Language definition files
   - Color scheme support

4. **Multi-file Support**:
   - Buffer manager for multiple documents
   - Tab or window switching
   - Cross-file search

5. **Asynchronous Operations**:
   - Background file loading
   - Non-blocking network requests
   - Progress indicators

### Portability Roadmap

**Platform Abstraction**:
- Plugin loading: Abstract DLL/SO loading
- File I/O: POSIX or Windows APIs
- Console: termios (Unix) vs. Windows Console API
- HTTP: libcurl (cross-platform) vs. WinHTTP

**Build System**:
- CMake for cross-platform builds
- Conditional compilation for platform-specific code

---

**Document Version**: 1.0
**Last Updated**: 2026-01-11
**Status**: Current architecture documentation
