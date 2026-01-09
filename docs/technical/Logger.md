# Logger System - Technical Documentation

## Overview

The Logger system provides a robust, thread-safe logging infrastructure for the R-Type project. It supports multiple log levels, file output with timestamps, colored console output, and seamless integration with the argument parser for runtime configuration.

## Features

- **Thread-Safe Logging**: All logging operations are protected by mutex locks
- **Multiple Log Levels**: DEBUG, INFO, WARNING, ERROR, FATAL, and NONE
- **File Output**: Automatic log file creation with timestamped filenames
- **Colored Console Output**: ANSI color codes for visual distinction (can be disabled)
- **ISO 8601 Timestamps**: Millisecond-precision timestamps for all log entries
- **Singleton Pattern**: Global access through `Logger::instance()`
- **Stream-Style Macros**: Convenient logging with `LOG_INFO("Value: " << value)`
- **Performance**: Buffered file I/O with automatic flushing

## File Structure

```
lib/common/src/Logger/
├── Logger.hpp           # Main logger class (singleton)
├── LogLevel.hpp         # Log level enumeration
├── Macros.hpp          # Convenient logging macros
├── FileWriter.hpp      # Thread-safe file output handler
├── Timestamp.hpp       # ISO 8601 timestamp utilities
└── ColorFormatter.hpp  # ANSI color code support
```

## Components

### Logger Class

The main logger class implemented as a singleton:

```cpp
auto& logger = rtype::Logger::instance();
logger.setLogLevel(rtype::LogLevel::Info);
logger.setLogFile("logs/app.log");
logger.info("Application started");
```

#### Key Methods

| Method | Description |
|--------|-------------|
| `instance()` | Get singleton instance |
| `setLogLevel(level)` | Set minimum log level |
| `getLogLevel()` | Get current log level |
| `setLogFile(path, append)` | Enable file logging |
| `closeFile()` | Close log file |
| `generateLogFilename(prefix, dir)` | Generate timestamped filename |
| `setColorEnabled(enabled)` | Enable/disable colored output |
| `isColorEnabled()` | Check if colors are enabled |
| `debug(msg)` | Log debug message |
| `info(msg)` | Log info message |
| `warning(msg)` | Log warning message |
| `error(msg)` | Log error message |
| `fatal(msg)` | Log fatal error message |

### Log Levels

```cpp
enum class LogLevel {
    Debug = 0,    // Verbose debugging information
    Info = 1,     // Informational messages
    Warning = 2,  // Warning messages
    Error = 3,    // Error messages
    Fatal = 4,    // Fatal error messages (critical failures)
    None = 5      // Disable all logging
};
```

**Level Ordering**: Messages are only logged if their level is >= the configured minimum level.

**Console Output Routing**:
- `DEBUG` and `INFO` → stdout
- `WARNING`, `ERROR`, and `FATAL` → stderr

### ColorFormatter

Provides ANSI color codes for console output:

| Log Level | Color | ANSI Code |
|-----------|-------|-----------|
| DEBUG | Cyan | `\033[36m` |
| INFO | Green | `\033[32m` |
| WARNING | Yellow | `\033[33m` |
| ERROR | Red | `\033[31m` |
| FATAL | Bright Red | `\033[91m` |

**Platform Behavior**:
- Unix/Linux: Colors enabled by default
- Windows: Colors disabled by default (requires Windows Terminal or similar)

### Logging Macros

Convenient macros for stream-style logging:

```cpp
LOG_DEBUG("Starting operation " << operationId);
LOG_INFO("Server listening on port " << port);
LOG_WARNING("Connection timeout for client " << clientId);
LOG_ERROR("Failed to open file: " << filename);
LOG_FATAL("Critical system failure: " << error);
```

**Features**:
- Support for stream operators (`<<`)
- Handle expressions with commas (template arguments, initializer lists)
- Thread-safe
- Automatic string conversion

## Usage Examples

### Basic Usage

```cpp
#include "Logger/Macros.hpp"

int main() {
    auto& logger = rtype::Logger::instance();
    logger.setLogLevel(rtype::LogLevel::Info);
    
    LOG_INFO("Application started");
    LOG_DEBUG("This won't appear (level too low)");
    LOG_WARNING("Low memory");
    LOG_ERROR("Connection failed");
}
```

### File Logging with Timestamped Filename

```cpp
auto& logger = rtype::Logger::instance();

// Generate: logs/server_session_2025-12-18_14-30-45.log
const auto logFile = rtype::Logger::generateLogFilename("server_session");

if (logger.setLogFile(logFile, false)) {  // false = overwrite
    LOG_INFO("Logging to: " << logFile.string());
} else {
    LOG_ERROR("Failed to open log file");
}
```

### Integration with ArgParser

Server example:

```cpp
int main(int argc, char** argv) {
    bool verbose = false;
    bool noColor = false;
    
    // Parse arguments
    rtype::ArgParser parser;
    parser.programName(argv[0])
        .flag("--debug", "Enable debug logs", [&verbose]() {
            verbose = true;
            return rtype::ParseResult::Success;
        })
        .flag("--no-color", "Disable colors", [&noColor]() {
            noColor = true;
            return rtype::ParseResult::Success;
        });
    
    std::vector<std::string_view> args(argv + 1, argv + argc);
    if (parser.parse(args) != rtype::ParseResult::Success) {
        return 1;
    }
    
    // Configure logger
    auto& logger = rtype::Logger::instance();
    logger.setLogLevel(verbose ? rtype::LogLevel::Debug : rtype::LogLevel::Info);
    logger.setColorEnabled(!noColor);
    
    const auto logFile = rtype::Logger::generateLogFilename("server_session");
    logger.setLogFile(logFile);
    
    LOG_INFO("Server starting...");
    // ...
}
```

### Multi-Threaded Usage

The logger is fully thread-safe:

```cpp
void workerThread(int id) {
    LOG_INFO("Worker " << id << " started");
    // Thread-safe logging from multiple threads
    LOG_DEBUG("Processing task " << id);
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(workerThread, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}
```

## Log Format

### Console Output (with colors)

```
[color][2025-12-18 14:30:45.123] [INFO] Server started[reset]
[color][2025-12-18 14:30:45.456] [WARNING] High CPU usage[reset]
[color][2025-12-18 14:30:45.789] [ERROR] Connection lost[reset]
```

### File Output (no colors)

```
[2025-12-18 14:30:45.123] [INFO] Server started
[2025-12-18 14:30:45.456] [WARNING] High CPU usage
[2025-12-18 14:30:45.789] [ERROR] Connection lost
```

## Performance Considerations

### File I/O Buffering

The FileWriter automatically flushes after each write to ensure logs are persisted immediately, even if the application crashes. For high-performance scenarios where this is problematic:

```cpp
// Option 1: Reduce log level in production
logger.setLogLevel(rtype::LogLevel::Warning);  // Only warnings and above

// Option 2: Disable file logging for performance-critical sections
logger.closeFile();
// ... performance-critical code ...
logger.setLogFile(logFile);
```

### Thread Contention

Each log call acquires a mutex. For extremely high-frequency logging:
- Use DEBUG level for verbose logs and disable in production
- Consider log aggregation (batch multiple related logs)
- Profile to identify hot paths

## Testing

### Unit Tests

The logger includes comprehensive unit tests in `tests/common/test_logger.cpp`:

- Log level filtering
- File output correctness
- Thread safety with concurrent logging
- Color formatting
- Timestamp generation
- Auto-generated filenames
- FATAL log level
- Mock logger for testing other components

### Testing with Mock Logger

```cpp
#include <gtest/gtest.h>
#include "common/src/Logger/Logger.hpp"

class MockLogger : public rtype::Logger {
public:
    std::vector<std::string> messages;
    
    void info(const std::string& msg) override {
        messages.push_back(msg);
        Logger::info(msg);
    }
};

TEST(MyTest, LogsCorrectly) {
    MockLogger mockLogger;
    rtype::Logger::setInstance(mockLogger);
    
    // Your code that logs...
    
    EXPECT_EQ(mockLogger.messages.size(), 1);
    EXPECT_EQ(mockLogger.messages[0], "Expected message");
    
    rtype::Logger::resetInstance();
}
```

## Design Decisions

### Singleton Pattern

**Rationale**: Global access for logging from any component without dependency injection overhead.

**Alternative Considered**: Dependency injection - rejected due to added complexity for a utility that should be universally accessible.

### Mutex-Based Thread Safety

**Rationale**: Simple, correct, and sufficient for typical logging frequency.

**Alternative Considered**: Lock-free queue - rejected as premature optimization for current requirements.

### Immediate File Flushing

**Rationale**: Ensures crash recovery - logs are persisted even if application terminates unexpectedly.

**Trade-off**: Performance cost acceptable for reliability in debugging production issues.

### ANSI Color Codes

**Rationale**: Widely supported on Unix/Linux, aids in visual parsing of logs during development.

**Windows Consideration**: Disabled by default; can be enabled for Windows Terminal or ConEmu.

### ISO 8601 Timestamps

**Rationale**: Unambiguous, sortable, internationally recognized format.

**Format**: `YYYY-MM-DD HH:MM:SS.mmm` with millisecond precision.

## Integration with Build System

The Logger is part of the `common` library:

```cmake
# lib/common/CMakeLists.txt
add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
```

Usage in other targets:

```cmake
target_link_libraries(my_target PRIVATE common)
```

## Command-Line Flags

### Server Flags

```bash
./r-type_server --help
./r-type_server --debug          # Enable DEBUG level logs
./r-type_server --verbose        # Same as --debug
./r-type_server --no-color       # Disable colored output
./r-type_server -p 4242          # Set port (logs reflect config)
```

### Client Flags

```bash
./r-type_client --help
./r-type_client --debug          # Enable DEBUG level logs
./r-type_client --verbose        # Same as --debug
./r-type_client --no-color       # Disable colored output
./r-type_client -s 192.168.1.10  # Server host
./r-type_client -p 4242          # Server port
```

## Best Practices

### When to Use Each Level

| Level | Use Case | Example |
|-------|----------|---------|
| DEBUG | Detailed trace for development | `LOG_DEBUG("Parsing packet: " << data)` |
| INFO | Important operational events | `LOG_INFO("Server started on port " << port)` |
| WARNING | Recoverable issues | `LOG_WARNING("Client timeout: " << clientId)` |
| ERROR | Errors that affect functionality | `LOG_ERROR("Failed to load config: " << err)` |
| FATAL | Critical failures requiring termination | `LOG_FATAL("Out of memory")` |

### Recommendations

1. **Production**: Set level to `INFO` or `WARNING`
2. **Development**: Use `DEBUG` level
3. **File Logging**: Always enable for servers
4. **Thread Safety**: Logger is thread-safe; use freely from any thread
5. **Performance**: Avoid logging in tight loops; use DEBUG level that can be disabled
6. **Error Handling**: Log before throwing exceptions to aid debugging

## Related Documentation

- [ArgParser Documentation](ArgParser.md)
- [Testing Guide](../website/docs/testing-guide.md)
- [Server Architecture](architecture/)

## Future Enhancements

Potential improvements for consideration:

- Asynchronous logging queue for high-performance scenarios
- Log rotation (file size/age limits)
- Structured logging (JSON format option)
- Remote logging (syslog, network)
- Per-module log level configuration
- Log filtering by pattern/regex

## Changelog

### Current Version (2025-12-18)
- ✅ Added FATAL log level
- ✅ Implemented ANSI color support
- ✅ Auto-generated timestamped filenames
- ✅ Integration with ArgParser (--debug, --no-color flags)
- ✅ Comprehensive unit tests
- ✅ Thread-safe singleton implementation
- ✅ ISO 8601 timestamps with millisecond precision
