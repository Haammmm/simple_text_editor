# SimpleTextEditor

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/language-C%2FC%2B%2B-orange.svg)](https://isocpp.org/)

A lightweight console text editor inspired by Vim, written in C/C++. It supports UTF-8 text operations, basic editing commands, and a Windows DLL plugin system. A sample LLM plugin demonstrates integration via [`openai-cpp`](https://github.com/olrea/openai-cpp).

## 📚 Documentation

- **[中文文档](中文文档.md)** - Complete Chinese documentation
- **[API Documentation](API.md)** - Plugin development guide
- **[Architecture](ARCHITECTURE.md)** - Design and architecture details
- **[Examples](EXAMPLES.md)** - Usage examples and workflows
- **[Troubleshooting](TROUBLESHOOTING.md)** - Common issues and solutions
- **[Contributing](CONTRIBUTING.md)** - How to contribute
- **[Code of Conduct](CODE_OF_CONDUCT.md)** - Community guidelines
- **[Changelog](CHANGELOG.md)** - Version history and changes

## ✨ Features

### Core Editing
- **Line-based Operations**: Insert, delete, and replace entire lines
- **Substring Operations**: Insert, replace, and delete substrings at any position
- **Character Operations**: Replace individual characters with UTF-8 awareness
- **Search**: KMP algorithm-based substring search with UTF-8 support
- **Character Statistics**: Count letters, digits, spaces, punctuation, and CJK characters

### UTF-8 Support
- Full multi-byte character handling (Chinese, Japanese, Korean, emoji)
- Character-based positioning (not byte-based)
- Proper length calculation for mixed ASCII and multi-byte text
- CJK unified ideographs recognition

### File Operations
- Open and save text files (UTF-8 encoding)
- Automatic line truncation for safety (4096 character limit)
- Modified state tracking
- Safe CRT APIs throughout

### Plugin System
- Dynamic DLL loading from `plugins` directory
- Custom command registration via `EditorAPI`
- Buffer manipulation (read/write lines)
- File I/O and HTTP request helpers
- Sample LLM plugin demonstrating tool-calling capabilities

### User Interface
- Console-based menu system
- Clear visual formatting and separators
- Input validation and confirmation dialogs
- Bilingual interface (Chinese and English)
- Comprehensive error messages

## Requirements
- Windows (WinHTTP for HTTP requests; used by plugin manager and LLM plugin).
- Visual Studio with C17/C++20 support.
- Windows SDK (provides `winhttp.lib`).
- Optional: `openai-cpp` headers for building the LLM plugin (`Dll1` project).

## Build
1. Open the solution in Visual Studio.
2. Ensure `winhttp.lib` is linked in project settings (Linker > Input > Additional Dependencies).
3. Build `Project1` (console app) and `Dll1` (LLM plugin DLL).
4. Copy built DLL plugins to `./plugins` (alongside the executable) to load them at runtime.

## Run
- Launch the console app (`Project1`).
- Use the menu (0-10) for editing, file I/O, statistics, and plugin management.
- In “插件管理”, choose to scan/load plugins, list commands, and execute plugin commands.

## LLM Plugin (`LLMDialog`)
- Located in `Dll1/openai_agent.cpp`.
- Uses [`openai-cpp`](https://github.com/olrea/openai-cpp) for API calls.
- Requires environment variable `test_apikey` to be set with your API key.
- Registers command `LLMDialog`; when executed, it:
  - Sends current buffer (read-only) plus your prompt to the LLM.
  - Can update the buffer via the `apply_editor` tool call.

## 🛠️ Plugin Development

### Quick Start

1. **Create a DLL project** in Visual Studio
2. **Include** `plugin.h` from the main project
3. **Implement and export** `PluginInit(EditorAPI* api)`
4. **Register commands** using `api->register_command()`
5. **Compile to DLL** and place in `./plugins` directory

### EditorAPI Functions

The `EditorAPI` provides:
- **Buffer Access**: `get_line_count()`, `get_line()`
- **Buffer Modification**: `insert_line()`, `delete_line()`, `replace_line()`
- **File Operations**: `read_file()`, `write_file()`
- **Network**: `http_get()` (synchronous HTTP GET)
- **UI**: `print_msg()`, `clear_screen()`
- **Command Registration**: `register_command()`

### Example Plugin

See [API.md](API.md) for comprehensive examples and detailed documentation.

## ⚙️ Configuration

### Plugin Directory
- **Default Path**: `./plugins` (next to the executable)
- Place DLL files here for automatic loading

### Environment Variables
- **`test_apikey`**: API key for the LLM plugin
  - Required for LLM plugin functionality
  - Set via: `set test_apikey=your_api_key_here`
  - Default endpoint: ModelScope API
  - To use other providers: modify `kBaseUrl`/`kModel` in `Dll1/openai_agent.cpp`

### Limits and Constraints
- **MAX_LINES**: 1000 (maximum number of lines)
- **MAX_LINE_LENGTH**: 4096 (maximum characters per line)
- **MAX_FILENAME**: 256 (maximum filename length)
- **Plugin Limit**: 32 DLLs can be loaded simultaneously

## 📖 Quick Start

### Basic Usage

1. **Launch the editor** and you'll see the main menu
2. **Select option 1** to input text line by line (empty line to finish)
3. **Select option 4** to see character statistics
4. **Select option 3** to save your text to a file
5. **Select option 0** to exit

### Example: Find and Replace

1. Open or create a text file
2. Select option 5 to find a substring (e.g., "test")
3. Select option 7 → 3 to replace all occurrences (e.g., replace "test" with "production")
4. Save the modified file

For more examples, see [EXAMPLES.md](EXAMPLES.md).

## 🔌 Plugin Development

Create your own plugins to extend the editor:

```c
#include "plugin.h"

static EditorAPI* g_api = NULL;

void my_command(void) {
    if (!g_api) return;
    int count = g_api->get_line_count();
    char msg[64];
    sprintf_s(msg, sizeof(msg), "Total lines: %d\n", count);
    g_api->print_msg(msg);
}

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("MyCommand", my_command, "Show line count");
    return 0;
}
}
```

Compile to DLL, place in `./plugins`, and load in the editor!

For comprehensive plugin development guide, see [API.md](API.md).

## 🏗️ Architecture

The editor follows a layered architecture:
- **Presentation Layer**: Console UI and menu system
- **Business Logic**: Text editing operations and algorithms
- **Data Layer**: Text buffer management
- **Plugin Layer**: Dynamic extensibility

Key design decisions:
- Fixed-size buffers for predictable memory usage
- KMP algorithm for efficient O(n+m) substring search
- Safe CRT functions for buffer overflow protection
- UTF-8 code point handling for proper character operations

For detailed architecture documentation, see [ARCHITECTURE.md](ARCHITECTURE.md).

## 🔒 Security

- **Buffer Overflow Protection**: Uses safe CRT functions (`*_s` variants)
- **Input Validation**: All user inputs are validated
- **Bounds Checking**: Array accesses are range-checked
- **String Safety**: Automatic truncation with `_TRUNCATE` flag

**Plugin Security Note**: Plugins are fully trusted and run with the same privileges as the main program. Only load plugins from trusted sources.

## 🐛 Troubleshooting

### Common Issues

**Q: Chinese characters display as boxes**
- A: Set console to UTF-8: `chcp 65001`

**Q: Plugin doesn't load**
- A: Ensure the DLL exports `PluginInit` and matches the executable architecture (x86/x64)

**Q: LLM plugin returns error**
- A: Set environment variable: `set test_apikey=your_api_key`

For more solutions, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).

## 🤝 Contributing

We welcome contributions! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style guidelines
- Pull request process
- Development workflow
- Testing requirements

## 📋 Project Structure

```
simple_text_editor/
├── Project1/SimpleTextEditor/  # Main console application
│   ├── main.c                   # UI and entry point
│   ├── text_editor.c/h         # Core editing logic
│   ├── plugin_manager.c/h      # Plugin system
│   └── plugin.h                 # Plugin interface
├── Dll1/                        # LLM plugin example
│   ├── openai_agent.cpp        # Plugin implementation
│   └── openai/                  # openai-cpp library
├── plugins/                     # Plugin directory (runtime)
└── Documentation files
    ├── README.md                # This file
    ├── 中文文档.md              # Chinese documentation
    ├── API.md                   # Plugin API reference
    ├── ARCHITECTURE.md          # Design documentation
    ├── EXAMPLES.md              # Usage examples
    ├── TROUBLESHOOTING.md       # Problem solutions
    ├── CONTRIBUTING.md          # Contribution guide
    └── CHANGELOG.md             # Version history
```

## 📝 Notes
- Uses safe CRT functions (e.g., `*_s` variants) to avoid `_CRT_SECURE_NO_WARNINGS`.
- WinHTTP-based HTTP client is synchronous and Windows-only.
- LLM calls default to ModelScope; adjust source to target other platforms/models as needed.
- Maximum capacity: 1000 lines × 4096 characters per line.
- UTF-8 encoding is required for proper text handling.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE.txt](LICENSE.txt) file for details.

## 🙏 Acknowledgments

- **[openai-cpp](https://github.com/olrea/openai-cpp)** - OpenAI API C++ client library
- **[nlohmann/json](https://github.com/nlohmann/json)** - JSON for Modern C++
- **KMP Algorithm** - Donald Knuth, Vaughan Pratt, and James H. Morris
- **UTF-8 Specification** - Rob Pike and Ken Thompson

## 🔗 Related Resources

- [UTF-8 Everywhere](http://utf8everywhere.org/) - Best practices for UTF-8
- [C Secure Coding Standard](https://wiki.sei.cmu.edu/confluence/display/c/SEI+CERT+C+Coding+Standard) - Security guidelines
- [Plugin Architecture Patterns](https://en.wikipedia.org/wiki/Plugin_(computing)) - Design patterns

## 📮 Contact & Support

- **Issues**: [GitHub Issues](https://github.com/Haammmm/simple_text_editor/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Haammmm/simple_text_editor/discussions)
- **Documentation**: See the [Documentation](#-documentation) section above

## 🌟 Show Your Support

If you find this project useful, please consider:
- ⭐ Starring the repository
- 🐛 Reporting bugs
- 💡 Suggesting features
- 🤝 Contributing code
- 📖 Improving documentation

## 🚀 Version

**Current Version**: 1.0.0

See [CHANGELOG.md](CHANGELOG.md) for version history and upcoming features.

---

**Made with ❤️ for text editing enthusiasts**


