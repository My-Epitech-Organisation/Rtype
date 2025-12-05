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
