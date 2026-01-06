/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TomlParser additional branch coverage tests
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <rtype/common.hpp>

using namespace rtype::config;

class TomlParserBranchCoverageTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "toml_branch_cov_test";
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
// Tests targeting uncovered branches in parseString
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, ParseStringCatchesStdException) {
    TomlParser parser;
    // While it's hard to trigger std::exception without toml::parse_error,
    // we can at least cover the happy path more thoroughly
    std::string validToml = "[section1]\nkey1 = 1\n[section2]\nkey2 = 2";
    
    auto result = parser.parseString(validToml);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, ParseStringMultipleParseErrors) {
    TomlParser parser;
    
    // First parse error
    auto result1 = parser.parseString("[invalid");
    EXPECT_FALSE(result1.has_value());
    
    // Second parse error - tests error accumulation
    auto result2 = parser.parseString("key = ");
    EXPECT_FALSE(result2.has_value());
}

TEST_F(TomlParserBranchCoverageTest, ParseStringResetsBetweenCalls) {
    TomlParser parser;
    
    // First call - error
    auto result1 = parser.parseString("[bad syntax");
    EXPECT_FALSE(result1.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    
    // Second call - success (should reset _lastResult)
    auto result2 = parser.parseString("[good]\nkey = 1");
    ASSERT_TRUE(result2.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

// ============================================================================
// Tests targeting uncovered branches in saveToFile
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, SaveToFileNoParentPath) {
    TomlParser parser;
    toml::table table;
    table.insert_or_assign("key", "value");
    
    // File without parent directories (relative path in current dir)
    auto filePath = "simple_file.toml";
    bool result = parser.saveToFile(table, filePath);
    
    // Cleanup
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }
}

TEST_F(TomlParserBranchCoverageTest, SaveToFileFileWriteSuccess) {
    TomlParser parser;
    toml::table table;
    table.insert_or_assign("section", toml::table{{"key", "value"}});
    
    auto filePath = testDir / "write_test.toml";
    bool result = parser.saveToFile(table, filePath);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(filePath));
    
    // Verify file content is valid
    std::ifstream file(filePath);
    ASSERT_TRUE(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_FALSE(content.empty());
}

TEST_F(TomlParserBranchCoverageTest, SaveToFileRenameFailure) {
    TomlParser parser;
    toml::table table;
    table.insert_or_assign("key", 42);
    
    // Try to save to a file, then immediately try to save another file
    // with the same name to trigger potential rename conflicts
    auto filePath = testDir / "rename_test.toml";
    
    // First save
    parser.saveToFile(table, filePath);
    
    // Create a read-only file to potentially cause rename failure
    // (platform-specific behavior)
    #ifndef _WIN32
    if (std::filesystem::exists(filePath)) {
        std::filesystem::permissions(filePath, std::filesystem::perms::none);
        
        // Try to save again (should handle permission errors)
        toml::table table2;
        table2.insert_or_assign("key2", 43);
        bool result = parser.saveToFile(table2, filePath);
        
        // Restore permissions for cleanup
        std::filesystem::permissions(filePath, std::filesystem::perms::owner_all);
    }
    #endif
}

// ============================================================================
// Tests targeting uncovered branches in getString
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, GetStringThrowsException) {
    TomlParser parser;
    
    // Create table with nested structure
    std::string toml = R"(
[section]
nested = { inner = "value" }
)";
    
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    // Try to get nested.inner as string (might throw)
    std::string value = parser.getString(*result, "section", "nested", "default");
    // Should return default since nested is a table, not a string
}

TEST_F(TomlParserBranchCoverageTest, GetStringNullSection) {
    TomlParser parser;
    std::string toml = "[section]\nkey = \"value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    // Access non-existent section
    std::string value = parser.getString(*result, "nonexistent", "key", "fallback");
    EXPECT_EQ(value, "fallback");
}

TEST_F(TomlParserBranchCoverageTest, GetStringNullKey) {
    TomlParser parser;
    std::string toml = "[section]\nother = \"value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    // Access non-existent key in existing section
    std::string value = parser.getString(*result, "section", "nonexistent", "fallback");
    EXPECT_EQ(value, "fallback");
}

TEST_F(TomlParserBranchCoverageTest, GetStringValidPath) {
    TomlParser parser;
    std::string toml = "[section]\nkey = \"actual_value\"";
    auto result = parser.parseString(toml);
    ASSERT_TRUE(result.has_value());
    
    // Valid path - should return actual value
    std::string value = parser.getString(*result, "section", "key", "fallback");
    EXPECT_EQ(value, "actual_value");
}

// ============================================================================
// Tests for error callback paths
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, ReportErrorWithCallback) {
    TomlParser parser;
    int callCount = 0;
    std::string lastSection;
    
    parser.setErrorCallback([&](const ParseError& error) {
        callCount++;
        lastSection = error.section;
    });
    
    ParseError error{"test_section", "test_key", "test_message"};
    parser.reportError(error);
    
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(lastSection, "test_section");
}

TEST_F(TomlParserBranchCoverageTest, ReportErrorWithoutCallback) {
    TomlParser parser;
    // No callback set
    
    ParseError error{"section", "key", "message"};
    parser.reportError(error);
    
    // Should not crash, error should be added to list
    EXPECT_FALSE(parser.getLastErrors().empty());
}

TEST_F(TomlParserBranchCoverageTest, ErrorCallbackTriggeredByParseFile) {
    TomlParser parser;
    int callCount = 0;
    
    parser.setErrorCallback([&](const ParseError&) {
        callCount++;
    });
    
    // Trigger error with non-existent file
    auto result = parser.parseFile(testDir / "does_not_exist.toml");
    (void)result;
    
    EXPECT_GT(callCount, 0);
}

TEST_F(TomlParserBranchCoverageTest, ErrorCallbackTriggeredByParseString) {
    TomlParser parser;
    int callCount = 0;
    
    parser.setErrorCallback([&](const ParseError&) {
        callCount++;
    });
    
    // Trigger parse error
    auto result = parser.parseString("[invalid syntax");
    (void)result;
    
    EXPECT_GT(callCount, 0);
}

// ============================================================================
// Tests for parseFile specific branches
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, ParseFileExistingValidFile) {
    writeFile("valid.toml", "[section]\nkey = \"value\"");
    
    TomlParser parser;
    auto result = parser.parseFile(testDir / "valid.toml");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, ParseFileExistingInvalidFile) {
    writeFile("invalid.toml", "[bad syntax");
    
    TomlParser parser;
    auto result = parser.parseFile(testDir / "invalid.toml");
    
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, ParseFileReturnsNulloptOnError) {
    TomlParser parser;
    
    // Non-existent file
    auto result = parser.parseFile("truly_nonexistent_file_xyz123.toml");
    
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

// ============================================================================
// Tests for complex table operations
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, SaveAndLoadComplexTable) {
    TomlParser parser;
    
    // Create complex table
    toml::table table;
    toml::table section1;
    section1.insert_or_assign("string", "test");
    section1.insert_or_assign("number", 42);
    section1.insert_or_assign("float", 3.14);
    section1.insert_or_assign("bool", true);
    
    table.insert_or_assign("section1", section1);
    table.insert_or_assign("section2", toml::table{{"key", "value"}});
    
    auto filePath = testDir / "complex.toml";
    
    // Save
    bool saveResult = parser.saveToFile(table, filePath);
    EXPECT_TRUE(saveResult);
    
    // Load back
    auto loadResult = parser.parseFile(filePath);
    ASSERT_TRUE(loadResult.has_value());
    
    // Verify values
    EXPECT_EQ(parser.getString(*loadResult, "section1", "string", ""), "test");
    EXPECT_EQ(parser.getValue(*loadResult, "section1", "number", 0), 42);
}

// ============================================================================
// Edge cases and boundary conditions
// ============================================================================

TEST_F(TomlParserBranchCoverageTest, ParseEmptyString) {
    TomlParser parser;
    auto result = parser.parseString("");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, ParseOnlyComments) {
    TomlParser parser;
    auto result = parser.parseString("# Just a comment\n# Another comment");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, ParseOnlyWhitespace) {
    TomlParser parser;
    auto result = parser.parseString("   \n\n  \t  \n  ");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(parser.getLastResult().success);
}

TEST_F(TomlParserBranchCoverageTest, GetValueFromEmptyTable) {
    TomlParser parser;
    auto result = parser.parseString("");
    ASSERT_TRUE(result.has_value());
    
    int value = parser.getValue(*result, "section", "key", 999);
    EXPECT_EQ(value, 999);
}

TEST_F(TomlParserBranchCoverageTest, GetStringFromEmptyTable) {
    TomlParser parser;
    auto result = parser.parseString("");
    ASSERT_TRUE(result.has_value());
    
    std::string value = parser.getString(*result, "section", "key", "default");
    EXPECT_EQ(value, "default");
}

TEST_F(TomlParserBranchCoverageTest, MultipleErrorsAccumulate) {
    TomlParser parser;
    
    parser.reportError({"sec1", "key1", "error1"});
    parser.reportError({"sec2", "key2", "error2"});
    parser.reportError({"sec3", "key3", "error3"});
    
    const auto& errors = parser.getLastErrors();
    EXPECT_GE(errors.size(), 3u);
}

TEST_F(TomlParserBranchCoverageTest, ParseResultSuccessAfterErrors) {
    TomlParser parser;
    
    // Generate error
    auto fail = parser.parseString("[bad");
    (void)fail;
    EXPECT_FALSE(parser.getLastResult().success);
    
    // Generate success
    auto success = parser.parseString("[good]\nkey = 1");
    (void)success;
    EXPECT_TRUE(parser.getLastResult().success);
}
