# ArgParser - Command Line Argument Parser

## Overview

The `ArgParser` is a lightweight, header-only command line argument parsing library for C++20. It provides a fluent API for defining and parsing command line options with support for flags, value options, and positional arguments.

## Features

- **Fluent API**: Method chaining for easy configuration
- **Flag Options**: Boolean switches (e.g., `--help`, `-h`)
- **Value Options**: Options with arguments (e.g., `--port 4242`, `-p 4242`)
- **Positional Arguments**: Unnamed arguments (e.g., `config.toml`)
- **Automatic Help Generation**: Properly aligned usage messages
- **Type-Safe Numeric Parsing**: Template-based number parsing with range validation
- **Duplicate Detection**: Warns when registering duplicate options
- **Thread-Safe Logging**: Integrated with the Logger system

## File Structure

```
src/common/ArgParser/
├── ArgParser.hpp      # Main parser class
├── Option.hpp         # Option and PositionalArg structures
├── ParseResult.hpp    # Result enumeration
└── NumberParser.hpp   # Numeric parsing utilities
```

## Components

### ParseResult Enumeration

Defines the result of parsing operations:

```cpp
enum class ParseResult {
    Success,    // Parsing succeeded, continue execution
    Exit,       // Parsing succeeded but should exit (e.g., --help)
    Error       // Parsing failed due to an error
};
```

### Option Structure

Defines a command line option:

```cpp
struct Option {
    std::string shortOpt;       // Short option (e.g., "-h")
    std::string longOpt;        // Long option (e.g., "--help")
    std::string description;    // Description for help message
    bool hasArg = false;        // Whether option takes an argument
    std::string argName;        // Name of the argument (for help message)
};
```

### PositionalArg Structure

Defines a positional argument:

```cpp
struct PositionalArg {
    std::string name;           // Name of the argument (for help message)
    std::string description;    // Description for help message
    bool required = true;       // Whether this argument is required
};
```

### ArgParser Class

The main parser class with the following public API:

| Method | Description |
|--------|-------------|
| `programName(name)` | Set the program name for usage message |
| `flag(short, long, desc, handler)` | Add a flag option (no argument) |
| `option(short, long, argName, desc, handler)` | Add an option with argument |
| `positional(name, desc, handler, required)` | Add a positional argument |
| `parse(args)` | Parse command line arguments |
| `printUsage()` | Print formatted usage message |

### NumberParser Template

Type-safe numeric parsing with range validation:

```cpp
template<typename T>
[[nodiscard]] std::optional<T> parseNumber(
    std::string_view str,
    std::string_view name,
    T minVal = std::numeric_limits<T>::min(),
    T maxVal = std::numeric_limits<T>::max()
) noexcept;
```

## Usage Examples

### Basic Usage

```cpp
#include "ArgParser.hpp"
#include <atomic>

int main(int argc, char* argv[]) {
    uint16_t port = 4242;
    size_t maxPlayers = 4;
    std::string configPath;
    
    rtype::ArgParser parser;
    parser.programName("r-type_server")
        .flag("-h", "--help", "Display this help message", [&parser]() {
            parser.printUsage();
            return rtype::ParseResult::Exit;
        })
        .option("-p", "--port", "port", "Server port (1024-65535)", 
            [&port](std::string_view value) {
                auto result = rtype::parseNumber<uint16_t>(value, "port", 1024, 65535);
                if (!result) return rtype::ParseResult::Error;
                port = *result;
                return rtype::ParseResult::Success;
            })
        .option("-m", "--max-players", "count", "Maximum players (1-8)",
            [&maxPlayers](std::string_view value) {
                auto result = rtype::parseNumber<size_t>(value, "max-players", 1, 8);
                if (!result) return rtype::ParseResult::Error;
                maxPlayers = *result;
                return rtype::ParseResult::Success;
            })
        .positional("config", "Path to configuration file",
            [&configPath](std::string_view value) {
                configPath = value;
                return rtype::ParseResult::Success;
            }, false);  // optional positional argument
    
    // Convert argv to vector (excluding program name)
    std::vector<std::string_view> args(argv + 1, argv + argc);
    
    switch (parser.parse(args)) {
        case rtype::ParseResult::Success:
            // Continue with program execution
            break;
        case rtype::ParseResult::Exit:
            return 0;  // Clean exit (e.g., --help was used)
        case rtype::ParseResult::Error:
            return 1;  // Error occurred
    }
    
    // Use parsed values...
    return 0;
}
```

### Generated Help Output

```
Usage: r-type_server [options] [config]
Options:
  -h, --help                Display this help message
  -p, --port <port>         Server port (1024-65535)
  -m, --max-players <count> Maximum players (1-8)
Arguments:
  config    Path to configuration file (optional)
```

## Handler Return Values

Handlers should return appropriate `ParseResult` values:

| Scenario | Return Value |
|----------|--------------|
| Handler succeeded, continue parsing | `ParseResult::Success` |
| Handler succeeded, exit program (e.g., `--help`) | `ParseResult::Exit` |
| Handler failed (validation error) | `ParseResult::Error` |

## Error Handling

The parser handles errors gracefully:

1. **Unknown Options**: Logs error and prints usage
2. **Missing Arguments**: Logs error when option requires value but none provided
3. **Missing Required Positionals**: Logs error and prints usage
4. **Duplicate Options**: Logs warning and ignores duplicate registration
5. **Extra Positional Arguments**: Logs warning but continues

## Integration with Logger

ArgParser uses the Logger system for error and warning messages:

```cpp
LOG_ERROR(std::format("Unknown option: {}", key));
LOG_WARNING(std::format("Duplicate option: {}/{}", shortOpt, longOpt));
```

## Design Decisions

### Shared Pointer Handlers

Handlers are stored as `shared_ptr` to avoid duplication when both short and long options map to the same handler:

```cpp
auto sharedHandler = std::make_shared<std::function<ParseResult()>>(std::move(handler));
_flagHandlers[std::string(shortOpt)] = sharedHandler;
_flagHandlers[std::string(longOpt)] = sharedHandler;
```

### Const Parse Method

The `parse()` method is `const` to ensure the parser configuration is not modified during parsing:

```cpp
[[nodiscard]] ParseResult parse(const std::vector<std::string_view>& args) const;
```

### String View for Input

Input strings use `std::string_view` to avoid unnecessary copies:

```cpp
ArgParser& flag(std::string_view shortOpt, std::string_view longOpt, ...);
```

## Thread Safety

- The `ArgParser` class itself is not thread-safe for configuration
- Once configured, `parse()` can be called from multiple threads (it's `const`)
- Error logging is thread-safe through the Logger system

## Dependencies

- C++20 (for `std::format`, `contains()`, etc.)
- Logger system (`../Logger/Macros.hpp`)

## Related Documentation

- [Logger Technical Documentation](./Logger.md)
- [Server Architecture](../architecture/files_architecture.md)
