#include <gtest/gtest.h>

#include <filesystem>

#include "../../../lib/common/src/Config/TomlParser.hpp"

using namespace rtype::config;

TEST(TomlParserTest, ParseString_ValidAndGetters) {
    TomlParser parser;
    const std::string content = R"(
[server]
host = "127.0.0.1"
port = 4242
)";

    auto table = parser.parseString(content);
    ASSERT_TRUE(table.has_value());
    EXPECT_TRUE(parser.getLastResult());

    EXPECT_EQ(parser.getString(*table, "server", "host", ""), "127.0.0.1");
    EXPECT_EQ(parser.getValue<int>(*table, "server", "port", 0), 4242);
}

TEST(TomlParserTest, ParseString_InvalidReportsError) {
    TomlParser parser;
    const std::string bad = "this is not valid toml =\"\"";

    auto table = parser.parseString(bad);
    EXPECT_FALSE(table.has_value());
    EXPECT_FALSE(parser.getLastResult().success);
    EXPECT_FALSE(parser.getLastErrors().empty());
}

TEST(TomlParserTest, ParseFile_NotFound) {
    TomlParser parser;
    auto table = parser.parseFile("/nonexistent/path/definitely_not_present.toml");
    EXPECT_FALSE(table.has_value());
    EXPECT_FALSE(parser.getLastResult().errorMessage.empty());
}

TEST(TomlParserTest, SaveToFile_SuccessAndFailure) {
    TomlParser parser;

    toml::table table;
    table.insert("key", "value");

    auto tmp = std::filesystem::temp_directory_path() / "rtype_toml_test";
    std::filesystem::create_directories(tmp);
    auto filepath = tmp / "out.toml";

    // Success
    EXPECT_TRUE(parser.saveToFile(table, filepath));
    EXPECT_TRUE(std::filesystem::exists(filepath));

    // Failure: make directory read-only and try to write
    std::filesystem::permissions(tmp, std::filesystem::perms::owner_all, std::filesystem::perm_options::remove);
    auto badpath = tmp / "cannot_write.toml";
    EXPECT_FALSE(parser.saveToFile(table, badpath));

    // Restore perms and cleanup
    std::filesystem::permissions(tmp, std::filesystem::perms::owner_all, std::filesystem::perm_options::add);
    std::filesystem::remove_all(tmp);
}
