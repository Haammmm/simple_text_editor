# Examples and Usage Guide

## Table of Contents
- [Basic Usage Examples](#basic-usage-examples)
- [Advanced Operations](#advanced-operations)
- [Plugin Usage Examples](#plugin-usage-examples)
- [Real-World Scenarios](#real-world-scenarios)
- [Tips and Tricks](#tips-and-tricks)

## Basic Usage Examples

### Example 1: Creating and Saving a Simple Text File

**Scenario**: Create a new text file with a few lines and save it.

**Steps**:
1. Launch SimpleTextEditor
2. Select option `1` (New/Input Text)
3. Enter the following lines:
   ```
   Line 1: Hello World
   Line 2: This is a simple text editor
   Line 3: Supporting UTF-8 characters: 你好世界
   (Press Enter on empty line to finish)
   ```
4. Select option `3` (Save Text File)
5. Enter filename: `example.txt`
6. Confirm the file is saved

**Expected Output**:
```
Success saving to file 'example.txt', wrote 3 lines
```

---

### Example 2: Opening and Viewing a File

**Scenario**: Open an existing text file and view its contents.

**Steps**:
1. Launch SimpleTextEditor
2. Select option `2` (Open Text File)
3. Enter filename: `example.txt`
4. The file content will be displayed automatically

**Expected Output**:
```
Successfully opened file 'example.txt', read 3 lines

========== Current Text Content ==========
File: example.txt
Total 3 lines
----------------------------------
  1 | Hello World
  2 | This is a simple text editor
  3 | Supporting UTF-8 characters: 你好世界
==================================
```

---

### Example 3: Character Statistics

**Scenario**: Analyze character composition of your text.

**Steps**:
1. Open or input some text
2. Select option `4` (Character Statistics)

**Sample Text**:
```
Hello World 123
你好世界！
```

**Expected Output**:
```
========== Character Statistics ==========
English letters: 10
Chinese characters: 4
Digits: 3
Spaces: 2
Punctuation: 1
Other characters: 0
Total characters: 20
==================================
```

---

### Example 4: Finding Substrings

**Scenario**: Find all occurrences of a specific substring.

**Steps**:
1. Have text in the buffer
2. Select option `5` (Find Substring)
3. Enter the substring to search for

**Sample Text**:
```
Line 1: Hello World
Line 2: Hello Universe
Line 3: World of Hello
```

**Search for**: `Hello`

**Expected Output**:
```
========== Search Results ==========
Search substring: "Hello"
------------------------------
Line 1, Column 9: Hello World
Line 2, Column 9: Hello Universe
Line 3, Column 10: World of Hello
------------------------------
Found 3 matches
==================================
```

---

### Example 5: Inserting Substrings

**Scenario**: Insert text at a specific position.

**Before**:
```
Line 1: Hello World
```

**Steps**:
1. Select option `6` (Insert Substring at Position)
2. Enter line number: `1`
3. Enter column number: `7` (after "Hello ")
4. Enter substring to insert: `Beautiful `

**After**:
```
Line 1: Hello Beautiful World
```

---

### Example 6: Replacing Text

**Scenario**: Replace all occurrences of a word.

**Before**:
```
I like cats. Cats are cute. My cats are fluffy.
```

**Steps**:
1. Select option `7` (Modify Substring)
2. Select `3` (Replace all matching substrings)
3. Enter substring to find: `cats`
4. Enter replacement: `dogs`

**After**:
```
I like dogs. Cats are cute. My dogs are fluffy.
```

**Note**: Search is case-sensitive!

---

### Example 7: Deleting Substrings

**Scenario**: Remove all occurrences of a specific text.

**Before**:
```
This is a test. This is only a test.
```

**Steps**:
1. Select option `8` (Delete Substring)
2. Enter substring to delete: `test`
3. Confirm deletion when prompted

**After**:
```
This is a . This is only a .
```

## Advanced Operations

### Multi-byte Character Handling

**Example**: Working with Chinese text

```
Original: 你好世界
```

**Insert at column 3**:
- Input: Insert "美丽的" at line 1, column 3
- Result: `你好美丽的世界`

**Note**: Column numbers are character-based, not byte-based. The Chinese character "世" is at column 3, even though it's at byte offset 6.

---

### Replacing Single Characters

**Scenario**: Fix a typo by replacing a single character.

**Before**:
```
Helo World
```

**Steps**:
1. Select option `7` (Modify Substring)
2. Select `1` (Modify character at position)
3. Enter line: `1`, column: `3`
4. Enter new character: `ll`

**After**:
```
Hello World
```

---

### Replacing Multi-character Substring

**Scenario**: Replace a portion of text with different length text.

**Before**:
```
I have a cat
```

**Steps**:
1. Select option `7` (Modify Substring)
2. Select `2` (Modify substring at position)
3. Enter line: `1`, column: `10` (start of "cat")
4. Enter length: `3` (length of "cat")
5. Enter new string: `dog`

**After**:
```
I have a dog
```

---

### Working with Long Lines

**Scenario**: Handling lines approaching the limit.

**Maximum Line Length**: 4096 characters

**Behavior**:
- Lines exceeding 4096 characters are truncated
- Warning displayed when truncation occurs
- Original file is not modified unless you save

**Example**:
```
Input: [4100 character line]
Result: [4096 character line] + warning message
```

## Plugin Usage Examples

### Example 1: Loading and Using LLM Plugin

**Prerequisites**:
1. Set environment variable `test_apikey`:
   ```cmd
   set test_apikey=your_api_key_here
   ```
2. Ensure `Dll1.dll` is in `./plugins` directory

**Steps**:
1. Launch SimpleTextEditor
2. Select option `10` (Plugin Management)
3. Select `1` (Scan and load plugins)
4. Select `2` (List plugin commands)
   - You should see `LLMDialog` in the list
5. Select `3` (Execute plugin command)
6. Enter command name: `LLMDialog`
7. Enter your instruction when prompted

**Example Interaction**:
```
> Please capitalize all words in the buffer

[LLM Response]:
I'll capitalize all words in the buffer.

LLM updated buffer via tool
```

---

### Example 2: Custom Plugin - Line Counter

**Plugin Code** (`line_counter.c`):
```c
#include "plugin.h"

static EditorAPI* g_api = NULL;

void count_lines_cmd(void) {
    if (!g_api) return;
    int count = g_api->get_line_count();
    char msg[64];
    sprintf_s(msg, sizeof(msg), "Total lines: %d\n", count);
    g_api->print_msg(msg);
}

__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("CountLines", count_lines_cmd, "Count total lines");
    return 0;
}
```

**Usage**:
1. Compile to `line_counter.dll`
2. Copy to `./plugins` directory
3. Load plugin in editor
4. Execute `CountLines` command

---

### Example 3: Custom Plugin - Text Statistics

**Plugin Code** (`text_stats.cpp`):
```cpp
#include "plugin.h"
#include <string>

static EditorAPI* g_api = nullptr;

void text_stats_cmd(void) {
    if (!g_api) return;
    
    int total_chars = 0;
    int total_words = 0;
    int line_count = g_api->get_line_count();
    
    for (int i = 0; i < line_count; i++) {
        const char* line = g_api->get_line(i);
        if (line) {
            std::string str(line);
            total_chars += str.length();
            
            // Simple word count (split by spaces)
            bool in_word = false;
            for (char c : str) {
                if (c == ' ' || c == '\t') {
                    in_word = false;
                } else if (!in_word) {
                    total_words++;
                    in_word = true;
                }
            }
        }
    }
    
    char msg[256];
    sprintf_s(msg, sizeof(msg), 
        "Lines: %d\nWords: %d\nCharacters: %d\n",
        line_count, total_words, total_chars);
    g_api->print_msg(msg);
}

extern "C" {
__declspec(dllexport) int PluginInit(EditorAPI* api) {
    if (!api) return -1;
    g_api = api;
    api->register_command("TextStats", text_stats_cmd, "Show text statistics");
    return 0;
}
}
```

## Real-World Scenarios

### Scenario 1: Editing Configuration Files

**Task**: Update a configuration file with UTF-8 content.

**Original `config.txt`**:
```
app_name=MyApp
version=1.0
author=作者名字
```

**Steps**:
1. Open `config.txt`
2. Find and replace `version=1.0` with `version=2.0`
3. Save the file

**Commands Used**:
- Option 2: Open file
- Option 7: Modify (Replace all)
- Option 3: Save file

---

### Scenario 2: Cleaning Up Text Data

**Task**: Remove unwanted characters from text.

**Original**:
```
Data[1] = value1
Data[2] = value2
Data[3] = value3
```

**Goal**: Remove all `Data` prefixes.

**Steps**:
1. Open or input the text
2. Option 8: Delete substring
3. Enter: `Data`
4. Confirm deletion

**Result**:
```
[1] = value1
[2] = value2
[3] = value3
```

---

### Scenario 3: Text Analysis

**Task**: Analyze a mixed-language document.

**Sample Document**:
```
Title: Project Report
Author: John Smith (张三)
Date: 2026-01-11

Summary: This document contains 中文 and English text.
Total items: 42
```

**Steps**:
1. Open the document
2. Option 4: Character statistics
3. Review the breakdown

**Analysis**:
- English letters: XX
- Chinese characters: XX
- Digits: XX
- Spaces: XX
- Punctuation: XX

---

### Scenario 4: Batch Text Replacement

**Task**: Update multiple occurrences of a term.

**Original**:
```
The quick brown fox jumps over the lazy dog.
The fox is clever. The dog is lazy.
```

**Goal**: Replace "fox" with "cat" and "dog" with "mouse".

**Steps**:
1. Option 7: Modify → Replace all
   - Replace `fox` with `cat`
2. Option 7: Modify → Replace all
   - Replace `dog` with `mouse`

**Result**:
```
The quick brown cat jumps over the lazy mouse.
The cat is clever. The mouse is lazy.
```

---

### Scenario 5: Creating Templates

**Task**: Create a template for repeated use.

**Template Content**:
```
====================================
Document Title: [TITLE]
Date: [DATE]
Author: [AUTHOR]
====================================

Content goes here...

====================================
End of Document
====================================
```

**Usage**:
1. Create and save template as `template.txt`
2. When needed:
   - Open `template.txt`
   - Replace placeholders ([TITLE], [DATE], [AUTHOR])
   - Save as new filename

## Tips and Tricks

### Tip 1: Quick Buffer Check

**Before any major operation**, display the current text (Option 9) to verify the buffer state.

---

### Tip 2: Incremental Saves

**When working on important files**, save frequently using Option 3. The editor tracks the modified flag and will warn you if you try to exit with unsaved changes.

---

### Tip 3: UTF-8 Text Entry

**For UTF-8 characters**:
- Make sure your console is set to UTF-8 code page:
  ```cmd
  chcp 65001
  ```
- Or use a text editor to prepare the text, then paste into the console

---

### Tip 4: Finding Multi-byte Patterns

**When searching for Chinese text or emoji**, remember that the search is exact and case-sensitive. Copy and paste the exact characters to avoid typos.

---

### Tip 5: Plugin Development

**Start simple**:
1. Begin with a plugin that just prints a message
2. Test loading and command execution
3. Gradually add buffer operations
4. Test with edge cases (empty buffer, full buffer, UTF-8)

---

### Tip 6: Handling Large Files

**Maximum capacity**: 1000 lines, 4096 characters per line

**If your file is larger**:
- Consider splitting it into multiple files
- Use the editor for sections or excerpts
- Future versions may support larger capacities

---

### Tip 7: Search and Replace Workflow

**Efficient workflow**:
1. Use Option 5 (Find) to preview matches
2. Review the count and positions
3. Use Option 7 (Modify → Replace all) to execute
4. Use Option 9 (Display) to verify results

---

### Tip 8: Plugin Reloading

**To reload plugins after modification**:
- Exit the editor
- Replace the plugin DLL
- Restart the editor
- Reload plugins (Option 10 → 1)

**Note**: Windows locks loaded DLLs, so you must exit the editor before replacing them.

---

### Tip 9: Keyboard Navigation

**In plugin menu** (Option 10), use:
- Option 2: Quickly check available commands
- Option 3: Execute by typing command name exactly as listed

---

### Tip 10: Error Recovery

**If an operation fails**:
1. Check the error message for specific cause
2. Verify input parameters (line numbers, column numbers)
3. Ensure the buffer has content if needed
4. Check file permissions for file operations
5. Verify environment variables for plugins (e.g., `test_apikey`)

---

### Tip 11: Character Position Awareness

**Remember**: Column numbers are character-based, not byte-based.

**Example**:
```
Text: "Hello世界"
- Character 1: H
- Character 2: e
- Character 3: l
- Character 4: l
- Character 5: o
- Character 6: 世
- Character 7: 界
```

To insert after "Hello": Use column 6, not byte offset 8.

---

### Tip 12: LLM Plugin Best Practices

**For best results**:
1. Start with clear, specific instructions
2. Describe what you want the buffer to contain
3. The LLM can see the current buffer content
4. Be patient - API calls may take a few seconds
5. Check the buffer after LLM operations (Option 9)

**Example Instructions**:
- "Convert all text to uppercase"
- "Add line numbers to each line"
- "Summarize the content in 3 bullet points"
- "Translate the buffer to Chinese"

---

## Common Patterns

### Pattern 1: Safe Editing Workflow

```
1. Open file (Option 2)
2. Display content (Option 9) - verify loaded correctly
3. Make changes (Options 6, 7, or 8)
4. Display again (Option 9) - verify changes
5. Save file (Option 3)
```

---

### Pattern 2: Text Cleanup

```
1. Load text
2. Delete unwanted patterns (Option 8) - repeat as needed
3. Replace problematic text (Option 7)
4. Verify with display (Option 9)
5. Save
```

---

### Pattern 3: Analysis and Modification

```
1. Load text
2. Run statistics (Option 4) - understand composition
3. Find specific patterns (Option 5) - locate targets
4. Modify as needed (Options 6, 7, or 8)
5. Re-run statistics (Option 4) - verify changes
6. Save
```

---

## Sample Workflows

### Workflow: Creating a Bilingual Document

```
Step 1: Input Text (Option 1)
--------
Enter bilingual content:
English: Hello
中文: 你好
English: World
中文: 世界
(empty line)

Step 2: Statistics (Option 4)
--------
Review character breakdown

Step 3: Save (Option 3)
--------
Filename: bilingual.txt

Result: File saved with UTF-8 encoding
```

---

### Workflow: Text Transformation

```
Step 1: Open file (Option 2)
--------
Open: input.txt

Step 2: Find pattern (Option 5)
--------
Find: "TODO"
Result: 5 matches found

Step 3: Replace (Option 7 → 3)
--------
Find: "TODO"
Replace: "DONE"
Result: 5 replacements made

Step 4: Verify (Option 9)
--------
Display and review changes

Step 5: Save (Option 3 → 1)
--------
Save to current file
```

---

## Next Steps

- Review the [API Documentation](API.md) for plugin development
- Check [ARCHITECTURE.md](ARCHITECTURE.md) for design details
- See [CONTRIBUTING.md](CONTRIBUTING.md) to contribute
- Read [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for common issues

---

**Happy Editing!** 🎉
