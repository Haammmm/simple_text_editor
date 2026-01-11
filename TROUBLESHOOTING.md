# Troubleshooting Guide

## Table of Contents
- [Compilation Issues](#compilation-issues)
- [Runtime Errors](#runtime-errors)
- [Plugin Problems](#plugin-problems)
- [File Operation Issues](#file-operation-issues)
- [Text Display Problems](#text-display-problems)
- [LLM Plugin Issues](#llm-plugin-issues)
- [Performance Issues](#performance-issues)
- [Platform-Specific Issues](#platform-specific-issues)

## Compilation Issues

### Issue: Cannot find `winhttp.lib`

**Symptoms**:
```
error LNK1104: cannot open file 'winhttp.lib'
```

**Cause**: Windows SDK is not installed or linker cannot find the library.

**Solution**:
1. Verify Windows SDK is installed:
   - Open Visual Studio Installer
   - Modify your installation
   - Ensure "Windows SDK" is selected
2. Check linker settings:
   - Project Properties → Linker → Input → Additional Dependencies
   - Ensure `winhttp.lib` is listed
3. Rebuild the project

**Alternative**: If SDK is installed, manually add the library path:
- Project Properties → Linker → General → Additional Library Directories
- Add the path to your Windows SDK lib folder

---

### Issue: C4996 warnings about unsafe functions

**Symptoms**:
```
warning C4996: 'strcpy': This function or variable may be unsafe
```

**Cause**: Using non-secure CRT functions.

**Solution**: The code should already use safe functions (`strcpy_s`, etc.). If you see this warning:
1. Check that you're using `*_s` variants of functions
2. Ensure `_CRT_SECURE_NO_WARNINGS` is NOT defined
3. Replace any unsafe function calls with secure versions

**Code Fix**:
```c
// ✗ Unsafe
strcpy(dest, src);

// ✓ Safe
strcpy_s(dest, sizeof(dest), src);
```

---

### Issue: C++ standard errors

**Symptoms**:
```
error C2760: syntax error: unexpected token 'identifier', expected ';'
```

**Cause**: C++20 features used but not enabled.

**Solution**:
1. Open Project Properties
2. C/C++ → Language → C++ Language Standard
3. Set to "ISO C++20 Standard (/std:c++20)"
4. Rebuild

---

### Issue: Mixed C/C++ compilation errors

**Symptoms**:
```
error C2440: 'initializing': cannot convert from 'void *' to 'SomeType *'
```

**Cause**: C code compiled as C++ (or vice versa).

**Solution**:
1. Ensure `.c` files are compiled as C
2. Ensure `.cpp` files are compiled as C++
3. Check Project Properties → C/C++ → Advanced → Compile As
   - C files: "Compile as C Code (/TC)"
   - C++ files: "Compile as C++ Code (/TP)"

---

## Runtime Errors

### Issue: Application crashes on startup

**Symptoms**: Program exits immediately or shows access violation.

**Possible Causes and Solutions**:

**1. Plugin loading failure**:
- Remove all DLLs from `./plugins` directory
- Restart the application
- Add plugins back one at a time

**2. Missing dependencies**:
- Ensure Visual C++ Redistributable is installed
- Check that all DLL dependencies are available

**3. Corrupted binary**:
- Clean and rebuild the solution
- Delete `Debug` or `Release` folders and rebuild

**Debugging Steps**:
1. Run in Visual Studio debugger (F5)
2. Check the Output window for error messages
3. Look at the call stack when crash occurs
4. Check Event Viewer for application errors

---

### Issue: Access violation when accessing buffer

**Symptoms**:
```
Exception thrown: read access violation
buf->lines was 0x[address]
```

**Cause**: Uninitialized or NULL buffer pointer.

**Solution**:
- Ensure `buffer_init(&g_buffer)` is called before any buffer operations
- Check that buffer pointer is not NULL before use
- Verify line indices are within valid range (0 to line_count-1)

**Prevention**:
```c
// Always validate
if (buf == NULL) return -1;
if (line < 0 || line >= buf->line_count) return -1;
```

---

### Issue: Buffer modifications not taking effect

**Symptoms**: Changes appear to work but buffer content doesn't change.

**Possible Causes**:
1. **Operating on wrong buffer**: Ensure you're modifying the global buffer
2. **Function returning error**: Check return values
3. **Line length exceeded**: Text too long and silently truncated

**Solution**:
```c
int result = insert_substring(&g_buffer, line, col, text);
if (result != 0) {
    printf("Operation failed: check parameters and buffer state\n");
}
```

---

## Plugin Problems

### Issue: Plugin not loading

**Symptoms**: Plugin DLL is in `./plugins` but doesn't appear in command list.

**Possible Causes and Solutions**:

**1. Missing export**:
- Verify the DLL exports `PluginInit` function
- Use Dependency Walker or `dumpbin` to check exports:
  ```cmd
  dumpbin /EXPORTS your_plugin.dll
  ```

**2. Wrong architecture**:
- Ensure DLL matches executable (both x86 or both x64)
- Check project platform configuration

**3. PluginInit returns error**:
- Debug the plugin's `PluginInit` function
- Check return value (must be 0 for success)

**4. Dependencies missing**:
- Plugin DLL may depend on other DLLs
- Use Dependency Walker to check missing dependencies

**5. C++ name mangling**:
- Ensure `extern "C"` is used:
  ```cpp
  extern "C" {
  __declspec(dllexport) int PluginInit(EditorAPI* api);
  }
  ```

---

### Issue: Plugin crashes when executed

**Symptoms**: Application crashes when plugin command is executed.

**Debugging Steps**:
1. **Check NULL pointers**:
   ```cpp
   if (!g_api) return;  // Always check API pointer
   ```

2. **Validate buffer access**:
   ```cpp
   int count = g_api->get_line_count();
   if (line < 0 || line >= count) return;  // Validate indices
   ```

3. **Memory allocation**:
   - Ensure proper allocation and deallocation
   - Don't free memory returned by `get_line()`

4. **String safety**:
   - Use safe string functions
   - Ensure buffers are large enough
   - Always null-terminate strings

---

### Issue: Plugin commands not appearing

**Symptoms**: Plugin loads but commands don't show in list.

**Cause**: Command not registered or registration failed.

**Solution**:
1. Check `register_command()` is called in `PluginInit`:
   ```cpp
   api->register_command("CommandName", my_function, "Description");
   ```

2. Verify command name length (max 31 characters)

3. Verify description length (max 63 characters)

4. Check that command name is unique

5. Ensure function pointer is valid

---

### Issue: Cannot reload plugin after changes

**Symptoms**: Modified plugin DLL doesn't reflect changes.

**Cause**: Windows locks loaded DLLs.

**Solution**:
1. Exit the SimpleTextEditor application completely
2. Replace the plugin DLL file
3. Restart SimpleTextEditor
4. Reload plugins (Option 10 → 1)

**Note**: You cannot replace a DLL while it's loaded.

---

## File Operation Issues

### Issue: Cannot open file

**Symptoms**:
```
Error: Unable to open file 'filename.txt'
```

**Possible Causes**:

**1. File doesn't exist**:
- Check file path and name (case-sensitive on some systems)
- Use absolute path if relative path doesn't work

**2. File permissions**:
- Verify you have read permissions
- Check if file is locked by another program
- Run editor with appropriate permissions

**3. File path too long**:
- Maximum path length is MAX_FILENAME (256 characters)
- Use shorter path or filename

**4. Invalid characters in filename**:
- Avoid special characters: `< > : " / \ | ? *`
- Use standard alphanumeric characters

---

### Issue: Cannot save file

**Symptoms**:
```
Error: Unable to create file 'output.txt'
```

**Possible Causes**:

**1. No write permissions**:
- Check folder permissions
- Try saving to a different location (e.g., Documents folder)

**2. Disk full**:
- Check available disk space
- Clean up temporary files

**3. File locked**:
- Close file in other programs
- Check for read-only flag

**4. Invalid path**:
- Ensure parent directory exists
- Use valid path separators (`\` on Windows)

---

### Issue: File content corrupted or garbled

**Symptoms**: File displays with strange characters or boxes.

**Cause**: Encoding mismatch.

**Solution**:
1. **Ensure UTF-8 encoding**:
   - The editor expects UTF-8 encoded files
   - Convert files to UTF-8 before opening

2. **Check console code page**:
   ```cmd
   chcp 65001
   ```
   This sets the console to UTF-8 mode.

3. **Use UTF-8 capable text editor**:
   - Save files as UTF-8 (not ANSI or UTF-16)
   - Notepad, VS Code, or Notepad++ can save as UTF-8

---

### Issue: Lines truncated when opening file

**Symptoms**: Long lines are cut off.

**Cause**: Line exceeds MAX_LINE_LENGTH (4096 characters).

**Behavior**: This is expected - lines are automatically truncated for safety.

**Solution**:
- Split long lines in the source file
- Or modify MAX_LINE_LENGTH in `text_editor.h` and recompile (may impact memory usage)

---

## Text Display Problems

### Issue: Chinese characters display as boxes or question marks

**Symptoms**: UTF-8 characters don't display correctly.

**Solution**:
1. **Set console code page to UTF-8**:
   ```cmd
   chcp 65001
   ```

2. **Use a font that supports Chinese**:
   - Console Properties → Font
   - Select "SimSun", "Microsoft YaHei", or "Consolas"

3. **For persistent fix**, create a shortcut with:
   - Target: `cmd.exe /K chcp 65001`
   - Font: Unicode-capable font

---

### Issue: Display formatting is misaligned

**Symptoms**: Columns don't line up properly with Chinese text.

**Cause**: Chinese characters are wider than ASCII in console display.

**Note**: This is a console limitation. The editor correctly handles character positions internally.

**Workaround**: Use a monospace font and accept that some alignment may be imperfect.

---

### Issue: Menu or borders appear broken

**Symptoms**: Box-drawing characters appear as garbage.

**Cause**: Console code page doesn't support the characters used.

**Solution**: Set code page to UTF-8 (see above) or use a different console.

---

## LLM Plugin Issues

### Issue: "Missing environment variable test_apikey"

**Symptoms**:
```
Missing environment variable test_apikey, cannot call LLM interface
```

**Cause**: API key not set.

**Solution**:
1. **Set the environment variable**:
   ```cmd
   set test_apikey=your_actual_api_key_here
   ```

2. **For persistent setting**:
   - Windows Search → "Environment Variables"
   - System Properties → Environment Variables
   - Add new user variable: `test_apikey` = `your_key`

3. **Restart the editor** after setting the variable

---

### Issue: HTTP request fails

**Symptoms**:
```
OpenAI Error: [http error details]
```

**Possible Causes**:

**1. Network connectivity**:
- Check internet connection
- Test with browser: https://api-inference.modelscope.cn/

**2. Invalid API key**:
- Verify the key is correct
- Check for extra spaces or quotes

**3. API endpoint down**:
- Try again later
- Check service status

**4. Firewall/proxy**:
- Check firewall settings
- Corporate networks may block external APIs
- Configure proxy if needed (not currently supported - code modification required)

---

### Issue: LLM doesn't update buffer

**Symptoms**: LLM responds but buffer content doesn't change.

**Possible Reasons**:

**1. LLM decided not to modify**:
- LLM may just provide information
- Rephrase instruction to be more explicit
- Example: "Update the buffer to..." instead of "Can you..."

**2. Tool call failed**:
- Check console output for error messages
- Verify buffer is not at maximum capacity

**3. API response incomplete**:
- Network timeout
- Try again with simpler request

---

### Issue: LLM response is slow

**Symptoms**: Long wait time (30+ seconds).

**Causes**:
- API server load
- Complex request
- Network latency

**Solutions**:
- Use simpler instructions
- Reduce buffer size (fewer lines for LLM to process)
- Try at different times of day
- Consider local model deployment (requires code changes)

---

## Performance Issues

### Issue: Search is slow with large buffer

**Symptoms**: Finding substrings takes several seconds.

**Explanation**: 
- KMP algorithm is O(n+m), quite efficient
- With 1000 lines of 4096 characters each, some delay is expected

**Solutions**:
- Reduce buffer size (fewer lines)
- Search in specific sections of the file
- Consider optimizing the search (compile with optimizations enabled)

---

### Issue: Application becomes unresponsive

**Symptoms**: Editor freezes or hangs.

**Possible Causes**:

**1. Infinite loop in plugin**:
- Debug the plugin
- Check for while loops without proper exit conditions

**2. Network operation hanging**:
- HTTP requests are synchronous and blocking
- Network timeout may be long
- Wait or terminate process

**3. Large buffer operations**:
- Replacing many occurrences may take time
- Wait for operation to complete

---

## Platform-Specific Issues

### Issue: Editor doesn't work on Windows 7

**Symptoms**: Application fails to start or crashes.

**Possible Causes**:
- Missing Visual C++ Redistributable
- Windows SDK version incompatibility

**Solution**:
1. Install Visual C++ Redistributable (x86 or x64, matching your build)
2. Rebuild with Windows 7 SDK
3. Consider upgrading to Windows 10/11 for better support

---

### Issue: Cannot run on Linux or macOS

**Symptoms**: N/A - doesn't compile.

**Explanation**: The application uses Windows-specific APIs:
- WinHTTP for HTTP requests
- LoadLibrary/GetProcAddress for plugin loading
- Windows.h headers

**Solution**: Port to platform-independent APIs:
- Use libcurl for HTTP
- Use dlopen/dlsym for plugin loading (POSIX)
- Abstract platform-specific code
- See ARCHITECTURE.md for portability roadmap

---

## Diagnostic Tools

### Checking DLL Exports

```cmd
dumpbin /EXPORTS plugin.dll
```

Expected output should include `PluginInit`.

---

### Checking DLL Dependencies

Use Dependency Walker or:
```cmd
dumpbin /DEPENDENTS plugin.dll
```

---

### Enabling Debug Output

In Visual Studio:
1. Build in Debug mode (not Release)
2. Run with debugger (F5)
3. Check Output window for detailed messages

---

### Memory Leak Detection

Use Visual Studio's diagnostic tools:
1. Debug → Windows → Show Diagnostic Tools
2. Take memory snapshots before and after operations
3. Look for growing heap allocations

---

## Getting Help

If you've tried the solutions above and still have issues:

1. **Check the logs**: Look for error messages in console output

2. **Simplify the scenario**: Try with minimal input to isolate the problem

3. **Gather information**:
   - OS version
   - Visual Studio version
   - Build configuration (Debug/Release, x86/x64)
   - Exact error message
   - Steps to reproduce

4. **Search issues**: Check GitHub issues for similar problems

5. **Report the bug**: Open a new issue with detailed information

## Common Error Messages Reference

| Error Message | Meaning | Solution |
|--------------|---------|----------|
| "Buffer is full" | MAX_LINES reached | Delete lines or increase limit |
| "Line too long" | Exceeds MAX_LINE_LENGTH | Shorten line or increase limit |
| "Invalid line number" | Out of range access | Check line index validity |
| "File not found" | Cannot locate file | Verify path and filename |
| "Permission denied" | Cannot access file | Check permissions |
| "Plugin init failed" | PluginInit returned -1 | Debug plugin initialization |
| "Command not found" | Unknown plugin command | Check command name spelling |
| "Network error" | HTTP request failed | Check connectivity and API |

---

**Still Having Issues?**

Please open an issue on GitHub with:
- Detailed description of the problem
- Steps to reproduce
- Error messages or screenshots
- System information
- What you've already tried

We're here to help! 🛠️
