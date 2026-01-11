# SimpleTextEditor

A lightweight console text editor inspired by Vim, written in C/C++. It supports UTF-8 text operations, basic editing commands, and a Windows DLL plugin system. A sample LLM plugin demonstrates integration via [`openai-cpp`](https://github.com/olrea/openai-cpp).

## Features
- Line-based editing: insert/delete lines, substring insert/replace/delete, character replace.
- UTF-8 aware substring search (KMP), character statistics (letters/digits/spaces/punctuation/CJK).
- File open/save (text), truncate on overlength lines, safe CRT APIs.
- Plugin system: load DLLs from `plugins` directory, register custom commands via `EditorAPI`.
- Sample LLM plugin (`LLMDialog`) that can read/write the editor buffer through tool calls.

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

## Plugin Development
- Implement `PluginInit(EditorAPI* api)` in a DLL and export it.
- Register commands with `api->register_command(name, func, desc)`.
- `EditorAPI` exposes line access, insert/delete/replace, printing log messages, and simple file/http helpers.
- Place your DLLs in `plugins` (default path resolved next to the executable).

## Configuration
- Plugin directory defaults to `./plugins` next to the executable.
- Environment variables:
  - `test_apikey`: API key for the LLM plugin (default endpoints point to ModelScope; change `kBaseUrl`/`kModel` in `Dll1/openai_agent.cpp` if you switch providers).

## Notes
- Uses safe CRT functions (e.g., `*_s` variants) to avoid `_CRT_SECURE_NO_WARNINGS`.
- WinHTTP-based HTTP client is synchronous and Windows-only.
- LLM calls default to ModelScope; adjust source to target other platforms/models as needed.

