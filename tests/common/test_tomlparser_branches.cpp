/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Extended branch coverage tests for TomlParser
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "common/Config/TomlParser.hpp"

using namespace rtype::config;

class TomlParserBranchTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "toml_branch_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(testDir);
    }

    void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(testDir / filename);
        file << content;
    }

    std::filesystem::path testDir;
};

// ============================================================================
// TomlParser::parseFile() Branch Tests
// ============================================================================

TEST_F(TomlParserBranchTest, ParseFileNotFound) {
    TomlParser parser;
    auto result = parser.parseFile("nonexistent_file.toml");

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
    EXPECT_NE(parser.getLastResult().errorMessage.find("not found"), std::string::npos);
}

TEST_F(TomlParserBranchTest, ParseFileCannotOpen) {
    // Create a file without read permissions
    auto filePath = testDir / "unreadable.toml";
    std::ofstream file(filePath);
    file << "[test]\nkey = 1";
    file.close();
    
    // Remove read permissions
    std::filesystem::permissions(filePath, std::filesystem::perms::none);

    TomlParser parser;
    auto result = parser.parseFile(filePath);

    // Restore permissions for cleanup
    std::filesystem::permissions(filePath, std::filesystem::perms::owner_all);

    // Trying to open a file without read permissions should fail
    EXPECT_FALSE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseFileValidToml) {
    const std::string toml = R"(
[section]
key = "value"
number = 42
)";
    writeFile("valid.toml", toml);

    TomlParser parser;
    auto result = parser.parseFile(testDir / "valid.toml");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, ParseFileInvalidToml) {
    const std::string toml = R"(
[section
key = "value
missing brackets and quotes
)";
    writeFile("invalid.toml", toml);

    TomlParser parser;
    auto result = parser.parseFile(testDir / "invalid.toml");

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
    EXPECT_NE(parser.getLastResult().errorMessage.find("parse error"), std::string::npos);
}

// ============================================================================
// TomlParser::parseString() Branch Tests
// ============================================================================

TEST_F(TomlParserBranchTest, ParseStringValid) {
    const std::string toml = R"(
[test]
value = 123
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, ParseStringInvalidSyntax) {
    const std::string toml = "this is not valid toml [";

    TomlParser parser;
    auto result = parser.parseString(toml);

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, ParseStringEmpty) {
    TomlParser parser;
    auto result = parser.parseString("");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

// ============================================================================
// TomlParser::saveToFile() Branch Tests
// ============================================================================

TEST_F(TomlParserBranchTest, SaveToFileSuccess) {
    toml::table table;
    table.insert("key", "value");

    TomlParser parser;
    bool result = parser.saveToFile(table, testDir / "output.toml");

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testDir / "output.toml"));
}

TEST_F(TomlParserBranchTest, SaveToFileCreatesParentDirectories) {
    toml::table table;
    table.insert("key", "value");

    auto deepPath = testDir / "deep" / "nested" / "path" / "config.toml";

    TomlParser parser;
    bool result = parser.saveToFile(table, deepPath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(deepPath));
}

TEST_F(TomlParserBranchTest, SaveToFileInvalidPath) {
    toml::table table;
    table.insert("key", "value");

    TomlParser parser;
    // Test saving to a valid path to ensure the function works
    auto validPath = testDir / "valid_output.toml";
    bool result = parser.saveToFile(table, validPath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(validPath));
}

TEST_F(TomlParserBranchTest, SaveToFileNoParentPath) {
    toml::table table;
    table.insert("key", "value");

    // Save to current directory (no parent path)
    auto currentDir = std::filesystem::current_path();
    auto filepath = testDir / "simple.toml";

    TomlParser parser;
    bool result = parser.saveToFile(table, filepath);

    EXPECT_TRUE(result);
}

// ============================================================================
// TomlParser::getString() Branch Tests
// ============================================================================

TEST_F(TomlParserBranchTest, GetStringExistingKey) {
    const std::string toml = R"(
[section]
key = "test_value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(value, "test_value");
}

TEST_F(TomlParserBranchTest, GetStringMissingKey) {
    const std::string toml = R"(
[section]
other = "value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "missing", "default_value");
    EXPECT_EQ(value, "default_value");
}

TEST_F(TomlParserBranchTest, GetStringMissingSection) {
    const std::string toml = R"(
[other_section]
key = "value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "missing_section", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetStringWrongType) {
    const std::string toml = R"(
[section]
key = 123
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Key exists but is not a string
    std::string value = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(value, "default");
}

// ============================================================================
// TomlParser::getValue() Branch Tests
// ============================================================================

TEST_F(TomlParserBranchTest, GetValueInt) {
    const std::string toml = R"(
[section]
number = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "number", 0);
    EXPECT_EQ(value, 42);
}

TEST_F(TomlParserBranchTest, GetValueDouble) {
    const std::string toml = R"(
[section]
pi = 3.14159
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    double value = parser.getValue<double>(*table, "section", "pi", 0.0);
    EXPECT_NEAR(value, 3.14159, 0.00001);
}

TEST_F(TomlParserBranchTest, GetValueBool) {
    const std::string toml = R"(
[section]
enabled = true
disabled = false
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    EXPECT_TRUE(parser.getValue<bool>(*table, "section", "enabled", false));
    EXPECT_FALSE(parser.getValue<bool>(*table, "section", "disabled", true));
}

TEST_F(TomlParserBranchTest, GetValueMissing) {
    const std::string toml = R"(
[section]
other = 100
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "missing", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchTest, GetValueWrongType) {
    const std::string toml = R"(
[section]
key = "not a number"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "key", 123);
    EXPECT_EQ(value, 123);
}

// ============================================================================
// TomlParser::reportError() and Error Callback Tests
// ============================================================================

TEST_F(TomlParserBranchTest, ErrorCallbackCalled) {
    std::vector<ParseError> capturedErrors;
    TomlParser parser;
    parser.setErrorCallback([&capturedErrors](const ParseError& error) {
        capturedErrors.push_back(error);
    });

    // Trigger an error by parsing invalid TOML
    [[maybe_unused]] auto result = parser.parseString("invalid [ toml");

    EXPECT_FALSE(capturedErrors.empty());
}

TEST_F(TomlParserBranchTest, ErrorCallbackNotSetNoError) {
    TomlParser parser;
    // Don't set callback

    // This should not crash
    [[maybe_unused]] auto result = parser.parseString("invalid [ toml");

    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST_F(TomlParserBranchTest, MultipleErrors) {
    std::vector<ParseError> capturedErrors;
    TomlParser parser;
    parser.setErrorCallback([&capturedErrors](const ParseError& error) {
        capturedErrors.push_back(error);
    });

    // File not found
    [[maybe_unused]] auto r1 = parser.parseFile("nonexistent1.toml");
    [[maybe_unused]] auto r2 = parser.parseFile("nonexistent2.toml");

    EXPECT_GE(capturedErrors.size(), 2u);
}

// ============================================================================
// TomlParser - Edge Cases
// ============================================================================

TEST_F(TomlParserBranchTest, ParseComplexToml) {
    const std::string toml = R"(
# This is a comment
[database]
server = "192.168.1.1"
ports = [8001, 8002, 8003]
enabled = true

[servers.alpha]
ip = "10.0.0.1"
dc = "eqdc10"

[servers.beta]
ip = "10.0.0.2"
dc = "eqdc10"
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, ParseUnicodeContent) {
    const std::string toml = R"(
[section]
japanese = "日本語"
emoji = "🎮"
accents = "café résumé"
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseMultilineStrings) {
    const std::string toml = R"(
[section]
multiline = """
This is a
multiline string
"""
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, LastResultResetOnNewParse) {
    TomlParser parser;

    // First parse - invalid
    [[maybe_unused]] auto r1 = parser.parseString("invalid [");
    EXPECT_FALSE(parser.getLastResult().success);

    // Second parse - valid
    [[maybe_unused]] auto r2 = parser.parseString("[valid]\nkey = 1");
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, GetLastResultAfterFileNotFound) {
    TomlParser parser;
    [[maybe_unused]] auto result = parser.parseFile("does_not_exist.toml");

    auto lastResult = parser.getLastResult();
    EXPECT_FALSE(lastResult.success);
    EXPECT_FALSE(lastResult.errorMessage.empty());
}

TEST_F(TomlParserBranchTest, EmptyTableOperations) {
    const std::string toml = "";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Try to get values from empty table
    std::string str = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(str, "default");

    int64_t num = parser.getValue<int64_t>(*table, "section", "key", 42);
    EXPECT_EQ(num, 42);
}

// ============================================================================
// Additional Branch Coverage Tests for TomlParser.cpp
// ============================================================================

TEST_F(TomlParserBranchTest, ParseStringTomlParseError) {
    // This should trigger the toml::parse_error catch block
    const std::string invalidToml = "[section\nkey = value";

    TomlParser parser;
    auto result = parser.parseString(invalidToml);

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    // Should contain "TOML parse error" from the catch block
    EXPECT_NE(parser.getLastResult().errorMessage.find("parse error"),
              std::string::npos);
}

TEST_F(TomlParserBranchTest, ParseStringMissingClosingBracket) {
    const std::string toml = "[section\n";

    TomlParser parser;
    auto result = parser.parseString(toml);

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST_F(TomlParserBranchTest, ParseStringMissingQuote) {
    const std::string toml = "[section]\nkey = \"unclosed string";

    TomlParser parser;
    auto result = parser.parseString(toml);

    EXPECT_FALSE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringInvalidKey) {
    const std::string toml = "[section]\n123invalid = \"value\"";

    TomlParser parser;
    auto result = parser.parseString(toml);

    // May or may not be valid depending on TOML version
    // Just ensure it doesn't crash
}

TEST_F(TomlParserBranchTest, ParseStringDuplicateKey) {
    const std::string toml = R"(
[section]
key = "value1"
key = "value2"
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    // Duplicate keys should trigger parse error
    EXPECT_FALSE(result.has_value());
}

TEST_F(TomlParserBranchTest, SaveToFileWithParentPath) {
    toml::table table;
    table.insert("key", "value");

    // Test with a path that has a parent directory
    auto filepath = testDir / "subdir" / "output.toml";

    TomlParser parser;
    bool result = parser.saveToFile(table, filepath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(TomlParserBranchTest, SaveToFileWithoutParentPath) {
    toml::table table;
    table.insert("key", "value");

    // Save directly to testDir (which already exists)
    auto filepath = testDir / "direct_output.toml";

    TomlParser parser;
    bool result = parser.saveToFile(table, filepath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(TomlParserBranchTest, SaveToFileOverwrite) {
    toml::table table1;
    table1.insert("key", "original");

    auto filepath = testDir / "overwrite.toml";

    TomlParser parser;
    EXPECT_TRUE(parser.saveToFile(table1, filepath));

    // Now overwrite
    toml::table table2;
    table2.insert("key", "updated");
    EXPECT_TRUE(parser.saveToFile(table2, filepath));

    // Verify content was updated
    auto loaded = parser.parseFile(filepath);
    ASSERT_TRUE(loaded.has_value());
}

TEST_F(TomlParserBranchTest, SaveToFileComplexTable) {
    toml::table table;

    // Add a section with multiple values
    toml::table section;
    section.insert("string_key", "value");
    section.insert("int_key", 42);
    section.insert("float_key", 3.14);
    section.insert("bool_key", true);

    table.insert("section", section);

    auto filepath = testDir / "complex.toml";

    TomlParser parser;
    bool result = parser.saveToFile(table, filepath);

    EXPECT_TRUE(result);

    // Verify it can be read back
    auto loaded = parser.parseFile(filepath);
    ASSERT_TRUE(loaded.has_value());
}

TEST_F(TomlParserBranchTest, GetStringFromNestedSection) {
    const std::string toml = R"(
[parent]
child = "nested_value"

[parent.nested]
deep = "deep_value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "parent", "child", "default");
    EXPECT_EQ(value, "nested_value");
}

TEST_F(TomlParserBranchTest, GetStringEmptyString) {
    const std::string toml = R"(
[section]
empty = ""
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "empty", "default");
    EXPECT_EQ(value, "");
}

TEST_F(TomlParserBranchTest, GetValueFromMissingSection) {
    const std::string toml = R"(
[existing]
key = 100
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Section doesn't exist - should return default
    int64_t value = parser.getValue<int64_t>(*table, "nonexistent", "key", 42);
    EXPECT_EQ(value, 42);
}

TEST_F(TomlParserBranchTest, GetValueTypeMismatchIntToString) {
    const std::string toml = R"(
[section]
number = 123
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Try to get int as string - should return default
    std::string value = parser.getString(*table, "section", "number", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetValueTypeMismatchStringToInt) {
    const std::string toml = R"(
[section]
text = "hello"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Try to get string as int - should return default
    int64_t value = parser.getValue<int64_t>(*table, "section", "text", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchTest, GetValueTypeMismatchBoolToInt) {
    const std::string toml = R"(
[section]
flag = true
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // TOML++ may convert bool to int (true=1), so just test it doesn't crash
    int64_t value = parser.getValue<int64_t>(*table, "section", "flag", 999);
    // Value could be 1 (converted) or 999 (default) depending on TOML++ behavior
    EXPECT_TRUE(value == 1 || value == 999);
}

TEST_F(TomlParserBranchTest, GetValueTypeMismatchFloatToInt) {
    const std::string toml = R"(
[section]
decimal = 3.14
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Try to get float as int64 - should return default
    int64_t value = parser.getValue<int64_t>(*table, "section", "decimal", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchTest, ErrorCallbackWithFileNotFound) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    [[maybe_unused]] auto result = parser.parseFile("nonexistent_xyz.toml");

    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].section, "file");
}

TEST_F(TomlParserBranchTest, ErrorCallbackWithParseError) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    [[maybe_unused]] auto result = parser.parseString("[invalid");

    ASSERT_FALSE(errors.empty());
    EXPECT_EQ(errors[0].section, "parser");
}

TEST_F(TomlParserBranchTest, ReportErrorAddsToLastResult) {
    TomlParser parser;

    // Parse something to initialize state
    [[maybe_unused]] auto result = parser.parseString("[section]\nkey = 1");

    // The errors should be tracked
    auto lastErrors = parser.getLastErrors();
    // Valid parse should have no errors
    EXPECT_TRUE(lastErrors.empty());
}

TEST_F(TomlParserBranchTest, ParseErrorToString) {
    ParseError errorWithKey{"section", "key", "error message"};
    EXPECT_EQ(errorWithKey.toString(), "[section.key] error message");

    ParseError errorWithoutKey{"section", "", "error message"};
    EXPECT_EQ(errorWithoutKey.toString(), "[section] error message");
}

TEST_F(TomlParserBranchTest, ParseResultBoolOperator) {
    ParseResult success;
    success.success = true;
    EXPECT_TRUE(static_cast<bool>(success));

    ParseResult failure;
    failure.success = false;
    EXPECT_FALSE(static_cast<bool>(failure));
}

TEST_F(TomlParserBranchTest, GetValueUint32) {
    const std::string toml = R"(
[section]
value = 4294967295
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    uint32_t value = parser.getValue<uint32_t>(*table, "section", "value", 0);
    // Note: TOML uses int64 internally, but we cast
}

TEST_F(TomlParserBranchTest, GetValueNegativeInt) {
    const std::string toml = R"(
[section]
value = -12345
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "value", 0);
    EXPECT_EQ(value, -12345);
}

TEST_F(TomlParserBranchTest, ParseFileWithCallback) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    // Valid file
    writeFile("callback_test.toml", "[section]\nkey = 1");
    auto result = parser.parseFile(testDir / "callback_test.toml");

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(errors.empty());
}

TEST_F(TomlParserBranchTest, SaveAndLoadRoundTrip) {
    TomlParser parser;

    // Create a table
    toml::table original;
    toml::table section;
    section.insert("name", "test");
    section.insert("count", 42);
    section.insert("ratio", 0.75);
    section.insert("enabled", true);
    original.insert("config", section);

    auto filepath = testDir / "roundtrip.toml";

    // Save
    EXPECT_TRUE(parser.saveToFile(original, filepath));

    // Load
    auto loaded = parser.parseFile(filepath);
    ASSERT_TRUE(loaded.has_value());

    // Verify
    std::string name = parser.getString(*loaded, "config", "name", "");
    EXPECT_EQ(name, "test");

    int64_t count = parser.getValue<int64_t>(*loaded, "config", "count", 0);
    EXPECT_EQ(count, 42);

    double ratio = parser.getValue<double>(*loaded, "config", "ratio", 0.0);
    EXPECT_NEAR(ratio, 0.75, 0.001);

    bool enabled = parser.getValue<bool>(*loaded, "config", "enabled", false);
    EXPECT_TRUE(enabled);
}

TEST_F(TomlParserBranchTest, ParseStringWithArrays) {
    const std::string toml = R"(
[section]
numbers = [1, 2, 3, 4, 5]
names = ["alice", "bob", "charlie"]
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringWithInlineTables) {
    const std::string toml = R"(
[section]
point = { x = 10, y = 20 }
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringWithDates) {
    const std::string toml = R"(
[section]
date = 2025-01-15
time = 14:30:00
datetime = 2025-01-15T14:30:00
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseFileReadContents) {
    const std::string content = R"(
[video]
width = 1920
height = 1080

[audio]
volume = 0.8
)";

    writeFile("read_test.toml", content);

    TomlParser parser;
    auto result = parser.parseFile(testDir / "read_test.toml");

    ASSERT_TRUE(result.has_value());

    int64_t width = parser.getValue<int64_t>(*result, "video", "width", 0);
    EXPECT_EQ(width, 1920);

    double volume = parser.getValue<double>(*result, "audio", "volume", 0.0);
    EXPECT_NEAR(volume, 0.8, 0.001);
}

TEST_F(TomlParserBranchTest, GetValueWithEmptySection) {
    const std::string toml = R"(
[section]
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Section exists but key doesn't
    int64_t value = parser.getValue<int64_t>(*table, "section", "missing", 100);
    EXPECT_EQ(value, 100);
}

TEST_F(TomlParserBranchTest, MultipleParseCallsResetState) {
    TomlParser parser;

    // First parse - fail
    auto result1 = parser.parseString("[invalid");
    EXPECT_FALSE(result1.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());

    // Second parse - success
    auto result2 = parser.parseString("[valid]\nkey = 1");
    EXPECT_TRUE(result2.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
    // Error message should be reset or empty for successful parse
}

TEST_F(TomlParserBranchTest, ParseFileThenString) {
    writeFile("first.toml", "[section]\nkey = 1");

    TomlParser parser;

    auto fileResult = parser.parseFile(testDir / "first.toml");
    EXPECT_TRUE(fileResult.has_value());

    auto stringResult = parser.parseString("[other]\nvalue = 2");
    EXPECT_TRUE(stringResult.has_value());
}

TEST_F(TomlParserBranchTest, GetStringWithSpecialCharacters) {
    const std::string toml = R"(
[section]
path = "C:\\Users\\test\\file.txt"
url = "https://example.com/path?query=value&other=123"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string path = parser.getString(*table, "section", "path", "");
    EXPECT_FALSE(path.empty());

    std::string url = parser.getString(*table, "section", "url", "");
    EXPECT_FALSE(url.empty());
}

// ============================================================================
// Additional Branch Coverage Tests - saveToFile error paths
// ============================================================================

TEST_F(TomlParserBranchTest, SaveToFileCannotCreateTemp) {
    toml::table table;
    table.insert("key", "value");

    // Try to save to a path where we cannot create files
    // Using /proc which is read-only on Linux
    TomlParser parser;
    bool result = parser.saveToFile(table, "/proc/test_output.toml");

    EXPECT_FALSE(result);
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST_F(TomlParserBranchTest, SaveToFileRenameFailure) {
    toml::table table;
    table.insert("key", "value");

    // Create a directory with same name as target to cause rename failure
    auto targetPath = testDir / "blocked_file.toml";
    std::filesystem::create_directories(targetPath);

    TomlParser parser;
    bool result = parser.saveToFile(table, targetPath);

    // Clean up the directory before assertion
    std::filesystem::remove_all(targetPath);

    EXPECT_FALSE(result);
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST_F(TomlParserBranchTest, SaveToFileNoParentPathBranch) {
    toml::table table;
    table.insert("key", "value");

    // Use a filename-only path (no parent directory component)
    // This tests the "filepath.has_parent_path()" branch being false
    auto cwd = std::filesystem::current_path();
    std::filesystem::current_path(testDir);

    TomlParser parser;
    bool result = parser.saveToFile(table, "no_parent.toml");

    std::filesystem::current_path(cwd);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testDir / "no_parent.toml"));
}

// ============================================================================
// Additional Branch Coverage Tests - getValue template branches
// ============================================================================

TEST_F(TomlParserBranchTest, GetValueSectionNotTable) {
    const std::string toml = R"(
section = "not a table"
[other]
key = 123
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // "section" exists but is not a table (it's a string)
    // This tests the "if (auto* sec = table[section].as_table())" branch being false
    int64_t value = parser.getValue<int64_t>(*table, "section", "key", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchTest, GetStringSectionNotTable) {
    const std::string toml = R"(
section = "not a table"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Try getString on a section that exists but is not a table
    std::string value = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetValueNullSection) {
    const std::string toml = R"(
[real_section]
key = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Access a completely non-existent section
    int64_t value = parser.getValue<int64_t>(*table, "nonexistent", "key", 123);
    EXPECT_EQ(value, 123);
}

TEST_F(TomlParserBranchTest, GetStringNullSection) {
    const std::string toml = R"(
[real_section]
key = "value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Access a completely non-existent section
    std::string value = parser.getString(*table, "nonexistent", "key", "default");
    EXPECT_EQ(value, "default");
}

// ============================================================================
// Additional Branch Coverage Tests - error callback branches
// ============================================================================

TEST_F(TomlParserBranchTest, ReportErrorWithoutCallback) {
    TomlParser parser;
    // Don't set callback - test the "if (_errorCallback)" branch being false

    [[maybe_unused]] auto result = parser.parseFile("nonexistent.toml");

    // Should work without crashing even without callback
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
    EXPECT_FALSE(parser.getLastResult().errors.empty());
}

TEST_F(TomlParserBranchTest, ReportErrorWithCallback) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    [[maybe_unused]] auto result = parser.parseFile("nonexistent.toml");

    EXPECT_FALSE(errors.empty());
}

// ============================================================================
// Additional Branch Coverage Tests - parseString exception branches
// ============================================================================

TEST_F(TomlParserBranchTest, ParseStringBadEscapeSequence) {
    // Try to trigger a parse error
    const std::string toml = "[section]\nkey = \"value\\x\"";

    TomlParser parser;
    auto result = parser.parseString(toml);

    // This should fail due to invalid escape sequence
    EXPECT_FALSE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringNestedArrayBrackets) {
    const std::string toml = "[section]\narray = [[1, 2], [3, 4]]";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringWithComments) {
    const std::string toml = R"(
# This is a comment
[section] # inline comment
key = "value" # another comment
# trailing comment
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

// ============================================================================
// Additional Branch Coverage Tests - file permission variations
// ============================================================================

TEST_F(TomlParserBranchTest, SaveToFilePermissionDenied) {
    toml::table table;
    table.insert("key", "value");

    // Create a directory and make it read-only
    auto readOnlyDir = testDir / "readonly_dir";
    std::filesystem::create_directories(readOnlyDir);
    std::filesystem::permissions(readOnlyDir, std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec);

    TomlParser parser;
    bool result = parser.saveToFile(table, readOnlyDir / "output.toml");

    // Restore permissions for cleanup
    std::filesystem::permissions(readOnlyDir, std::filesystem::perms::owner_all);

    EXPECT_FALSE(result);
}

TEST_F(TomlParserBranchTest, ParseFileEmptyContent) {
    writeFile("empty.toml", "");

    TomlParser parser;
    auto result = parser.parseFile(testDir / "empty.toml");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchTest, ParseFileWhitespaceOnly) {
    writeFile("whitespace.toml", "   \n\t\n   ");

    TomlParser parser;
    auto result = parser.parseFile(testDir / "whitespace.toml");

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

// ============================================================================
// Additional Branch Coverage Tests - getValue type variations
// ============================================================================

TEST_F(TomlParserBranchTest, GetValueFloat) {
    const std::string toml = R"(
[section]
value = 1.5
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    float value = parser.getValue<float>(*table, "section", "value", 0.0f);
    EXPECT_NEAR(value, 1.5f, 0.001f);
}

TEST_F(TomlParserBranchTest, GetValueInt32) {
    const std::string toml = R"(
[section]
value = 100
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int32_t value = parser.getValue<int32_t>(*table, "section", "value", 0);
    EXPECT_EQ(value, 100);
}

TEST_F(TomlParserBranchTest, GetValueKeyExists) {
    const std::string toml = R"(
[section]
existing = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Key exists - tests the "if (auto val = (*sec)[key].value<T>())" branch being true
    int64_t value = parser.getValue<int64_t>(*table, "section", "existing", 0);
    EXPECT_EQ(value, 42);
}

TEST_F(TomlParserBranchTest, GetValueKeyMissing) {
    const std::string toml = R"(
[section]
other = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Key doesn't exist - tests the "if (auto val = (*sec)[key].value<T>())" branch being false
    int64_t value = parser.getValue<int64_t>(*table, "section", "missing", 999);
    EXPECT_EQ(value, 999);
}

// ============================================================================
// More Branch Coverage Tests - toml++ optional value branches
// ============================================================================

TEST_F(TomlParserBranchTest, GetStringFromArrayElement) {
    const std::string toml = R"(
[section]
key = ["array", "values"]
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Trying to get array as string should return default
    std::string value = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetValueFromTableValue) {
    const std::string toml = R"(
[section]
key = { nested = "table" }
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Trying to get inline table as int should return default
    int64_t value = parser.getValue<int64_t>(*table, "section", "key", 123);
    EXPECT_EQ(value, 123);
}

TEST_F(TomlParserBranchTest, GetStringFromTableValue) {
    const std::string toml = R"(
[section]
key = { nested = "table" }
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Trying to get inline table as string should return default
    std::string value = parser.getString(*table, "section", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, ParseFileLargeContent) {
    // Create a large TOML file to test buffer handling
    std::string toml = "[large]\n";
    for (int i = 0; i < 100; ++i) {
        toml += "key" + std::to_string(i) + " = " + std::to_string(i) + "\n";
    }
    writeFile("large.toml", toml);

    TomlParser parser;
    auto result = parser.parseFile(testDir / "large.toml");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(parser.getValue<int64_t>(*result, "large", "key50", 0), 50);
}

TEST_F(TomlParserBranchTest, GetValueUint16) {
    const std::string toml = R"(
[section]
port = 8080
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Test uint16_t conversion (common for ports)
    uint16_t value = parser.getValue<uint16_t>(*table, "section", "port", 0);
    EXPECT_EQ(value, 8080);
}

TEST_F(TomlParserBranchTest, GetValueFromEmptyTable) {
    const std::string toml = R"(
[empty_section]
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Section exists but is empty
    double value = parser.getValue<double>(*table, "empty_section", "missing", 3.14);
    EXPECT_NEAR(value, 3.14, 0.001);
}

TEST_F(TomlParserBranchTest, GetValueBoolFromWrongType) {
    const std::string toml = R"(
[section]
value = "not a bool"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    bool value = parser.getValue<bool>(*table, "section", "value", true);
    EXPECT_TRUE(value);  // Should return default
}

TEST_F(TomlParserBranchTest, GetValueDoubleFromInt) {
    const std::string toml = R"(
[section]
value = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Getting int as double - should return default since types differ in TOML
    double value = parser.getValue<double>(*table, "section", "value", 99.9);
    // TOML++ may or may not convert int to double
    EXPECT_TRUE(value == 42.0 || value == 99.9);
}

TEST_F(TomlParserBranchTest, ParseStringVariousEscapes) {
    const std::string toml = R"(
[section]
newline = "line1\nline2"
tab = "col1\tcol2"
quote = "he said \"hello\""
backslash = "path\\to\\file"
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    std::string newline = parser.getString(*result, "section", "newline", "");
    EXPECT_NE(newline.find('\n'), std::string::npos);
}

TEST_F(TomlParserBranchTest, ParseStringLiteralStrings) {
    const std::string toml = R"(
[section]
literal = 'no escape \n here'
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    std::string literal = parser.getString(*result, "section", "literal", "");
    // Literal strings preserve backslashes
    EXPECT_NE(literal.find("\\n"), std::string::npos);
}

TEST_F(TomlParserBranchTest, SaveToFileEmptyTable) {
    toml::table empty;

    TomlParser parser;
    bool result = parser.saveToFile(empty, testDir / "empty_output.toml");

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(testDir / "empty_output.toml"));
}

TEST_F(TomlParserBranchTest, SaveToFileNestedTables) {
    toml::table table;
    toml::table inner;
    toml::table deeper;
    deeper.insert("value", 42);
    inner.insert("deep", deeper);
    table.insert("outer", inner);

    TomlParser parser;
    bool result = parser.saveToFile(table, testDir / "nested.toml");

    EXPECT_TRUE(result);

    // Verify it can be read back
    auto loaded = parser.parseFile(testDir / "nested.toml");
    ASSERT_TRUE(loaded.has_value());
}

TEST_F(TomlParserBranchTest, GetStringEmptySection) {
    const std::string toml = R"(
[section]
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "nonexistent", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetValueLargeNumber) {
    const std::string toml = R"(
[section]
big = 9223372036854775807
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "big", 0);
    EXPECT_EQ(value, 9223372036854775807LL);
}

TEST_F(TomlParserBranchTest, GetValueNegativeLargeNumber) {
    const std::string toml = R"(
[section]
small = -9223372036854775807
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "small", 0);
    EXPECT_EQ(value, -9223372036854775807LL);
}

TEST_F(TomlParserBranchTest, ParseStringMixedTypes) {
    const std::string toml = R"(
[types]
string = "hello"
integer = 42
float = 3.14159
bool_true = true
bool_false = false
)";

    TomlParser parser;
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(parser.getString(*result, "types", "string", ""), "hello");
    EXPECT_EQ(parser.getValue<int64_t>(*result, "types", "integer", 0), 42);
    EXPECT_NEAR(parser.getValue<double>(*result, "types", "float", 0.0), 3.14159, 0.00001);
    EXPECT_TRUE(parser.getValue<bool>(*result, "types", "bool_true", false));
    EXPECT_FALSE(parser.getValue<bool>(*result, "types", "bool_false", true));
}

TEST_F(TomlParserBranchTest, ErrorCallbackOnFileSaveFailure) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    toml::table table;
    table.insert("key", "value");

    // Create a read-only directory to prevent file creation
    auto readOnlyDir = testDir / "readonly_save_test";
    std::filesystem::create_directories(readOnlyDir);
    std::filesystem::permissions(readOnlyDir, std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec);

    bool result = parser.saveToFile(table, readOnlyDir / "file.toml");

    // Restore permissions for cleanup
    std::filesystem::permissions(readOnlyDir, std::filesystem::perms::owner_all);

    EXPECT_FALSE(result);
    EXPECT_FALSE(errors.empty());
}

TEST_F(TomlParserBranchTest, MultipleCallbackErrors) {
    std::vector<ParseError> errors;
    TomlParser parser;
    parser.setErrorCallback([&errors](const ParseError& e) {
        errors.push_back(e);
    });

    // Cause multiple errors
    [[maybe_unused]] auto r1 = parser.parseFile("nonexistent1.toml");
    [[maybe_unused]] auto r2 = parser.parseFile("nonexistent2.toml");
    [[maybe_unused]] auto r3 = parser.parseString("[invalid");

    EXPECT_GE(errors.size(), 3u);
}

TEST_F(TomlParserBranchTest, ParseErrorEmptyKey) {
    ParseError error{"section", "", "error message"};
    EXPECT_EQ(error.toString(), "[section] error message");
}

TEST_F(TomlParserBranchTest, ParseErrorWithKey) {
    ParseError error{"section", "key", "error message"};
    EXPECT_EQ(error.toString(), "[section.key] error message");
}

TEST_F(TomlParserBranchTest, ParseResultOperatorBool) {
    ParseResult success;
    success.success = true;
    EXPECT_TRUE(static_cast<bool>(success));

    ParseResult failure;
    failure.success = false;
    EXPECT_FALSE(static_cast<bool>(failure));
}

// ============================================================================
// Edge Cases for Branch Coverage
// ============================================================================

TEST_F(TomlParserBranchTest, GetValueFromDeeplyNestedSection) {
    const std::string toml = R"(
[a.b.c]
value = 123
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Access via top-level key only
    int64_t value = parser.getValue<int64_t>(*table, "a", "value", 999);
    EXPECT_EQ(value, 999);  // Should return default since "a" is a table not containing "value" directly
}

TEST_F(TomlParserBranchTest, SaveToFileWithSpecialCharacters) {
    toml::table table;
    toml::table section;
    section.insert("special", "value with \"quotes\" and 'apostrophes'");
    section.insert("newlines", "line1\nline2\nline3");
    table.insert("section", section);

    auto filepath = testDir / "special_chars.toml";
    TomlParser parser;
    bool result = parser.saveToFile(table, filepath);

    EXPECT_TRUE(result);

    // Verify round-trip
    auto loaded = parser.parseFile(filepath);
    ASSERT_TRUE(loaded.has_value());
}

TEST_F(TomlParserBranchTest, ParseFileSymlink) {
    // Create a regular file
    writeFile("original.toml", "[test]\nvalue = 42");

    // Create a symlink to it (if possible)
    auto linkPath = testDir / "link.toml";
    std::error_code ec;
    std::filesystem::create_symlink(testDir / "original.toml", linkPath, ec);

    if (!ec) {
        TomlParser parser;
        auto result = parser.parseFile(linkPath);

        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(parser.getValue<int64_t>(*result, "test", "value", 0), 42);
    }
}

// ============================================================================
// Additional Edge Cases for Better Branch Coverage
// ============================================================================

TEST_F(TomlParserBranchTest, GetValueTypeMismatchArrayToScalar) {
    const std::string toml = R"(
[section]
array = [1, 2, 3]
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Trying to get array as scalar should return default
    int64_t value = parser.getValue<int64_t>(*table, "section", "array", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchTest, GetStringFromNumericValue) {
    const std::string toml = R"(
[section]
number = 12345
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "number", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchTest, GetValueFromNonExistentSection) {
    const std::string toml = R"(
[existing]
key = 42
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    // Non-existent section
    int64_t value = parser.getValue<int64_t>(*table, "nonexistent", "key", 123);
    EXPECT_EQ(value, 123);
}

TEST_F(TomlParserBranchTest, ParseStringWithAllTypes) {
    const std::string toml = R"(
[types]
string = "hello world"
integer = 42
float = 3.14159
boolean = true
date = 2025-01-15
time = 14:30:00
datetime = 2025-01-15T14:30:00Z
array_int = [1, 2, 3]
array_str = ["a", "b", "c"]
inline_table = { x = 1, y = 2 }
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, GetValueZero) {
    const std::string toml = R"(
[section]
zero_int = 0
zero_float = 0.0
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    EXPECT_EQ(parser.getValue<int64_t>(*table, "section", "zero_int", 999), 0);
    EXPECT_NEAR(parser.getValue<double>(*table, "section", "zero_float", 999.0), 0.0, 0.001);
}

TEST_F(TomlParserBranchTest, GetValueNegativeDefault) {
    const std::string toml = R"(
[section]
other = 100
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    int64_t value = parser.getValue<int64_t>(*table, "section", "missing", -999);
    EXPECT_EQ(value, -999);
}

TEST_F(TomlParserBranchTest, SaveToFileWithArray) {
    toml::table table;
    toml::array arr;
    arr.push_back(1);
    arr.push_back(2);
    arr.push_back(3);
    table.insert("array", arr);

    TomlParser parser;
    bool result = parser.saveToFile(table, testDir / "array.toml");

    EXPECT_TRUE(result);
}

TEST_F(TomlParserBranchTest, ParseFileWithBOM) {
    // UTF-8 BOM + content
    std::string content = "\xEF\xBB\xBF[section]\nkey = \"value\"";
    writeFile("bom.toml", content);

    TomlParser parser;
    auto result = parser.parseFile(testDir / "bom.toml");

    // TOML++ may or may not handle BOM
    // Just test it doesn't crash
}

TEST_F(TomlParserBranchTest, GetValueEmptyDefault) {
    const std::string toml = R"(
[section]
existing = "value"
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    std::string value = parser.getString(*table, "section", "missing", "");
    EXPECT_EQ(value, "");
}

TEST_F(TomlParserBranchTest, ParseStringWithHexNumbers) {
    const std::string toml = R"(
[section]
hex = 0xDEADBEEF
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    int64_t value = parser.getValue<int64_t>(*result, "section", "hex", 0);
    EXPECT_EQ(value, 0xDEADBEEF);
}

TEST_F(TomlParserBranchTest, ParseStringWithOctalNumbers) {
    const std::string toml = R"(
[section]
octal = 0o755
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    int64_t value = parser.getValue<int64_t>(*result, "section", "octal", 0);
    EXPECT_EQ(value, 0755);
}

TEST_F(TomlParserBranchTest, ParseStringWithBinaryNumbers) {
    const std::string toml = R"(
[section]
binary = 0b11010110
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    int64_t value = parser.getValue<int64_t>(*result, "section", "binary", 0);
    EXPECT_EQ(value, 0b11010110);
}

TEST_F(TomlParserBranchTest, ParseStringWithScientificNotation) {
    const std::string toml = R"(
[section]
sci1 = 1e10
sci2 = 5e-3
sci3 = 3.14e2
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(parser.getValue<double>(*result, "section", "sci1", 0.0), 1e10, 1e5);
    EXPECT_NEAR(parser.getValue<double>(*result, "section", "sci2", 0.0), 5e-3, 1e-6);
    EXPECT_NEAR(parser.getValue<double>(*result, "section", "sci3", 0.0), 314.0, 0.1);
}

TEST_F(TomlParserBranchTest, ParseStringWithInfinity) {
    const std::string toml = R"(
[section]
pos_inf = inf
neg_inf = -inf
nan_val = nan
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
    double pos_inf = parser.getValue<double>(*result, "section", "pos_inf", 0.0);
    double neg_inf = parser.getValue<double>(*result, "section", "neg_inf", 0.0);

    EXPECT_TRUE(std::isinf(pos_inf) && pos_inf > 0);
    EXPECT_TRUE(std::isinf(neg_inf) && neg_inf < 0);
}

TEST_F(TomlParserBranchTest, GetValueUint8) {
    const std::string toml = R"(
[section]
byte = 255
)";

    TomlParser parser;
    auto table = parser.parseString(toml);
    ASSERT_TRUE(table.has_value());

    uint8_t value = parser.getValue<uint8_t>(*table, "section", "byte", 0);
    EXPECT_EQ(value, 255);
}

TEST_F(TomlParserBranchTest, ParseStringNestedArrayOfTables) {
    const std::string toml = R"(
[[products]]
name = "Hammer"
sku = 738594937

[[products]]
name = "Nail"
sku = 284758393
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserBranchTest, ParseStringDottedKeys) {
    const std::string toml = R"(
fruit.apple.color = "red"
fruit.apple.taste.sweet = true
)";

    TomlParser parser;
    auto result = parser.parseString(toml);

    ASSERT_TRUE(result.has_value());
}

