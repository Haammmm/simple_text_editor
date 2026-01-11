# Changelog

All notable changes to SimpleTextEditor will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Planned Features
- Undo/redo functionality
- Configuration file support
- Syntax highlighting for common file types
- Multi-file editing (tabs or windows)
- Cross-platform support (Linux, macOS)
- Plugin API versioning
- Asynchronous HTTP requests
- Search with regular expressions
- Line number display toggle
- Status bar with cursor position

## [1.0.0] - 2026-01-11

### Added

#### Core Features
- **Text Buffer Management**
  - Support for up to 1000 lines with 4096 characters per line
  - Line-based text storage and manipulation
  - Modified state tracking
  - Filename association

#### Text Editing Operations
- **Line Operations**
  - Insert new lines at any position
  - Delete lines by index
  - Replace entire line content
  
- **Substring Operations**
  - Insert substring at specific character position
  - Replace substring at position with new text
  - Replace single character at position
  - Replace all occurrences of a pattern
  - Delete all occurrences of a pattern

- **Search Functionality**
  - KMP (Knuth-Morris-Pratt) algorithm for efficient substring search
  - Find all occurrences with line and column positions
  - Count total occurrences of a substring
  - UTF-8 aware search

#### Character Statistics
- Count English letters (ASCII and fullwidth)
- Count digits (ASCII and fullwidth)
- Count spaces (ASCII and ideographic)
- Count punctuation marks (ASCII and CJK)
- Count CJK unified ideographs (Chinese characters)
- Count other characters
- Total character count

#### UTF-8 Support
- Complete UTF-8 multi-byte character handling
- Character-based (not byte-based) positioning
- Support for Chinese, Japanese, Korean characters
- Emoji and special symbols support
- Proper character length calculation
- Byte-to-character offset conversion

#### File Operations
- Open text files (UTF-8 encoded)
- Save text to files
- Save to current file (quick save)
- Automatic line truncation on load (safety feature)
- Modified flag tracking
- Safe CRT file operations

#### Plugin System
- **Dynamic Plugin Loading**
  - Scan and load DLL plugins from `./plugins` directory
  - Plugin initialization via `PluginInit` export
  - Command registration system
  - Linked list command registry

- **EditorAPI Interface**
  - Buffer query functions (line count, line content)
  - Buffer modification functions (insert, delete, replace lines)
  - File I/O helpers (read_file, write_file)
  - Network helpers (http_get via WinHTTP)
  - UI helpers (print_msg, clear_screen)

- **LLM Plugin Example (Dll1)**
  - Chat with LLM using OpenAI-compatible API
  - Tool calling support for buffer manipulation
  - `apply_editor` tool to overwrite buffer content
  - `get_buffer` tool to read current buffer
  - Integration with ModelScope API
  - Uses `openai-cpp` library

#### User Interface
- Console-based menu system
- Clear visual separators and formatting
- Input validation and error handling
- Confirmation dialogs for destructive operations
- Bilingual interface (Chinese and English)
- Progress indicators and status messages

#### Security Features
- Safe CRT functions (`*_s` variants) throughout
- Bounds checking on all array accesses
- Buffer overflow protection
- Automatic string truncation with `_TRUNCATE`
- Input validation for all user inputs

#### Documentation
- Comprehensive README in English
- Chinese documentation (中文文档.md)
- API documentation for plugin developers
- Architecture and design documentation
- Contributing guidelines
- Examples and usage guide
- Troubleshooting guide
- This changelog

### Technical Details

#### Algorithms
- **KMP String Matching**: O(n+m) time complexity for substring search
- **UTF-8 Decoding**: Proper handling of 1-4 byte sequences
- **Character Classification**: Unicode code point range checking

#### Data Structures
- Fixed-size 2D array for text buffer (predictable memory)
- Linked list for plugin command registry
- Dynamic arrays for search results (allocated on demand)

#### Dependencies
- Windows API (WinHTTP for HTTP, LoadLibrary for plugins)
- Windows SDK (winhttp.lib)
- C17 standard library
- C++20 for plugins (optional)
- openai-cpp for LLM plugin (optional)
- nlohmann/json for JSON parsing in plugins (optional)

#### Platform
- Windows 7 or later
- Visual Studio 2019 or later
- x86 or x64 architecture

### Known Limitations
- Windows-only (uses WinHTTP and Windows DLL loading)
- Maximum 1000 lines
- Maximum 4096 characters per line
- No undo/redo
- No syntax highlighting
- No multi-file support
- Synchronous network operations (blocking)
- No plugin sandboxing (plugins have full access)

### Security Notes
- Uses safe CRT functions to prevent buffer overflows
- No input sanitization for SQL or XSS (not applicable to text editor)
- Plugins are fully trusted and not sandboxed
- File operations accept any path (user must ensure safety)
- HTTP requests do not validate certificates (use trusted endpoints only)

## Version History Summary

### [1.0.0] - Initial Release
First public release of SimpleTextEditor with core editing features, UTF-8 support, plugin system, and LLM integration example.

---

## Future Roadmap

### Version 1.1 (Planned)
- Undo/redo system
- Configuration file support
- Plugin API improvements
- Better error messages
- Performance optimizations

### Version 1.2 (Planned)
- Syntax highlighting
- Line number display
- Status bar
- Better cursor positioning
- Multi-file editing

### Version 2.0 (Future)
- Cross-platform support (Linux, macOS)
- GUI version
- Regular expression search
- Scripting support
- Network protocol improvements

---

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## Versioning

We use [SemVer](http://semver.org/) for versioning:
- **MAJOR**: Incompatible API changes
- **MINOR**: Backwards-compatible functionality additions
- **PATCH**: Backwards-compatible bug fixes

## License

This project is licensed under the MIT License - see [LICENSE.txt](LICENSE.txt) for details.

---

**Note**: Dates in this changelog use ISO 8601 format (YYYY-MM-DD).

[Unreleased]: https://github.com/Haammmm/simple_text_editor/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Haammmm/simple_text_editor/releases/tag/v1.0.0
