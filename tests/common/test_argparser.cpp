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
