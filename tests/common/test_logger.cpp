/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for Logger system
*/

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "common/src/Logger/FileWriter.hpp"
#include "common/src/Logger/LogLevel.hpp"
#include "common/src/Logger/Logger.hpp"
#include "common/src/Logger/Macros.hpp"
#include "common/src/Logger/Timestamp.hpp"

using namespace rtype;

// Helper function to safely remove files with Windows-specific handling
void safeRemoveFile(const std::filesystem::path& filePath) {
#ifdef _WIN32
    // On Windows, file handles may not be immediately released after closing
    // Retry removal with small delays
    for (int attempt = 0; attempt < 10; ++attempt) {
        std::error_code ec;
        std::filesystem::remove(filePath, ec);
        if (!ec) {
            return; // Successfully removed
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // If we get here, removal failed after retries - this will throw
    std::filesystem::remove(filePath);
#else
    // On other platforms, remove immediately
    std::filesystem::remove(filePath);
#endif
}

// Helper function to generate unique test file names
std::filesystem::path getUniqueTestFile(const std::string& baseName) {
    static std::atomic<int> counter{0};
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           std::format("{}_{}_{}.log", baseName, timestamp, ++counter);
}

// ============================================================================
// LogLevel Tests
// ============================================================================

TEST(LogLevelTest, EnumOrdering) {
    EXPECT_LT(static_cast<int>(LogLevel::Debug),
              static_cast<int>(LogLevel::Info));
    EXPECT_LT(static_cast<int>(LogLevel::Info),
              static_cast<int>(LogLevel::Warning));
    EXPECT_LT(static_cast<int>(LogLevel::Warning),
              static_cast<int>(LogLevel::Error));
    EXPECT_LT(static_cast<int>(LogLevel::Error),
              static_cast<int>(LogLevel::None));
}

TEST(LogLevelTest, ToStringDebug) {
    EXPECT_EQ(toString(LogLevel::Debug), "DEBUG");
}

TEST(LogLevelTest, ToStringInfo) {
    EXPECT_EQ(toString(LogLevel::Info), "INFO");
}

TEST(LogLevelTest, ToStringWarning) {
    EXPECT_EQ(toString(LogLevel::Warning), "WARNING");
}

TEST(LogLevelTest, ToStringError) {
    EXPECT_EQ(toString(LogLevel::Error), "ERROR");
}

TEST(LogLevelTest, ToStringNone) {
    EXPECT_EQ(toString(LogLevel::None), "NONE");
}

TEST(LogLevelTest, ToStringIsConstexpr) {
    constexpr auto str = toString(LogLevel::Info);
    EXPECT_EQ(str, "INFO");
}

// ============================================================================
// Timestamp Tests
// ============================================================================

TEST(TimestampTest, FormatMatchesExpectedPattern) {
    const std::string timestamp = Timestamp::now();

    // Expected format: YYYY-MM-DD HH:MM:SS.mmm
    std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})");
    EXPECT_TRUE(std::regex_match(timestamp, pattern))
        << "Timestamp '" << timestamp << "' doesn't match expected format";
}

TEST(TimestampTest, ReturnsValidYear) {
    const std::string timestamp = Timestamp::now();
    const int year = std::stoi(timestamp.substr(0, 4));
    EXPECT_GE(year, 2024);
    EXPECT_LE(year, 2100);
}

TEST(TimestampTest, ReturnsValidMonth) {
    const std::string timestamp = Timestamp::now();
    const int month = std::stoi(timestamp.substr(5, 2));
    EXPECT_GE(month, 1);
    EXPECT_LE(month, 12);
}

TEST(TimestampTest, ReturnsValidDay) {
    const std::string timestamp = Timestamp::now();
    const int day = std::stoi(timestamp.substr(8, 2));
    EXPECT_GE(day, 1);
    EXPECT_LE(day, 31);
}

TEST(TimestampTest, ReturnsValidHour) {
    const std::string timestamp = Timestamp::now();
    const int hour = std::stoi(timestamp.substr(11, 2));
    EXPECT_GE(hour, 0);
    EXPECT_LE(hour, 23);
}

TEST(TimestampTest, ReturnsValidMinute) {
    const std::string timestamp = Timestamp::now();
    const int minute = std::stoi(timestamp.substr(14, 2));
    EXPECT_GE(minute, 0);
    EXPECT_LE(minute, 59);
}

TEST(TimestampTest, ReturnsValidSecond) {
    const std::string timestamp = Timestamp::now();
    const int second = std::stoi(timestamp.substr(17, 2));
    EXPECT_GE(second, 0);
    EXPECT_LE(second, 59);
}

TEST(TimestampTest, ReturnsValidMilliseconds) {
    const std::string timestamp = Timestamp::now();
    const int millis = std::stoi(timestamp.substr(20, 3));
    EXPECT_GE(millis, 0);
    EXPECT_LE(millis, 999);
}

TEST(TimestampTest, TimestampsAreMonotonicallyIncreasing) {
    const std::string ts1 = Timestamp::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const std::string ts2 = Timestamp::now();

    EXPECT_LE(ts1, ts2);
}

// ============================================================================
// FileWriter Tests
// ============================================================================

class FileWriterTest : public ::testing::Test {
   protected:
    std::filesystem::path testFilePath;

    void SetUp() override {
        testFilePath =
            std::filesystem::temp_directory_path() / "test_filewriter.log";
        std::filesystem::remove(testFilePath);
    }

    void TearDown() override { safeRemoveFile(testFilePath); }

    std::string readFileContents() {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

TEST_F(FileWriterTest, InitiallyNotOpen) {
    FileWriter writer;
    EXPECT_FALSE(writer.isOpen());
}

TEST_F(FileWriterTest, OpenCreatesFile) {
    FileWriter writer;
    EXPECT_TRUE(writer.open(testFilePath));
    EXPECT_TRUE(writer.isOpen());
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(FileWriterTest, OpenFailsForInvalidPath) {
    FileWriter writer;
    std::filesystem::path invalidPath = "/nonexistent/directory/file.log";
    EXPECT_FALSE(writer.open(invalidPath));
    EXPECT_FALSE(writer.isOpen());
}

TEST_F(FileWriterTest, CloseClosesFile) {
    FileWriter writer;
    writer.open(testFilePath);
    writer.close();
    EXPECT_FALSE(writer.isOpen());
}

TEST_F(FileWriterTest, WriteWritesToFile) {
    FileWriter writer;
    writer.open(testFilePath);
    writer.write("Test message");
    writer.close();

    const std::string contents = readFileContents();
    EXPECT_EQ(contents, "Test message\n");
}

TEST_F(FileWriterTest, MultipleWritesAppendNewlines) {
    FileWriter writer;
    writer.open(testFilePath);
    writer.write("Line 1");
    writer.write("Line 2");
    writer.close();

    const std::string contents = readFileContents();
    EXPECT_EQ(contents, "Line 1\nLine 2\n");
}

TEST_F(FileWriterTest, WriteDoesNothingWhenNotOpen) {
    FileWriter writer;
    writer.write("This should not be written");
    EXPECT_FALSE(std::filesystem::exists(testFilePath));
}

TEST_F(FileWriterTest, AppendModeAppendsToExistingFile) {
    {
        std::ofstream file(testFilePath);
        file << "Existing content\n";
    }

    FileWriter writer;
    writer.open(testFilePath, true);
    writer.write("New content");
    writer.close();

    const std::string contents = readFileContents();
    EXPECT_EQ(contents, "Existing content\nNew content\n");
}

TEST_F(FileWriterTest, OverwriteModeReplacesExistingFile) {
    {
        std::ofstream file(testFilePath);
        file << "Existing content\n";
    }

    FileWriter writer;
    writer.open(testFilePath, false);
    writer.write("New content");
    writer.close();

    const std::string contents = readFileContents();
    EXPECT_EQ(contents, "New content\n");
}

TEST_F(FileWriterTest, GetFilePathReturnsCorrectPath) {
    FileWriter writer;
    writer.open(testFilePath);
    EXPECT_EQ(writer.getFilePath(), testFilePath);
}

TEST_F(FileWriterTest, GetFilePathEmptyWhenNotOpen) {
    FileWriter writer;
    EXPECT_TRUE(writer.getFilePath().empty());
}

TEST_F(FileWriterTest, OpenClosesExistingFileFirst) {
    auto secondPath =
        std::filesystem::temp_directory_path() / "test_filewriter2.log";
    safeRemoveFile(secondPath);

    FileWriter writer;
    writer.open(testFilePath);
    writer.write("First file");
    writer.open(secondPath);
    writer.write("Second file");
    writer.close();

    safeRemoveFile(secondPath);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(FileWriterTest, ThreadSafetyMultipleWrites) {
    FileWriter writer;
    writer.open(testFilePath);

    constexpr int numThreads = 10;
    constexpr int writesPerThread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&writer, i]() {
            for (int j = 0; j < writesPerThread; ++j) {
                writer.write("Thread " + std::to_string(i) + " message " +
                             std::to_string(j));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    writer.close();

    const std::string contents = readFileContents();
    int lineCount = 0;
    for (char c : contents) {
        if (c == '\n') {
            ++lineCount;
        }
    }
    EXPECT_EQ(lineCount, numThreads * writesPerThread);
}

// ============================================================================
// Logger Tests
// ============================================================================

class LoggerTest : public ::testing::Test {
   protected:
    Logger logger;
    std::filesystem::path testFilePath;

    void SetUp() override {
        testFilePath =
            std::filesystem::temp_directory_path() / "test_logger.log";
        std::filesystem::remove(testFilePath);
        logger.setLogLevel(LogLevel::Debug);
    }

    void TearDown() override {
        logger.closeFile();
        safeRemoveFile(testFilePath);
    }

    std::string readFileContents() {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

TEST_F(LoggerTest, DefaultLogLevelIsDebug) {
    Logger freshLogger;
    EXPECT_EQ(freshLogger.getLogLevel(), LogLevel::Debug);
}

TEST_F(LoggerTest, SetLogLevelChangesLevel) {
    logger.setLogLevel(LogLevel::Warning);
    EXPECT_EQ(logger.getLogLevel(), LogLevel::Warning);
}

TEST_F(LoggerTest, SetLogFileCreatesFile) {
    EXPECT_TRUE(logger.setLogFile(testFilePath));
    EXPECT_TRUE(logger.isFileLoggingEnabled());
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

TEST_F(LoggerTest, SetLogFileFailsForInvalidPath) {
    std::filesystem::path invalidPath = "/nonexistent/directory/file.log";
    EXPECT_FALSE(logger.setLogFile(invalidPath));
    EXPECT_FALSE(logger.isFileLoggingEnabled());
}

TEST_F(LoggerTest, CloseFileDisablesFileLogging) {
    logger.setLogFile(testFilePath);
    logger.closeFile();
    EXPECT_FALSE(logger.isFileLoggingEnabled());
}

TEST_F(LoggerTest, InfoWritesToFile) {
    logger.setLogFile(testFilePath);
    logger.info("Test info message");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("[INFO]") != std::string::npos);
    EXPECT_TRUE(contents.find("Test info message") != std::string::npos);
}

TEST_F(LoggerTest, WarningWritesToFile) {
    logger.setLogFile(testFilePath);
    logger.warning("Test warning message");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("[WARNING]") != std::string::npos);
    EXPECT_TRUE(contents.find("Test warning message") != std::string::npos);
}

TEST_F(LoggerTest, ErrorWritesToFile) {
    logger.setLogFile(testFilePath);
    logger.error("Test error message");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("[ERROR]") != std::string::npos);
    EXPECT_TRUE(contents.find("Test error message") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelFilteringInfo) {
    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Warning);
    logger.info("This should not appear");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("This should not appear") == std::string::npos);
}

TEST_F(LoggerTest, LogLevelFilteringDebug) {
    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Info);
    logger.debug("This should not appear");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("This should not appear") == std::string::npos);
}

TEST_F(LoggerTest, LogLevelNoneDisablesAllLogging) {
    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::None);
    logger.debug("Debug");
    logger.info("Info");
    logger.warning("Warning");
    logger.error("Error");
    logger.closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.empty());
}

TEST_F(LoggerTest, LogMessageIncludesTimestamp) {
    logger.setLogFile(testFilePath);
    logger.info("Test message");
    logger.closeFile();

    const std::string contents = readFileContents();
    std::regex pattern(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\])");
    EXPECT_TRUE(std::regex_search(contents, pattern));
}

TEST_F(LoggerTest, ThreadSafetyMultipleLogs) {
    logger.setLogFile(testFilePath);

    constexpr int numThreads = 10;
    constexpr int logsPerThread = 50;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < logsPerThread; ++j) {
                logger.info("Thread " + std::to_string(i) + " message " +
                            std::to_string(j));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    logger.closeFile();

    const std::string contents = readFileContents();
    int lineCount = 0;
    for (char c : contents) {
        if (c == '\n') {
            ++lineCount;
        }
    }
    EXPECT_EQ(lineCount, numThreads * logsPerThread);
}

// ============================================================================
// Logger Singleton Tests
// ============================================================================

TEST(LoggerSingletonTest, InstanceReturnsSameObject) {
    Logger& logger1 = Logger::instance();
    Logger& logger2 = Logger::instance();
    EXPECT_EQ(&logger1, &logger2);
}

TEST(LoggerSingletonTest, SetInstanceChangesInstance) {
    Logger customLogger;
    customLogger.setLogLevel(LogLevel::Error);

    Logger::setInstance(customLogger);

    EXPECT_EQ(&Logger::instance(), &customLogger);
    EXPECT_EQ(Logger::instance().getLogLevel(), LogLevel::Error);

    Logger::resetInstance();
}

TEST(LoggerSingletonTest, ResetInstanceRestoresDefault) {
    Logger customLogger;
    Logger::setInstance(customLogger);
    Logger::resetInstance();

    EXPECT_NE(&Logger::instance(), &customLogger);
}

// ============================================================================
// Logging Macros Tests
// ============================================================================

class MockLogger : public Logger {
   public:
    std::vector<std::pair<LogLevel, std::string>> loggedMessages;

    void info(const std::string& msg, LogCategory category = LogCategory::Main) override {
        loggedMessages.emplace_back(LogLevel::Info, msg);
        Logger::info(msg, category);
    }

    void warning(const std::string& msg, LogCategory category = LogCategory::Main) override {
        loggedMessages.emplace_back(LogLevel::Warning, msg);
        Logger::warning(msg, category);
    }

    void error(const std::string& msg, LogCategory category = LogCategory::Main) override {
        loggedMessages.emplace_back(LogLevel::Error, msg);
        Logger::error(msg, category);
    }

    void debug(const std::string& msg, LogCategory category = LogCategory::Main) override {
        loggedMessages.emplace_back(LogLevel::Debug, msg);
        Logger::debug(msg, category);
    }

    void fatal(const std::string& msg, LogCategory category = LogCategory::Main) override {
        loggedMessages.emplace_back(LogLevel::Fatal, msg);
        Logger::fatal(msg, category);
    }

    void clear() { loggedMessages.clear(); }
};

class LogMacrosTest : public ::testing::Test {
   protected:
    MockLogger mockLogger;

    void SetUp() override {
        mockLogger.setLogLevel(LogLevel::Debug);
        Logger::setInstance(mockLogger);
    }

    void TearDown() override { Logger::resetInstance(); }
};

TEST_F(LogMacrosTest, LogInfoMacro) {
    LOG_INFO("Test info");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].first, LogLevel::Info);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Test info");
}

TEST_F(LogMacrosTest, LogWarningMacro) {
    LOG_WARNING("Test warning");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].first, LogLevel::Warning);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Test warning");
}

TEST_F(LogMacrosTest, LogErrorMacro) {
    LOG_ERROR("Test error");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].first, LogLevel::Error);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Test error");
}

TEST_F(LogMacrosTest, LogMacroWithStreamOperator) {
    LOG_INFO("Value: " << 42 << " and " << "text");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Value: 42 and text");
}

TEST_F(LogMacrosTest, LogMacroWithComplexExpression) {
    int x = 10;
    int y = 20;
    LOG_INFO("Sum: " << (x + y));
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Sum: 30");
}

TEST_F(LogMacrosTest, LogToStringHandlesCommas) {
    // This tests the variadic macro handling of commas
    std::string result = LOG_TO_STRING("a" << ", " << "b");
    EXPECT_EQ(result, "a, b");
}

#ifndef NDEBUG
TEST_F(LogMacrosTest, LogDebugMacroInDebugBuild) {
    LOG_DEBUG("Test debug");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].first, LogLevel::Debug);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Test debug");
}
#endif

// ============================================================================
// Integration Tests
// ============================================================================

class LoggerIntegrationTest : public ::testing::Test {
   protected:
    std::filesystem::path testFilePath;

    void SetUp() override {
        testFilePath =
            std::filesystem::temp_directory_path() / "test_integration.log";
        std::filesystem::remove(testFilePath);
        Logger::resetInstance();
    }

    void TearDown() override {
        Logger::resetInstance();
        safeRemoveFile(testFilePath);
    }

    std::string readFileContents() {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

TEST_F(LoggerIntegrationTest, MacrosUseGlobalLogger) {
    Logger::instance().setLogFile(testFilePath);
    Logger::instance().setLogLevel(LogLevel::Debug);

    LOG_INFO("Integration test message");

    Logger::instance().closeFile();

    const std::string contents = readFileContents();
    EXPECT_TRUE(contents.find("Integration test message") != std::string::npos);
    EXPECT_TRUE(contents.find("[INFO]") != std::string::npos);
}

TEST_F(LoggerIntegrationTest, AllLogLevelsFormatCorrectly) {
    Logger::instance().setLogFile(testFilePath);
    Logger::instance().setLogLevel(LogLevel::Debug);

    LOG_INFO("Info message");
    LOG_WARNING("Warning message");
    LOG_ERROR("Error message");

    Logger::instance().closeFile();

    const std::string contents = readFileContents();

    std::regex infoPattern(
        R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[INFO\] Info message)");
    std::regex warningPattern(
        R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[WARNING\] Warning message)");
    std::regex errorPattern(
        R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] \[ERROR\] Error message)");

    EXPECT_TRUE(std::regex_search(contents, infoPattern));
    EXPECT_TRUE(std::regex_search(contents, warningPattern));
    EXPECT_TRUE(std::regex_search(contents, errorPattern));
}

// ============================================================================
// Additional Coverage Tests - Logger Edge Cases
// ============================================================================

TEST(LoggerEdgeCaseTest, DebugWritesToFile) {
    Logger logger;
    auto testFilePath = getUniqueTestFile("test_debug_writes");

    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Debug);
    logger.debug("Test debug message");
    logger.closeFile();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure file is flushed

    std::string contents;
    {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }  // Close file handle before removal

    EXPECT_TRUE(contents.find("[DEBUG]") != std::string::npos);
    EXPECT_TRUE(contents.find("Test debug message") != std::string::npos);
    safeRemoveFile(testFilePath);
}

TEST(LoggerEdgeCaseTest, LogLevelFilteringWarning) {
    Logger logger;
    auto testFilePath = getUniqueTestFile("test_warning_filter");

    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Error);
    logger.warning("This warning should not appear");
    logger.closeFile();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure file is flushed

    std::string contents;
    {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }  // Close file handle before removal

    EXPECT_TRUE(contents.find("This warning should not appear") == std::string::npos);

    safeRemoveFile(testFilePath);
}

TEST(LoggerEdgeCaseTest, ErrorGoesToStderr) {
    Logger logger;
    logger.setLogLevel(LogLevel::Debug);

    // Capture stderr
    testing::internal::CaptureStderr();
    logger.error("Error to stderr");
    std::string errOutput = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(errOutput.find("Error to stderr") != std::string::npos);
    EXPECT_TRUE(errOutput.find("[ERROR]") != std::string::npos);
}

TEST(LoggerEdgeCaseTest, WarningGoesToStderr) {
    Logger logger;
    logger.setLogLevel(LogLevel::Debug);

    // Capture stderr
    testing::internal::CaptureStderr();
    logger.warning("Warning to stderr");
    std::string errOutput = testing::internal::GetCapturedStderr();

    EXPECT_TRUE(errOutput.find("Warning to stderr") != std::string::npos);
    EXPECT_TRUE(errOutput.find("[WARNING]") != std::string::npos);
}

TEST(LoggerEdgeCaseTest, InfoGoesToStdout) {
    Logger logger;
    logger.setLogLevel(LogLevel::Debug);

    // Capture stdout
    testing::internal::CaptureStdout();
    logger.info("Info to stdout");
    std::string stdOutput = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(stdOutput.find("Info to stdout") != std::string::npos);
    EXPECT_TRUE(stdOutput.find("[INFO]") != std::string::npos);
}

TEST(LoggerEdgeCaseTest, SetLogFileWithAppendFalse) {
    auto testFilePath = getUniqueTestFile("test_no_append");

    // Create initial content
    {
        std::ofstream file(testFilePath);
        file << "Initial content\n";
    }

    Logger logger;
    logger.setLogFile(testFilePath, false);  // append = false
    logger.setLogLevel(LogLevel::Info);
    logger.info("New content");
    logger.closeFile();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure file is flushed

    std::string contents;
    {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }  // Close file handle before removal

    EXPECT_TRUE(contents.find("Initial content") == std::string::npos);
    EXPECT_TRUE(contents.find("New content") != std::string::npos);

    safeRemoveFile(testFilePath);
}

TEST(LoggerEdgeCaseTest, MultipleSetLogFileCalls) {
    auto firstFile = getUniqueTestFile("test_first");
    auto secondFile = getUniqueTestFile("test_second");

    Logger logger;
    logger.setLogLevel(LogLevel::Info);

    logger.setLogFile(firstFile);
    logger.info("First file message");

    logger.setLogFile(secondFile);
    logger.info("Second file message");
    logger.closeFile();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure files are flushed

    std::string contents1, contents2;
    {
        std::ifstream file1(firstFile);
        std::stringstream buffer1;
        buffer1 << file1.rdbuf();
        contents1 = buffer1.str();
    }  // Close file1 handle
    {
        std::ifstream file2(secondFile);
        std::stringstream buffer2;
        buffer2 << file2.rdbuf();
        contents2 = buffer2.str();
    }  // Close file2 handle

    EXPECT_TRUE(contents1.find("First file message") != std::string::npos);
    EXPECT_TRUE(contents2.find("Second file message") != std::string::npos);

    safeRemoveFile(firstFile);
    safeRemoveFile(secondFile);
}

// ============================================================================
// Additional Coverage Tests - FileWriter Edge Cases
// ============================================================================

TEST(FileWriterEdgeCaseTest, DestructorClosesFile) {
    auto testFilePath = getUniqueTestFile("test_destructor");

    {
        FileWriter writer;
        writer.open(testFilePath);
        writer.write("Test message");
        // Destructor should close the file
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure file is flushed

    // File should exist and contain the message
    std::string contents;
    {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }  // Close file handle before removal
    EXPECT_TRUE(contents.find("Test message") != std::string::npos);

    safeRemoveFile(testFilePath);
}

TEST(FileWriterEdgeCaseTest, DoubleCloseSafe) {
    auto testFilePath = std::filesystem::temp_directory_path() / "test_double_close.log";
    std::filesystem::remove(testFilePath);

    FileWriter writer;
    writer.open(testFilePath);
    writer.close();
    writer.close();  // Should not crash

    EXPECT_FALSE(writer.isOpen());

    safeRemoveFile(testFilePath);
}

TEST(FileWriterEdgeCaseTest, WriteAfterClose) {
    auto testFilePath = getUniqueTestFile("test_write_after_close");

    FileWriter writer;
    writer.open(testFilePath);
    writer.write("Before close");
    writer.close();
    writer.write("After close");  // Should do nothing
    std::this_thread::sleep_for(std::chrono::milliseconds(1));  // Ensure file is flushed

    std::string contents;
    {
        std::ifstream file(testFilePath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
    }  // Close file handle before removal

    EXPECT_TRUE(contents.find("Before close") != std::string::npos);
    EXPECT_TRUE(contents.find("After close") == std::string::npos);

    safeRemoveFile(testFilePath);
}

// ============================================================================
// Additional Coverage Tests - LogLevel Edge Cases
// ============================================================================

TEST(LogLevelEdgeCaseTest, ToStringUnknownLevel) {
    // Cast an invalid value to test default case
    LogLevel unknownLevel = static_cast<LogLevel>(999);
    std::string_view result = toString(unknownLevel);
    // Should return something reasonable (implementation-dependent)
    EXPECT_FALSE(result.empty());
}

TEST(LogLevelEdgeCaseTest, AllLevelComparisons) {
    EXPECT_TRUE(LogLevel::Debug < LogLevel::Info);
    EXPECT_TRUE(LogLevel::Info < LogLevel::Warning);
    EXPECT_TRUE(LogLevel::Warning < LogLevel::Error);
    EXPECT_TRUE(LogLevel::Error < LogLevel::None);

    EXPECT_FALSE(LogLevel::Info < LogLevel::Debug);
    EXPECT_FALSE(LogLevel::Warning < LogLevel::Info);
    EXPECT_FALSE(LogLevel::Error < LogLevel::Warning);
    EXPECT_FALSE(LogLevel::None < LogLevel::Error);
}

// ============================================================================
// Additional Coverage Tests - Macros
// ============================================================================

TEST_F(LogMacrosTest, LogDebugMacroWithComplexExpression) {
#ifndef NDEBUG
    int x = 5;
    int y = 10;
    LOG_DEBUG("Calculation: " << x << " + " << y << " = " << (x + y));
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Calculation: 5 + 10 = 15");
#endif
}

TEST_F(LogMacrosTest, MultipleMacrosInSequence) {
    LOG_INFO("First");
    LOG_WARNING("Second");
    LOG_ERROR("Third");

    ASSERT_EQ(mockLogger.loggedMessages.size(), 3);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "First");
    EXPECT_EQ(mockLogger.loggedMessages[1].second, "Second");
    EXPECT_EQ(mockLogger.loggedMessages[2].second, "Third");
}

TEST_F(LogMacrosTest, LogEmptyMessage) {
    LOG_INFO("");
    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "");
}

// ============================================================================
// LogLevel Fatal Tests
// ============================================================================

TEST(LogLevelTest, FatalLogLevelExists) {
    EXPECT_EQ(static_cast<int>(LogLevel::Fatal), 4);
    EXPECT_GT(LogLevel::Fatal, LogLevel::Error);
    EXPECT_LT(LogLevel::Fatal, LogLevel::None);
}

TEST(LogLevelTest, ToStringFatal) {
    EXPECT_EQ(toString(LogLevel::Fatal), "FATAL");
}

TEST(LoggerFatalTest, FatalWritesToFile) {
    const auto testFilePath = getUniqueTestFile("test_fatal");
    Logger logger;
    logger.setLogFile(testFilePath);
    logger.fatal("Fatal error occurred");
    logger.closeFile();

    std::ifstream file(testFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string contents = buffer.str();

    EXPECT_NE(contents.find("FATAL"), std::string::npos);
    EXPECT_NE(contents.find("Fatal error occurred"), std::string::npos);

    safeRemoveFile(testFilePath);
}

TEST(LoggerFatalTest, FatalGoesToStderr) {
    Logger logger;
    testing::internal::CaptureStderr();
    logger.fatal("Fatal test");
    const std::string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("FATAL"), std::string::npos);
}

TEST_F(LogMacrosTest, LogFatalMacro) {
    LOG_FATAL("Fatal error");

    ASSERT_EQ(mockLogger.loggedMessages.size(), 1);
    EXPECT_EQ(mockLogger.loggedMessages[0].first, LogLevel::Fatal);
    EXPECT_EQ(mockLogger.loggedMessages[0].second, "Fatal error");
}

// ============================================================================
// ColorFormatter Tests
// ============================================================================

#include "common/src/Logger/ColorFormatter.hpp"

TEST(ColorFormatterTest, DefaultEnabledState) {
#ifdef _WIN32
    // Colors disabled by default on Windows
    EXPECT_FALSE(ColorFormatter::isEnabled());
#else
    // Colors enabled by default on Unix/Linux
    EXPECT_TRUE(ColorFormatter::isEnabled());
#endif
}

TEST(ColorFormatterTest, SetEnabled) {
    bool originalState = ColorFormatter::isEnabled();
    
    ColorFormatter::setEnabled(true);
    EXPECT_TRUE(ColorFormatter::isEnabled());
    
    ColorFormatter::setEnabled(false);
    EXPECT_FALSE(ColorFormatter::isEnabled());
    
    // Restore original state
    ColorFormatter::setEnabled(originalState);
}

TEST(ColorFormatterTest, GetColorWhenEnabled) {
    ColorFormatter::setEnabled(true);
    
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Debug), "\033[36m");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Info), "\033[32m");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Warning), "\033[33m");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Error), "\033[31m");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Fatal), "\033[91m");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::None), "");
}

TEST(ColorFormatterTest, GetColorWhenDisabled) {
    ColorFormatter::setEnabled(false);
    
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Debug), "");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Info), "");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Warning), "");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Error), "");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::Fatal), "");
    EXPECT_EQ(ColorFormatter::getColor(LogLevel::None), "");
}

TEST(ColorFormatterTest, GetResetWhenEnabled) {
    ColorFormatter::setEnabled(true);
    EXPECT_EQ(ColorFormatter::getReset(), "\033[0m");
}

TEST(ColorFormatterTest, GetResetWhenDisabled) {
    ColorFormatter::setEnabled(false);
    EXPECT_EQ(ColorFormatter::getReset(), "");
}

TEST(LoggerColorTest, ColoredOutputToConsole) {
    Logger logger;
    ColorFormatter::setEnabled(true);
    
    testing::internal::CaptureStdout();
    logger.info("Test message");
    std::string output = testing::internal::GetCapturedStdout();
    
    // Should contain ANSI color codes
    EXPECT_NE(output.find("\033["), std::string::npos);
}

TEST(LoggerColorTest, NoColoredOutputWhenDisabled) {
    Logger logger;
    ColorFormatter::setEnabled(false);
    
    testing::internal::CaptureStdout();
    logger.info("Test message");
    std::string output = testing::internal::GetCapturedStdout();
    
    // Should not contain ANSI color codes
    EXPECT_EQ(output.find("\033["), std::string::npos);
}

TEST(LoggerColorTest, SetColorEnabledMethod) {
    Logger logger;
    
    logger.setColorEnabled(true);
    EXPECT_TRUE(logger.isColorEnabled());
    
    logger.setColorEnabled(false);
    EXPECT_FALSE(logger.isColorEnabled());
}

// ============================================================================
// GenerateLogFilename Tests
// ============================================================================

TEST(LoggerFilenameTest, GenerateLogFilenameWithDefaults) {
    const auto logFile = Logger::generateLogFilename();
    
    EXPECT_EQ(logFile.parent_path(), std::filesystem::path("logs"));
    EXPECT_NE(logFile.filename().string().find("session_"), std::string::npos);
    EXPECT_NE(logFile.filename().string().find(".log"), std::string::npos);
    
    // Clean up created directory if test created it
    if (std::filesystem::exists("logs") && std::filesystem::is_empty("logs")) {
        std::filesystem::remove("logs");
    }
}

TEST(LoggerFilenameTest, GenerateLogFilenameWithPrefix) {
    const auto logFile = Logger::generateLogFilename("server_session");
    
    EXPECT_NE(logFile.filename().string().find("server_session_"), 
              std::string::npos);
    EXPECT_NE(logFile.filename().string().find(".log"), std::string::npos);
    
    // Clean up created directory if test created it
    if (std::filesystem::exists("logs") && std::filesystem::is_empty("logs")) {
        std::filesystem::remove("logs");
    }
}

TEST(LoggerFilenameTest, GenerateLogFilenameWithCustomDirectory) {
    const auto testDir = std::filesystem::temp_directory_path() / "test_logs";
    const auto logFile = Logger::generateLogFilename("client_session", testDir);
    
    EXPECT_EQ(logFile.parent_path(), testDir);
    EXPECT_NE(logFile.filename().string().find("client_session_"), 
              std::string::npos);
    
    // Clean up
    if (std::filesystem::exists(testDir)) {
        std::filesystem::remove_all(testDir);
    }
}

TEST(LoggerFilenameTest, GenerateLogFilenameCreatesDirectory) {
    const auto testDir = std::filesystem::temp_directory_path() / 
                         "test_logs_create";
    
    // Ensure directory doesn't exist
    if (std::filesystem::exists(testDir)) {
        std::filesystem::remove_all(testDir);
    }
    
    EXPECT_FALSE(std::filesystem::exists(testDir));
    
    const auto logFile = Logger::generateLogFilename("test", testDir);
    
    EXPECT_TRUE(std::filesystem::exists(testDir));
    EXPECT_TRUE(std::filesystem::is_directory(testDir));
    
    // Clean up
    std::filesystem::remove_all(testDir);
}

TEST(LoggerFilenameTest, GenerateLogFilenameTimestampFormat) {
    const auto logFile = Logger::generateLogFilename("test");
    const auto filename = logFile.filename().string();
    
    // Should match format: test_YYYY-MM-DD_HH-MM-SS.log
    std::regex pattern(R"(test_\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.log)");
    EXPECT_TRUE(std::regex_match(filename, pattern));
    
    // Clean up created directory if test created it
    if (std::filesystem::exists("logs") && std::filesystem::is_empty("logs")) {
        std::filesystem::remove("logs");
    }
}

TEST(LoggerFilenameTest, GenerateLogFilenameUnique) {
    const auto logFile1 = Logger::generateLogFilename("test");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const auto logFile2 = Logger::generateLogFilename("test");
    
    // Files should have different names due to timestamp
    EXPECT_NE(logFile1, logFile2);
    
    // Clean up created directory if test created it
    if (std::filesystem::exists("logs") && std::filesystem::is_empty("logs")) {
        std::filesystem::remove("logs");
    }
}

// ============================================================================
// LogCategory Tests
// ============================================================================

#include "common/src/Logger/LogCategory.hpp"

TEST(LogCategoryTest, EnumValues) {
    EXPECT_EQ(static_cast<uint32_t>(LogCategory::None), 0u);
    EXPECT_EQ(static_cast<uint32_t>(LogCategory::Main), 1u << 0);
    EXPECT_EQ(static_cast<uint32_t>(LogCategory::Network), 1u << 1);
    EXPECT_EQ(static_cast<uint32_t>(LogCategory::GameEngine), 1u << 2);
    EXPECT_EQ(static_cast<uint32_t>(LogCategory::All), 0xFFFFFFFFu);
}

TEST(LogCategoryTest, BitwiseOperators) {
    LogCategory cat1 = LogCategory::Main;
    LogCategory cat2 = LogCategory::Network;
    
    LogCategory combined = cat1 | cat2;
    EXPECT_TRUE(isCategoryEnabled(combined, LogCategory::Main));
    EXPECT_TRUE(isCategoryEnabled(combined, LogCategory::Network));
    EXPECT_FALSE(isCategoryEnabled(combined, LogCategory::GameEngine));
}

TEST(LogCategoryTest, IsCategoryEnabled) {
    LogCategory mask = LogCategory::Main | LogCategory::Network;
    
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Main));
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Network));
    EXPECT_FALSE(isCategoryEnabled(mask, LogCategory::GameEngine));
    EXPECT_FALSE(isCategoryEnabled(mask, LogCategory::ECS));
}

TEST(LogCategoryTest, AllCategoryEnablesEverything) {
    LogCategory mask = LogCategory::All;
    
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Main));
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::Network));
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::GameEngine));
    EXPECT_TRUE(isCategoryEnabled(mask, LogCategory::ECS));
}

TEST(LogCategoryTest, ToStringConversion) {
    EXPECT_EQ(toString(LogCategory::Main), "Main");
    EXPECT_EQ(toString(LogCategory::Network), "Network");
    EXPECT_EQ(toString(LogCategory::GameEngine), "GameEngine");
    EXPECT_EQ(toString(LogCategory::All), "All");
}

TEST(LogCategoryTest, FromStringConversion) {
    EXPECT_EQ(categoryFromString("main"), LogCategory::Main);
    EXPECT_EQ(categoryFromString("Main"), LogCategory::Main);
    EXPECT_EQ(categoryFromString("MAIN"), LogCategory::Main);
    
    EXPECT_EQ(categoryFromString("network"), LogCategory::Network);
    EXPECT_EQ(categoryFromString("Network"), LogCategory::Network);
    
    EXPECT_EQ(categoryFromString("gameengine"), LogCategory::GameEngine);
    EXPECT_EQ(categoryFromString("game"), LogCategory::GameEngine);
    
    EXPECT_EQ(categoryFromString("all"), LogCategory::All);
    EXPECT_EQ(categoryFromString("ALL"), LogCategory::All);
    
    EXPECT_EQ(categoryFromString("invalid"), LogCategory::None);
}

TEST(LoggerCategoryTest, SetEnabledCategories) {
    Logger logger;
    
    logger.setEnabledCategories(LogCategory::Network);
    EXPECT_TRUE(logger.isCategoryEnabled(LogCategory::Network));
    EXPECT_FALSE(logger.isCategoryEnabled(LogCategory::Main));
}

TEST(LoggerCategoryTest, EnableCategory) {
    Logger logger;
    logger.setEnabledCategories(LogCategory::None);
    
    logger.enableCategory(LogCategory::Main);
    EXPECT_TRUE(logger.isCategoryEnabled(LogCategory::Main));
    EXPECT_FALSE(logger.isCategoryEnabled(LogCategory::Network));
    
    logger.enableCategory(LogCategory::Network);
    EXPECT_TRUE(logger.isCategoryEnabled(LogCategory::Main));
    EXPECT_TRUE(logger.isCategoryEnabled(LogCategory::Network));
}

TEST(LoggerCategoryTest, CategoryFiltering) {
    const auto testFilePath = getUniqueTestFile("test_category");
    Logger logger;
    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Debug);
    
    // Only enable Network category
    logger.setEnabledCategories(LogCategory::Network);
    
    logger.debug("Main message", LogCategory::Main);
    logger.debug("Network message", LogCategory::Network);
    logger.closeFile();
    
    std::ifstream file(testFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string contents = buffer.str();
    
    // Only Network message should be logged
    EXPECT_EQ(contents.find("Main message"), std::string::npos);
    EXPECT_NE(contents.find("Network message"), std::string::npos);
    
    safeRemoveFile(testFilePath);
}

TEST(LoggerCategoryTest, MacroWithCategory) {
    const auto testFilePath = getUniqueTestFile("test_category_macro");
    Logger logger;
    Logger::setInstance(logger);
    
    logger.setLogFile(testFilePath);
    logger.setLogLevel(LogLevel::Debug);
    logger.setEnabledCategories(LogCategory::GameEngine);
    
    LOG_DEBUG_CAT(LogCategory::Main, "Main debug");
    LOG_DEBUG_CAT(LogCategory::GameEngine, "GameEngine debug");
    LOG_INFO_CAT(LogCategory::Network, "Network info");
    
    logger.closeFile();
    
    std::ifstream file(testFilePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string contents = buffer.str();
    
    // Only GameEngine message should be logged
    EXPECT_EQ(contents.find("Main debug"), std::string::npos);
    EXPECT_NE(contents.find("GameEngine debug"), std::string::npos);
    EXPECT_EQ(contents.find("Network info"), std::string::npos);
    
    Logger::resetInstance();
    safeRemoveFile(testFilePath);
}
