#include <gtest/gtest.h>

#include <filesystem>

#include "../../../src/games/rtype/shared/Config/SaveManager/Operations/FileOperations.hpp"

using namespace rtype::game::config;

TEST(FileOperationsTest, WriteAndReadFile) {
    auto tmp = std::filesystem::temp_directory_path() / "rtype_fileops_test";
    std::filesystem::create_directories(tmp);
    auto filepath = tmp / "save.dat";

    std::vector<uint8_t> data = {1,2,3,4,5};
    auto err = FileOperations::writeToFile(filepath, data);
    EXPECT_FALSE(err.has_value());
    EXPECT_TRUE(std::filesystem::exists(filepath));

    auto [maybeData, readErr] = FileOperations::readFromFile(filepath);
    EXPECT_FALSE(readErr.has_value());
    ASSERT_TRUE(maybeData.has_value());
    EXPECT_EQ(*maybeData, data);

    std::filesystem::remove_all(tmp);
}

TEST(FileOperationsTest, ReadFile_NotFound) {
    auto filepath = std::filesystem::temp_directory_path() / "nonexistent_save.xyz";
    auto [maybeData, err] = FileOperations::readFromFile(filepath);
    EXPECT_FALSE(maybeData.has_value());
    EXPECT_TRUE(err.has_value());
}

TEST(FileOperationsTest, DeleteAndCopyFile) {
    auto tmp = std::filesystem::temp_directory_path() / "rtype_fileops_test2";
    std::filesystem::create_directories(tmp);
    auto src = tmp / "src.bin";
    auto dst = tmp / "dst.bin";

    std::vector<uint8_t> data = {9,8,7};
    FileOperations::writeToFile(src, data);
    EXPECT_TRUE(std::filesystem::exists(src));

    auto copyErr = FileOperations::copyFile(src, dst);
    EXPECT_FALSE(copyErr.has_value());
    EXPECT_TRUE(std::filesystem::exists(dst));

    auto delErr = FileOperations::deleteFile(src);
    EXPECT_FALSE(delErr.has_value());
    EXPECT_FALSE(std::filesystem::exists(src));

    std::filesystem::remove_all(tmp);
}
