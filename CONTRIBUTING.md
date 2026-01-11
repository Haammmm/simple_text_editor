# Contributing to SimpleTextEditor

Thank you for your interest in contributing to SimpleTextEditor! This document provides guidelines and instructions for contributing to the project.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Environment](#development-environment)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Bug Reports](#bug-reports)
- [Feature Requests](#feature-requests)
- [Documentation](#documentation)

## Code of Conduct

By participating in this project, you agree to maintain a respectful and inclusive environment:

- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Accept constructive criticism gracefully
- Focus on what is best for the community and project
- Show empathy towards other community members

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Visual Studio** (2019 or later) with:
   - C/C++ development workload
   - Windows SDK
   - C17 and C++20 support

2. **Git** for version control

3. **GitHub account** for submitting pull requests

### Forking the Repository

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/simple_text_editor.git
   cd simple_text_editor
   ```
3. Add the upstream repository:
   ```bash
   git remote add upstream https://github.com/Haammmm/simple_text_editor.git
   ```

### Building the Project

1. Open `Project1.slnx` in Visual Studio
2. Ensure `winhttp.lib` is linked (Project Properties → Linker → Input → Additional Dependencies)
3. Build the solution (Ctrl+Shift+B)
4. Run the application (F5 or Ctrl+F5)

## Development Environment

### Recommended Visual Studio Extensions

- **GitHub Extension for Visual Studio**: For GitHub integration
- **Visual Assist**: Enhanced C/C++ editing
- **ReSharper C++**: Code analysis and refactoring

### Project Structure

```
simple_text_editor/
├── Project1/SimpleTextEditor/    # Main application
│   ├── main.c                     # UI and main entry point
│   ├── text_editor.c/h           # Core text editing
│   ├── plugin_manager.c/h        # Plugin system
│   └── plugin.h                   # Plugin interface
├── Dll1/                          # LLM plugin example
│   ├── openai_agent.cpp          # Plugin implementation
│   └── openai/                    # openai-cpp library
├── plugins/                       # Plugin directory (runtime)
├── docs/                          # Documentation
└── README.md                      # Project overview
```

## How to Contribute

### Types of Contributions

We welcome various types of contributions:

1. **Bug Fixes**: Fix issues in the code
2. **New Features**: Add new functionality
3. **Documentation**: Improve or add documentation
4. **Tests**: Add or improve test coverage
5. **Plugins**: Create new plugins
6. **Code Refactoring**: Improve code quality
7. **Performance Improvements**: Optimize algorithms

### Contribution Workflow

1. **Check Existing Issues**: Look for existing issues or discussions related to your contribution
2. **Create an Issue**: If none exists, create one to discuss your proposal
3. **Get Feedback**: Wait for maintainer feedback before starting significant work
4. **Create a Branch**: Create a feature branch for your work
5. **Make Changes**: Implement your contribution
6. **Test Thoroughly**: Ensure your changes work correctly
7. **Submit Pull Request**: Open a PR with a clear description
8. **Address Feedback**: Respond to review comments
9. **Merge**: Once approved, your PR will be merged

## Coding Standards

### General Principles

1. **Consistency**: Follow existing code style
2. **Simplicity**: Keep code simple and readable
3. **Safety**: Use safe CRT functions (`*_s` variants)
4. **Comments**: Add comments for complex logic only
5. **UTF-8**: Ensure all text operations are UTF-8 aware

### C Code Style

#### Naming Conventions

```c
// Functions: snake_case
int get_line_count(void);
void buffer_init(TextBuffer *buf);

// Types: PascalCase
typedef struct TextBuffer {
    // ...
} TextBuffer;

// Constants: UPPER_CASE
#define MAX_LINES 1000
#define MAX_LINE_LENGTH 4096

// Variables: snake_case
int line_count = 0;
char *filename = NULL;

// Static (internal) functions: prefix with 'static'
static void trim_line_endings(char *line);
```

#### Indentation and Formatting

```c
// 4 spaces for indentation (no tabs)
void function_example(int param) {
    if (condition) {
        // Code here
    } else {
        // Code here
    }
    
    for (int i = 0; i < count; i++) {
        // Loop body
    }
}

// Braces on same line for control structures
if (condition) {
    statement;
}

// Function braces on new line
void function_name(void)
{
    // Function body
}
```

#### Comments

```c
// Single-line comments for brief explanations
int count = 0;  // Initialize counter

/*
 * Multi-line comments for function documentation
 * Format: Javadoc-style
 */
void complex_function(void) {
    // Implementation
}

/* Inline comments for complex logic */
```

### C++ Code Style

#### Naming Conventions

```cpp
// Similar to C, but with additional conventions
namespace lowercase_namespace {

class PascalCaseClass {
private:
    int m_memberVariable;  // Prefix with m_
    
public:
    void memberFunction();
};

}
```

#### Modern C++ Features

- Use `nullptr` instead of `NULL`
- Prefer `std::string` over C-style strings where appropriate
- Use RAII for resource management
- Prefer `const` and `constexpr` where applicable
- Use smart pointers when appropriate

### Safe Programming Practices

#### Always Use Safe CRT Functions

```c
// ✓ Correct
strcpy_s(dest, sizeof(dest), src);
sprintf_s(buffer, sizeof(buffer), format, args);
fopen_s(&fp, filename, mode);

// ✗ Incorrect
strcpy(dest, src);
sprintf(buffer, format, args);
fp = fopen(filename, mode);
```

#### Validate All Inputs

```c
int function_example(TextBuffer *buf, int line) {
    // Always validate pointers
    if (buf == NULL) return -1;
    
    // Always validate array indices
    if (line < 0 || line >= buf->line_count) return -1;
    
    // Proceed with operation
    // ...
}
```

#### Check Return Values

```c
// ✓ Correct
int result = operation();
if (result != 0) {
    // Handle error
    return -1;
}

// ✗ Incorrect
operation();  // Ignoring return value
```

### UTF-8 Considerations

```c
// Be aware of character vs. byte indexing
int char_count = utf8_strlen_chars(str);  // Character count
int byte_count = strlen(str);             // Byte count

// Use character-based operations for user-facing features
int char_offset = utf8_byte_offset(str, char_index);

// Test with multi-byte characters
// Example: "Hello世界" has 7 characters but 13 bytes
```

## Commit Messages

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Build process or auxiliary tool changes

### Examples

```
feat(editor): add undo/redo functionality

Implement undo/redo system using command pattern.
Operations are stored in a history stack with a
configurable limit of 100 operations.

Closes #42
```

```
fix(search): correct KMP algorithm for UTF-8

The KMP search was treating byte offsets as character
offsets, causing incorrect results with multi-byte
characters. Fixed by converting byte positions to
character positions after search.

Fixes #58
```

```
docs(api): add examples for plugin development

Added comprehensive examples demonstrating:
- Basic plugin structure
- Command registration
- Buffer manipulation
- Error handling

```

### Commit Best Practices

1. **Atomic Commits**: Each commit should be a single logical change
2. **Descriptive Messages**: Explain what and why, not how
3. **Reference Issues**: Include issue numbers when applicable
4. **Keep Commits Small**: Easier to review and revert if needed

## Pull Request Process

### Before Submitting

1. **Update from Upstream**:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Build Successfully**: Ensure the project builds without errors

3. **Test Thoroughly**: Test your changes with various scenarios

4. **Update Documentation**: Update relevant documentation

5. **Check Code Style**: Ensure your code follows project conventions

### PR Description Template

```markdown
## Description
Brief description of the changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring
- [ ] Performance improvement

## Related Issues
Closes #XX

## Testing
Describe the testing you performed

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Comments added for complex code
- [ ] Documentation updated
- [ ] No compiler warnings
- [ ] Changes tested with UTF-8 content
- [ ] Plugin compatibility verified (if applicable)
```

### Review Process

1. **Automated Checks**: Ensure CI builds pass (if configured)
2. **Code Review**: Maintainers will review your code
3. **Feedback**: Address any requested changes
4. **Approval**: Once approved, your PR will be merged
5. **Thank You**: Your contribution will be acknowledged!

### Addressing Review Feedback

- Be responsive to review comments
- Ask questions if feedback is unclear
- Make requested changes in new commits
- Don't force-push unless requested
- Be patient and professional

## Bug Reports

### Before Reporting

1. **Search Existing Issues**: Check if the bug is already reported
2. **Verify the Bug**: Ensure it's reproducible
3. **Test Latest Version**: Use the latest code from main branch

### Bug Report Template

```markdown
**Bug Description**
A clear description of the bug

**Steps to Reproduce**
1. Step one
2. Step two
3. ...

**Expected Behavior**
What you expected to happen

**Actual Behavior**
What actually happened

**Environment**
- OS: Windows 10/11
- Visual Studio Version: 2022
- Project Version/Commit: abc123

**Screenshots**
If applicable, add screenshots

**Additional Context**
Any other relevant information
```

### Good Bug Reports

- Specific and detailed
- Include minimal reproduction steps
- Provide error messages and logs
- Describe the impact/severity

## Feature Requests

### Before Requesting

1. **Check Existing Requests**: Look for similar feature requests
2. **Consider Scope**: Ensure it fits the project's goals
3. **Think About Implementation**: Consider feasibility

### Feature Request Template

```markdown
**Feature Description**
Clear description of the feature

**Problem It Solves**
What problem does this feature address?

**Proposed Solution**
Your suggested approach

**Alternative Solutions**
Other approaches considered

**Additional Context**
Mockups, examples, or references
```

### Good Feature Requests

- Clearly explain the use case
- Describe the expected behavior
- Consider edge cases
- Provide examples or mockups

## Documentation

### Types of Documentation

1. **Code Comments**: For complex logic or algorithms
2. **API Documentation**: For public interfaces (see API.md)
3. **User Documentation**: For end-user features (see README.md)
4. **Developer Documentation**: For architecture and design (see ARCHITECTURE.md)

### Documentation Style

- Clear and concise
- Use proper grammar and spelling
- Include code examples where helpful
- Keep documentation up-to-date with code changes
- Use Markdown for formatting

### Documentation Checklist

When adding/modifying features:

- [ ] Update README.md if user-facing
- [ ] Update API.md if plugin API changed
- [ ] Update ARCHITECTURE.md if design changed
- [ ] Add inline comments for complex code
- [ ] Update CHANGELOG.md

## Testing Guidelines

### Manual Testing

Test your changes with:

1. **Empty Buffer**: Start with no content
2. **Large Buffer**: Test with maximum lines (1000)
3. **Long Lines**: Test with lines near 4096 character limit
4. **UTF-8 Content**: Test with Chinese characters, emoji, etc.
5. **Edge Cases**: Test boundary conditions
6. **Error Conditions**: Test with invalid inputs

### Test Scenarios

```
Test Case: Insert substring with UTF-8
Input: Buffer with "Hello世界", insert "!" at character 5
Expected: "Hello!世界"
Actual: [verify]

Test Case: Search for multi-byte pattern
Input: Buffer with "你好世界", search for "世界"
Expected: Found at line 0, column 2
Actual: [verify]
```

### Plugin Testing

If creating a plugin:

1. Test plugin loading
2. Test command registration
3. Test command execution
4. Test with empty buffer
5. Test with full buffer
6. Test error handling

## Getting Help

If you need help:

1. **Check Documentation**: README.md, API.md, ARCHITECTURE.md
2. **Search Issues**: Look for similar questions
3. **Ask Questions**: Open a new issue with the "question" label
4. **Be Specific**: Provide context and what you've tried

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be acknowledged in:
- Git commit history
- Release notes
- Contributors list (if applicable)

Thank you for contributing to SimpleTextEditor! 🎉
