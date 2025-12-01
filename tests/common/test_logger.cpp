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
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "common/Logger/FileWriter.hpp"
#include "common/Logger/LogLevel.hpp"
#include "common/Logger/Logger.hpp"
#include "common/Logger/Macros.hpp"
#include "common/Logger/Timestamp.hpp"

using namespace rtype;

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

    void TearDown() override { std::filesystem::remove(testFilePath); }

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
    std::filesystem::remove(secondPath);

    FileWriter writer;
    writer.open(testFilePath);
    writer.write("First file");
    writer.open(secondPath);
    writer.write("Second file");
    writer.close();

    std::filesystem::remove(secondPath);
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
        std::filesystem::remove(testFilePath);
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

    void info(const std::string& msg) override {
        loggedMessages.emplace_back(LogLevel::Info, msg);
        Logger::info(msg);
    }

    void warning(const std::string& msg) override {
        loggedMessages.emplace_back(LogLevel::Warning, msg);
        Logger::warning(msg);
    }

    void error(const std::string& msg) override {
        loggedMessages.emplace_back(LogLevel::Error, msg);
        Logger::error(msg);
    }

    void debug(const std::string& msg) override {
        loggedMessages.emplace_back(LogLevel::Debug, msg);
        Logger::debug(msg);
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
        std::filesystem::remove(testFilePath);
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
