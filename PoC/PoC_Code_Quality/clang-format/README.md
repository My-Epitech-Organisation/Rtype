# Clang-Format PoC

This PoC demonstrates setting up clang-format for consistent C++ code styling.

## Setup

- `.clang-format`: Generated from Google style, modified with `IndentWidth: 4`.
- `messy_code_before.cpp`: Example of unformatted (messy) code.
- `messy_code_after.cpp`: The same code after clang-format.
- Running `clang-format -i messy_code_before.cpp` produces the cleaned-up version.

## Usage

To format files:

```bash
clang-format -i <file.cpp>
```

For VS Code "Format on Save":

- Install the C/C++ extension.
- Add to settings.json:

```json
{
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "file"
}
```

## Exit Criteria

Formatting a messy file automatically cleans it up: ✅ (compare `messy_code_before.cpp` and `messy_code_after.cpp`).
