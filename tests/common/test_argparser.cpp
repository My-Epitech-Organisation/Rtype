/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for ArgParser
*/

#include "common/ArgParser.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <string_view>

using namespace rtype;

// ============================================================================
// ParseResult Tests
// ============================================================================

TEST(ParseResultTest, EnumValues) {
    EXPECT_NE(ParseResult::Success, ParseResult::Exit);
    EXPECT_NE(ParseResult::Success, ParseResult::Error);
    EXPECT_NE(ParseResult::Exit, ParseResult::Error);
}

// ============================================================================
// ArgParser Flag Tests
// ============================================================================

class ArgParserFlagTest : public ::testing::Test {
 protected:
    ArgParser parser;
    bool flagCalled = false;

    void SetUp() override {
        flagCalled = false;
    }
};

TEST_F(ArgParserFlagTest, ShortFlagIsCalled) {
    parser.flag("-h", "--help", "Show help", [this]() {
        flagCalled = true;
        return ParseResult::Exit;
    });

    std::vector<std::string_view> args = {"-h"};
    ParseResult result = parser.parse(args);

    EXPECT_TRUE(flagCalled);
    EXPECT_EQ(result, ParseResult::Exit);
}

TEST_F(ArgParserFlagTest, LongFlagIsCalled) {
    parser.flag("-h", "--help", "Show help", [this]() {
        flagCalled = true;
        return ParseResult::Exit;
    });

    std::vector<std::string_view> args = {"--help"};
    ParseResult result = parser.parse(args);

    EXPECT_TRUE(flagCalled);
    EXPECT_EQ(result, ParseResult::Exit);
}

TEST_F(ArgParserFlagTest, FlagReturnsSuccess) {
    parser.flag("-v", "--verbose", "Enable verbose mode", [this]() {
        flagCalled = true;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-v"};
    ParseResult result = parser.parse(args);

    EXPECT_TRUE(flagCalled);
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserFlagTest, UnknownFlagReturnsError) {
    std::vector<std::string_view> args = {"--unknown"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
}

TEST_F(ArgParserFlagTest, MultipleFlagsAreParsed) {
    bool flag1Called = false;
    bool flag2Called = false;

    parser.flag("-a", "--alpha", "First flag", [&flag1Called]() {
        flag1Called = true;
        return ParseResult::Success;
    });
    parser.flag("-b", "--beta", "Second flag", [&flag2Called]() {
        flag2Called = true;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-a", "--beta"};
    ParseResult result = parser.parse(args);

    EXPECT_TRUE(flag1Called);
    EXPECT_TRUE(flag2Called);
    EXPECT_EQ(result, ParseResult::Success);
}

// ============================================================================
// ArgParser Option Tests
// ============================================================================

class ArgParserOptionTest : public ::testing::Test {
 protected:
    ArgParser parser;
    std::string capturedValue;

    void SetUp() override {
        capturedValue.clear();
    }
};

TEST_F(ArgParserOptionTest, ShortOptionWithValue) {
    parser.option("-p", "--port", "port", "Server port", [this](std::string_view value) {
        capturedValue = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-p", "4242"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(capturedValue, "4242");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserOptionTest, LongOptionWithValue) {
    parser.option("-p", "--port", "port", "Server port", [this](std::string_view value) {
        capturedValue = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"--port", "8080"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(capturedValue, "8080");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserOptionTest, OptionWithoutValueReturnsError) {
    parser.option("-p", "--port", "port", "Server port", [this](std::string_view value) {
        capturedValue = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-p"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
    EXPECT_TRUE(capturedValue.empty());
}

TEST_F(ArgParserOptionTest, OptionHandlerReturnsError) {
    parser.option("-p", "--port", "port", "Server port", [](std::string_view) {
        return ParseResult::Error;
    });

    std::vector<std::string_view> args = {"-p", "invalid"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
}

TEST_F(ArgParserOptionTest, MultipleOptionsAreParsed) {
    std::string host;
    std::string port;

    parser.option("-h", "--host", "host", "Server host", [&host](std::string_view value) {
        host = std::string(value);
        return ParseResult::Success;
    });
    parser.option("-p", "--port", "port", "Server port", [&port](std::string_view value) {
        port = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"--host", "localhost", "-p", "4242"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(host, "localhost");
    EXPECT_EQ(port, "4242");
    EXPECT_EQ(result, ParseResult::Success);
}

// ============================================================================
// ArgParser Positional Tests
// ============================================================================

class ArgParserPositionalTest : public ::testing::Test {
 protected:
    ArgParser parser;
};

TEST_F(ArgParserPositionalTest, RequiredPositionalArgument) {
    std::string configPath;

    parser.positional("config", "Configuration file", [&configPath](std::string_view value) {
        configPath = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"config.toml"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(configPath, "config.toml");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserPositionalTest, MissingRequiredPositionalReturnsError) {
    parser.positional("config", "Configuration file", [](std::string_view) {
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
}

TEST_F(ArgParserPositionalTest, OptionalPositionalArgument) {
    std::string configPath = "default.toml";

    parser.positional("config", "Configuration file", [&configPath](std::string_view value) {
        configPath = std::string(value);
        return ParseResult::Success;
    }, false);  // optional

    std::vector<std::string_view> args = {};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(configPath, "default.toml");  // unchanged
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserPositionalTest, MultiplePositionalArguments) {
    std::string input;
    std::string output;

    parser.positional("input", "Input file", [&input](std::string_view value) {
        input = std::string(value);
        return ParseResult::Success;
    });
    parser.positional("output", "Output file", [&output](std::string_view value) {
        output = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"input.txt", "output.txt"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(input, "input.txt");
    EXPECT_EQ(output, "output.txt");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserPositionalTest, PositionalHandlerReturnsError) {
    parser.positional("config", "Configuration file", [](std::string_view) {
        return ParseResult::Error;
    });

    std::vector<std::string_view> args = {"invalid.toml"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
}

// ============================================================================
// ArgParser Mixed Tests
// ============================================================================

class ArgParserMixedTest : public ::testing::Test {
 protected:
    ArgParser parser;
};

TEST_F(ArgParserMixedTest, FlagsOptionsAndPositionals) {
    bool verbose = false;
    std::string port;
    std::string config;

    parser.flag("-v", "--verbose", "Enable verbose mode", [&verbose]() {
        verbose = true;
        return ParseResult::Success;
    });
    parser.option("-p", "--port", "port", "Server port", [&port](std::string_view value) {
        port = std::string(value);
        return ParseResult::Success;
    });
    parser.positional("config", "Configuration file", [&config](std::string_view value) {
        config = std::string(value);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-v", "--port", "4242", "server.toml"};
    ParseResult result = parser.parse(args);

    EXPECT_TRUE(verbose);
    EXPECT_EQ(port, "4242");
    EXPECT_EQ(config, "server.toml");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserMixedTest, OptionsAfterPositional) {
    std::string port;
    std::string config;

    parser.option("-p", "--port", "port", "Server port", [&port](std::string_view value) {
        port = std::string(value);
        return ParseResult::Success;
    });
    parser.positional("config", "Configuration file", [&config](std::string_view value) {
        config = std::string(value);
        return ParseResult::Success;
    });

    // Positional first, then option
    std::vector<std::string_view> args = {"server.toml", "-p", "4242"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(port, "4242");
    EXPECT_EQ(config, "server.toml");
    EXPECT_EQ(result, ParseResult::Success);
}

TEST_F(ArgParserMixedTest, EmptyArgsWithNoRequiredPositionals) {
    bool flagCalled = false;

    parser.flag("-h", "--help", "Show help", [&flagCalled]() {
        flagCalled = true;
        return ParseResult::Exit;
    });

    std::vector<std::string_view> args = {};
    ParseResult result = parser.parse(args);

    EXPECT_FALSE(flagCalled);
    EXPECT_EQ(result, ParseResult::Success);
}

// ============================================================================
// ArgParser Configuration Tests
// ============================================================================

TEST(ArgParserConfigTest, FluentAPIChaining) {
    ArgParser parser;
    bool flag1 = false;
    bool flag2 = false;

    // Test that all methods return *this for chaining
    parser.programName("test")
          .flag("-a", "--alpha", "First", [&flag1]() {
              flag1 = true;
              return ParseResult::Success;
          })
          .flag("-b", "--beta", "Second", [&flag2]() {
              flag2 = true;
              return ParseResult::Success;
          });

    std::vector<std::string_view> args = {"-a", "-b"};
    ParseResult result = parser.parse(args);
    EXPECT_EQ(result, ParseResult::Success);

    EXPECT_TRUE(flag1);
    EXPECT_TRUE(flag2);
}

TEST(ArgParserConfigTest, ProgramNameIsSet) {
    ArgParser parser;
    parser.programName("my-program");

    // We can't easily test the output, but we can verify it doesn't crash
    testing::internal::CaptureStdout();
    parser.printUsage();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("my-program"), std::string::npos);
}

// ============================================================================
// NumberParser Tests
// ============================================================================

class NumberParserTest : public ::testing::Test {
 protected:
    // Suppress logger output during tests
    void SetUp() override {
        testing::internal::CaptureStderr();
    }

    void TearDown() override {
        testing::internal::GetCapturedStderr();
    }
};

TEST_F(NumberParserTest, ParseValidInteger) {
    auto result = parseNumber<int>("42", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST_F(NumberParserTest, ParseValidNegativeInteger) {
    auto result = parseNumber<int>("-42", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, -42);
}

TEST_F(NumberParserTest, ParseValidUint16) {
    auto result = parseNumber<uint16_t>("4242", "port");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 4242);
}

TEST_F(NumberParserTest, ParseWithinRange) {
    auto result = parseNumber<uint16_t>("1024", "port", uint16_t{1024}, uint16_t{65535});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 1024);
}

TEST_F(NumberParserTest, ParseBelowMinimumReturnsNullopt) {
    auto result = parseNumber<uint16_t>("100", "port", uint16_t{1024}, uint16_t{65535});
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseAboveMaximumReturnsNullopt) {
    auto result = parseNumber<uint16_t>("70000", "port", uint16_t{1024}, uint16_t{65535});
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseInvalidStringReturnsNullopt) {
    auto result = parseNumber<int>("not_a_number", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParsePartialNumberReturnsNullopt) {
    auto result = parseNumber<int>("42abc", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseEmptyStringReturnsNullopt) {
    auto result = parseNumber<int>("", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseZero) {
    auto result = parseNumber<int>("0", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0);
}

TEST_F(NumberParserTest, ParseMaxValue) {
    auto result = parseNumber<uint8_t>("255", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 255);
}

TEST_F(NumberParserTest, ParseOverflowReturnsNullopt) {
    // 256 is too large for uint8_t
    auto result = parseNumber<uint8_t>("256", "value");
    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(ArgParserEdgeCaseTest, ParseEmptyArgs) {
    ArgParser parser;
    std::vector<std::string_view> args = {};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
}

TEST(ArgParserEdgeCaseTest, FlagStopsParsingOnExit) {
    ArgParser parser;
    bool secondFlagCalled = false;

    parser.flag("-a", "--alpha", "First", []() {
        return ParseResult::Exit;
    });
    parser.flag("-b", "--beta", "Second", [&secondFlagCalled]() {
        secondFlagCalled = true;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-a", "-b"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Exit);
    EXPECT_FALSE(secondFlagCalled);
}

TEST(ArgParserEdgeCaseTest, FlagStopsParsingOnError) {
    ArgParser parser;
    bool secondFlagCalled = false;

    parser.flag("-a", "--alpha", "First", []() {
        return ParseResult::Error;
    });
    parser.flag("-b", "--beta", "Second", [&secondFlagCalled]() {
        secondFlagCalled = true;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-a", "-b"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Error);
    EXPECT_FALSE(secondFlagCalled);
}

// ============================================================================
// Additional Coverage Tests - Duplicate Options
// ============================================================================

TEST(ArgParserDuplicateTest, DuplicateFlagIsIgnored) {
    ArgParser parser;
    int callCount = 0;

    parser.flag("-a", "--alpha", "First flag", [&callCount]() {
        callCount++;
        return ParseResult::Success;
    });
    // This duplicate should be ignored with a warning
    parser.flag("-a", "--alpha", "Duplicate flag", [&callCount]() {
        callCount += 10;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-a"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
    EXPECT_EQ(callCount, 1);  // Only first handler called
}

TEST(ArgParserDuplicateTest, DuplicateOptionIsIgnored) {
    ArgParser parser;
    std::string capturedValue;

    parser.option("-p", "--port", "port", "First option", [&capturedValue](std::string_view val) {
        capturedValue = "first:" + std::string(val);
        return ParseResult::Success;
    });
    // This duplicate should be ignored
    parser.option("-p", "--port", "port", "Duplicate option", [&capturedValue](std::string_view val) {
        capturedValue = "second:" + std::string(val);
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-p", "4242"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
    EXPECT_EQ(capturedValue, "first:4242");
}

TEST(ArgParserDuplicateTest, DuplicateShortOptionConflictsWithLong) {
    ArgParser parser;
    int callCount = 0;

    parser.flag("-a", "--alpha", "First flag", [&callCount]() {
        callCount++;
        return ParseResult::Success;
    });
    // Different short but same long option - should be ignored
    parser.flag("-b", "--alpha", "Same long option", [&callCount]() {
        callCount += 10;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"--alpha"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
    EXPECT_EQ(callCount, 1);
}

TEST(ArgParserDuplicateTest, DuplicateLongOptionConflictsWithShort) {
    ArgParser parser;
    int callCount = 0;

    parser.flag("-a", "--alpha", "First flag", [&callCount]() {
        callCount++;
        return ParseResult::Success;
    });
    // Same short but different long option - should be ignored
    parser.flag("-a", "--beta", "Same short option", [&callCount]() {
        callCount += 10;
        return ParseResult::Success;
    });

    std::vector<std::string_view> args = {"-a"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
    EXPECT_EQ(callCount, 1);
}

// ============================================================================
// Additional Coverage Tests - Extra Positional Args
// ============================================================================

TEST(ArgParserExtraArgsTest, ExtraPositionalArgsAreIgnored) {
    ArgParser parser;
    std::string capturedInput;

    parser.positional("input", "Input file", [&capturedInput](std::string_view value) {
        capturedInput = std::string(value);
        return ParseResult::Success;
    });

    // More args than expected
    std::vector<std::string_view> args = {"input.txt", "extra1.txt", "extra2.txt"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Success);
    EXPECT_EQ(capturedInput, "input.txt");
}

TEST(ArgParserExtraArgsTest, NoExtraArgsWarningWithNoPositionals) {
    ArgParser parser;

    parser.flag("-v", "--verbose", "Verbose mode", []() {
        return ParseResult::Success;
    });

    // Extra args but no positional defined
    std::vector<std::string_view> args = {"extra_arg"};
    ParseResult result = parser.parse(args);

    // Should succeed since no positional is required
    EXPECT_EQ(result, ParseResult::Success);
}

// ============================================================================
// Additional Coverage Tests - PrintUsage
// ============================================================================

TEST(ArgParserPrintUsageTest, PrintUsageWithOptionsAndPositionals) {
    ArgParser parser;
    parser.programName("test-program");

    parser.flag("-h", "--help", "Show help", []() {
        return ParseResult::Exit;
    });
    parser.option("-p", "--port", "port", "Server port", [](std::string_view) {
        return ParseResult::Success;
    });
    parser.positional("config", "Configuration file", [](std::string_view) {
        return ParseResult::Success;
    });
    parser.positional("output", "Output file", [](std::string_view) {
        return ParseResult::Success;
    }, false);  // optional

    testing::internal::CaptureStdout();
    parser.printUsage();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("test-program"), std::string::npos);
    EXPECT_NE(output.find("-h"), std::string::npos);
    EXPECT_NE(output.find("--help"), std::string::npos);
    EXPECT_NE(output.find("-p"), std::string::npos);
    EXPECT_NE(output.find("--port"), std::string::npos);
    EXPECT_NE(output.find("<port>"), std::string::npos);
    EXPECT_NE(output.find("config"), std::string::npos);
    EXPECT_NE(output.find("output"), std::string::npos);
    EXPECT_NE(output.find("(optional)"), std::string::npos);
}

TEST(ArgParserPrintUsageTest, PrintUsageWithNoOptions) {
    ArgParser parser;
    parser.programName("minimal");

    testing::internal::CaptureStdout();
    parser.printUsage();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("minimal"), std::string::npos);
    EXPECT_EQ(output.find("Options:"), std::string::npos);  // No options section
}

TEST(ArgParserPrintUsageTest, PrintUsageWithNoPositionals) {
    ArgParser parser;
    parser.programName("flags-only");
    parser.flag("-v", "--version", "Show version", []() {
        return ParseResult::Exit;
    });

    testing::internal::CaptureStdout();
    parser.printUsage();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Options:"), std::string::npos);
    EXPECT_EQ(output.find("Arguments:"), std::string::npos);  // No arguments section
}

// ============================================================================
// Additional Coverage Tests - NumberParser Edge Cases
// ============================================================================

TEST_F(NumberParserTest, ParseSignedOverflow) {
    // Int8 max is 127
    auto result = parseNumber<int8_t>("128", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseSignedUnderflow) {
    // Int8 min is -128
    auto result = parseNumber<int8_t>("-129", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseNegativeAsUnsignedFails) {
    auto result = parseNumber<uint32_t>("-1", "value");
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseLeadingWhitespace) {
    auto result = parseNumber<int>("  42", "value");
    // Depends on implementation - might accept leading whitespace
    if (result.has_value()) {
        EXPECT_EQ(*result, 42);
    }
}

TEST_F(NumberParserTest, ParseTrailingWhitespace) {
    auto result = parseNumber<int>("42  ", "value");
    // Should fail because of trailing characters
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseFloat) {
    auto result = parseNumber<int>("42.5", "value");
    EXPECT_FALSE(result.has_value());  // Not a valid integer
}

TEST_F(NumberParserTest, ParseHexadecimal) {
    auto result = parseNumber<int>("0x10", "value");
    // stoull/stoll with base 10 should fail on hex
    EXPECT_FALSE(result.has_value());
}

TEST_F(NumberParserTest, ParseExactMinRange) {
    auto result = parseNumber<int>("10", "value", 10, 100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 10);
}

TEST_F(NumberParserTest, ParseExactMaxRange) {
    auto result = parseNumber<int>("100", "value", 10, 100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 100);
}

TEST_F(NumberParserTest, ParseInt32MaxValue) {
    auto result = parseNumber<int32_t>("2147483647", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, INT32_MAX);
}

TEST_F(NumberParserTest, ParseUint32MaxValue) {
    auto result = parseNumber<uint32_t>("4294967295", "value");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, UINT32_MAX);
}

TEST_F(NumberParserTest, ParseOutOfRangeHugeNumber) {
    // A number too large even for uint64_t
    auto result = parseNumber<uint64_t>("99999999999999999999999", "value");
    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Additional Coverage Tests - Option Handler Returns Exit
// ============================================================================

TEST(ArgParserOptionExitTest, OptionHandlerReturnsExit) {
    ArgParser parser;

    parser.option("-c", "--config", "file", "Config file", [](std::string_view) {
        return ParseResult::Exit;
    });

    std::vector<std::string_view> args = {"-c", "config.toml"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Exit);
}

TEST(ArgParserOptionExitTest, PositionalHandlerReturnsExit) {
    ArgParser parser;

    parser.positional("file", "Input file", [](std::string_view) {
        return ParseResult::Exit;
    });

    std::vector<std::string_view> args = {"input.txt"};
    ParseResult result = parser.parse(args);

    EXPECT_EQ(result, ParseResult::Exit);
}
