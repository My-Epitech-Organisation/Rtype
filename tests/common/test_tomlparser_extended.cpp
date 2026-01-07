/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Additional TomlParser tests for branch coverage
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <rtype/common.hpp>

using namespace rtype::config;

class TomlParserExtendedTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "toml_ext_test";
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
// parseString() Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, ParseStringValid) {
    TomlParser parser;
    std::string toml = R"(
[server]
port = 8080
host = "localhost"
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
    EXPECT_TRUE(parser.getLastErrors().empty());
}

TEST_F(TomlParserExtendedTest, ParseStringInvalidSyntax) {
    TomlParser parser;
    std::string toml = "[section\nkey = value";  // Missing closing bracket

    auto result = parser.parseString(toml);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    EXPECT_FALSE(parser.getLastErrors().empty());
}

TEST_F(TomlParserExtendedTest, ParseStringEmpty) {
    TomlParser parser;
    std::string toml = "";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserExtendedTest, ParseStringWithComments) {
    TomlParser parser;
    std::string toml = R"(
# Comment
[section]
key = 42  # Inline comment
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(parser.getValue(*result, "section", "key", 0), 42);
}

TEST_F(TomlParserExtendedTest, ParseStringMultipleSections) {
    TomlParser parser;
    std::string toml = R"(
[section1]
key1 = 1

[section2]
key2 = 2

[section3]
key3 = 3
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(parser.getValue(*result, "section1", "key1", 0), 1);
    EXPECT_EQ(parser.getValue(*result, "section2", "key2", 0), 2);
    EXPECT_EQ(parser.getValue(*result, "section3", "key3", 0), 3);
}

// ============================================================================
// getValue() Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, GetValueSectionNotFound) {
    TomlParser parser;
    std::string toml = "[other]\nkey = 1";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    int value = parser.getValue(*result, "missing_section", "key", 999);
    EXPECT_EQ(value, 999);  // Returns default
}

TEST_F(TomlParserExtendedTest, GetValueKeyNotFound) {
    TomlParser parser;
    std::string toml = "[section]\nother_key = 1";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    int value = parser.getValue(*result, "section", "missing_key", 888);
    EXPECT_EQ(value, 888);  // Returns default
}

TEST_F(TomlParserExtendedTest, GetValueWrongType) {
    TomlParser parser;
    std::string toml = "[section]\nkey = \"string_value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    // Trying to get string as int should return default
    int value = parser.getValue(*result, "section", "key", 777);
    EXPECT_EQ(value, 777);
}

TEST_F(TomlParserExtendedTest, GetValueBoolean) {
    TomlParser parser;
    std::string toml = "[section]\nflag = true";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    bool value = parser.getValue(*result, "section", "flag", false);
    EXPECT_TRUE(value);
}

TEST_F(TomlParserExtendedTest, GetValueDouble) {
    TomlParser parser;
    std::string toml = "[section]\nvalue = 3.14159";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    double value = parser.getValue(*result, "section", "value", 0.0);
    EXPECT_DOUBLE_EQ(value, 3.14159);
}

TEST_F(TomlParserExtendedTest, GetValueNegativeNumber) {
    TomlParser parser;
    std::string toml = "[section]\nvalue = -42";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    int value = parser.getValue(*result, "section", "value", 0);
    EXPECT_EQ(value, -42);
}

// ============================================================================
// getString() Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, GetStringValid) {
    TomlParser parser;
    std::string toml = "[section]\nname = \"test_string\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "section", "name", "default");
    EXPECT_EQ(value, "test_string");
}

TEST_F(TomlParserExtendedTest, GetStringNotFound) {
    TomlParser parser;
    std::string toml = "[section]\nother = \"value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "section", "missing", "fallback");
    EXPECT_EQ(value, "fallback");
}

TEST_F(TomlParserExtendedTest, GetStringSectionNotFound) {
    TomlParser parser;
    std::string toml = "[other]\nkey = \"value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "missing", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserExtendedTest, GetStringEmptyValue) {
    TomlParser parser;
    std::string toml = "[section]\nkey = \"\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "section", "key", "default");
    EXPECT_EQ(value, "");
}

TEST_F(TomlParserExtendedTest, GetStringWithSpecialChars) {
    TomlParser parser;
    std::string toml = "[section]\nkey = \"Special: !@#$%^&*()\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "section", "key", "");
    EXPECT_EQ(value, "Special: !@#$%^&*()");
}

// ============================================================================
// saveToFile() Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, SaveToFileSuccess) {
    TomlParser parser;
    toml::table table;
    table.insert_or_assign("section", toml::table{{"key", 42}});

    auto filePath = testDir / "output.toml";
    bool result = parser.saveToFile(table, filePath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filePath));
}

TEST_F(TomlParserExtendedTest, SaveToFileCreatesDirectory) {
    TomlParser parser;
    toml::table table;
    table.insert_or_assign("section", toml::table{{"key", 42}});

    auto filePath = testDir / "subdir" / "nested" / "output.toml";
    bool result = parser.saveToFile(table, filePath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filePath));
    EXPECT_TRUE(std::filesystem::is_directory(testDir / "subdir" / "nested"));
}

TEST_F(TomlParserExtendedTest, SaveToFileEmptyTable) {
    TomlParser parser;
    toml::table table;  // Empty table

    auto filePath = testDir / "empty.toml";
    bool result = parser.saveToFile(table, filePath);

    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filePath));
}

TEST_F(TomlParserExtendedTest, SaveToFileOverwriteExisting) {
    TomlParser parser;
    auto filePath = testDir / "overwrite.toml";

    // Write first content
    toml::table table1;
    table1.insert_or_assign("section", toml::table{{"key", 1}});
    parser.saveToFile(table1, filePath);

    // Overwrite with new content
    toml::table table2;
    table2.insert_or_assign("section", toml::table{{"key", 2}});
    bool result = parser.saveToFile(table2, filePath);

    EXPECT_TRUE(result);

    // Verify the file was overwritten
    auto parsed = parser.parseFile(filePath);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parser.getValue(*parsed, "section", "key", 0), 2);
}

// ============================================================================
// Error Reporting Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, ErrorCallbackCalled) {
    TomlParser parser;
    int callbackCount = 0;
    std::string lastError;

    parser.setErrorCallback([&](const ParseError& error) {
        callbackCount++;
        lastError = error.toString();
    });

    auto result = parser.parseFile("nonexistent.toml");
    (void)result;

    EXPECT_GT(callbackCount, 0);
    EXPECT_FALSE(lastError.empty());
}

TEST_F(TomlParserExtendedTest, ReportErrorManually) {
    TomlParser parser;
    int callbackCount = 0;

    parser.setErrorCallback([&](const ParseError&) {
        callbackCount++;
    });

    ParseError error{"test_section", "test_key", "test message"};
    parser.reportError(error);

    EXPECT_EQ(callbackCount, 1);
}

TEST_F(TomlParserExtendedTest, ParseErrorToString) {
    ParseError error1{"section", "key", "message"};
    EXPECT_EQ(error1.toString(), "[section.key] message");

    ParseError error2{"section", "", "message"};
    EXPECT_EQ(error2.toString(), "[section] message");
}

TEST_F(TomlParserExtendedTest, GetLastResultAfterSuccess) {
    TomlParser parser;
    auto parseResult = parser.parseString("[section]\nkey = 1");
    (void)parseResult;

    const auto& result = parser.getLastResult();
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.errors.empty());
}

TEST_F(TomlParserExtendedTest, GetLastResultAfterFailure) {
    TomlParser parser;
    auto parseResult = parser.parseString("[invalid syntax");
    (void)parseResult;

    const auto& result = parser.getLastResult();
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errors.empty());
}

TEST_F(TomlParserExtendedTest, GetLastErrorsMultiple) {
    TomlParser parser;
    
    parser.reportError({"sec1", "key1", "error1"});
    parser.reportError({"sec2", "key2", "error2"});
    
    const auto& errors = parser.getLastErrors();
    EXPECT_GE(errors.size(), 2u);
}

// ============================================================================
// Complex TOML Structures Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, ParseNestedTables) {
    TomlParser parser;
    std::string toml = R"(
[parent]
value = 1

[parent.child]
value = 2

[parent.child.grandchild]
value = 3
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserExtendedTest, ParseArrays) {
    TomlParser parser;
    std::string toml = R"(
[section]
numbers = [1, 2, 3, 4, 5]
strings = ["a", "b", "c"]
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserExtendedTest, ParseInlineTable) {
    TomlParser parser;
    std::string toml = R"(
[section]
inline = { key1 = 1, key2 = 2 }
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
}

TEST_F(TomlParserExtendedTest, ParseDateTime) {
    TomlParser parser;
    std::string toml = R"(
[section]
date = 2024-01-15T10:30:00Z
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

TEST_F(TomlParserExtendedTest, ParseStringVeryLong) {
    TomlParser parser;
    std::string longValue(10000, 'x');
    std::string toml = "[section]\nkey = \"" + longValue + "\"";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    std::string value = parser.getString(*result, "section", "key", "");
    EXPECT_EQ(value.length(), 10000u);
}

TEST_F(TomlParserExtendedTest, ParseManySections) {
    TomlParser parser;
    std::string toml;
    for (int i = 0; i < 100; ++i) {
        toml += "[section" + std::to_string(i) + "]\n";
        toml += "key = " + std::to_string(i) + "\n\n";
    }

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    // Verify a few values
    EXPECT_EQ(parser.getValue(*result, "section0", "key", -1), 0);
    EXPECT_EQ(parser.getValue(*result, "section50", "key", -1), 50);
    EXPECT_EQ(parser.getValue(*result, "section99", "key", -1), 99);
}

TEST_F(TomlParserExtendedTest, GetValueWithUnicode) {
    TomlParser parser;
    std::string toml = R"([section]
text = "Hello 世界 🌍"
)";

    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    std::string value = parser.getString(*result, "section", "text", "");
    EXPECT_FALSE(value.empty());
}

TEST_F(TomlParserExtendedTest, ParseResultBoolOperator) {
    ParseResult successResult;
    successResult.success = true;
    EXPECT_TRUE(static_cast<bool>(successResult));

    ParseResult failureResult;
    failureResult.success = false;
    EXPECT_FALSE(static_cast<bool>(failureResult));
}

TEST_F(TomlParserExtendedTest, GetValueZero) {
    TomlParser parser;
    std::string toml = "[section]\nvalue = 0";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    int value = parser.getValue(*result, "section", "value", 999);
    EXPECT_EQ(value, 0);  // Should return 0, not default
}

TEST_F(TomlParserExtendedTest, GetStringWrongType) {
    TomlParser parser;
    std::string toml = "[section]\nvalue = 123";  // Number not string
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());

    std::string value = parser.getString(*result, "section", "value", "fallback");
    EXPECT_EQ(value, "fallback");  // Should return default
}

TEST_F(TomlParserExtendedTest, SaveToFileWithComplexStructure) {
    TomlParser parser;
    toml::table table;
    
    toml::table section;
    section.insert_or_assign("string", "value");
    section.insert_or_assign("number", 42);
    section.insert_or_assign("float", 3.14);
    section.insert_or_assign("bool", true);
    
    table.insert_or_assign("section", section);

    auto filePath = testDir / "complex.toml";
    bool result = parser.saveToFile(table, filePath);

    EXPECT_TRUE(result);
    
    // Read it back and verify
    auto parsed = parser.parseFile(filePath);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parser.getString(*parsed, "section", "string", ""), "value");
    EXPECT_EQ(parser.getValue(*parsed, "section", "number", 0), 42);
}
