/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** FileOperations Branch Coverage Tests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "Config/SaveManager/Operations/FileOperations.hpp"

namespace rtype::game::config {

class FileOperationsTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "fileops_test";
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(testDir, ec);
    }

    std::filesystem::path testDir;
};

// =============================================================================
// writeToFile Tests - Branch coverage
// =============================================================================

TEST_F(FileOperationsTest, WriteToFileSuccessSimplePath) {
    auto filepath = testDir / "simple_file.bin";
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(FileOperationsTest, WriteToFileSuccessEmptyData) {
    auto filepath = testDir / "empty_file.bin";
    std::vector<uint8_t> data;
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(FileOperationsTest, WriteToFileLargeData) {
    auto filepath = testDir / "large_file.bin";
    std::vector<uint8_t> data(1024 * 1024);  // 1MB
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>(i % 256);
    }
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());

    // Verify file size
    auto fileSize = std::filesystem::file_size(filepath);
    EXPECT_EQ(fileSize, data.size());
}

TEST_F(FileOperationsTest, WriteToFileCreatesNestedDirectories) {
    auto filepath = testDir / "nested" / "deep" / "path" / "file.bin";
    std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC};
    std::string error;

    // Directory doesn't exist yet
    EXPECT_FALSE(std::filesystem::exists(filepath.parent_path()));

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(filepath));
}

TEST_F(FileOperationsTest, WriteToFileNoParentPath) {
    // File directly in current directory (no parent path logic)
    auto currentDir = std::filesystem::current_path();
    auto filepath = currentDir / "temp_test_file_no_parent.bin";
    std::vector<uint8_t> data = {0x11, 0x22, 0x33};
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    std::filesystem::remove(filepath);  // Cleanup
}

TEST_F(FileOperationsTest, WriteToFileOverwritesExisting) {
    auto filepath = testDir / "overwrite_file.bin";
    std::vector<uint8_t> oldData = {0x01, 0x02, 0x03};
    std::vector<uint8_t> newData = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB};
    std::string error;

    // First write
    (void)FileOperations::writeToFile(filepath, oldData, error);
    EXPECT_EQ(std::filesystem::file_size(filepath), 3);

    // Second write should overwrite
    bool result = FileOperations::writeToFile(filepath, newData, error);

    EXPECT_TRUE(result);
    EXPECT_EQ(std::filesystem::file_size(filepath), 5);
}

TEST_F(FileOperationsTest, WriteToFileBinaryData) {
    auto filepath = testDir / "binary_test.bin";
    std::vector<uint8_t> data;
    // Include all byte values 0-255
    for (int i = 0; i < 256; ++i) {
        data.push_back(static_cast<uint8_t>(i));
    }
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);

    // Read back and verify
    std::ifstream inFile(filepath, std::ios::binary);
    std::vector<uint8_t> readData((std::istreambuf_iterator<char>(inFile)),
                                  std::istreambuf_iterator<char>());

    EXPECT_EQ(readData.size(), 256);
    EXPECT_EQ(readData, data);
}

TEST_F(FileOperationsTest, WriteToFileExistingDirectoryNoCreate) {
    auto filepath = testDir / "existing_dir_file.bin";
    std::vector<uint8_t> data = {0x01};
    std::string error;

    // testDir already exists, so no directory creation needed
    bool result = FileOperations::writeToFile(filepath, data, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
}

// =============================================================================
// readFromFile Tests - Branch coverage
// =============================================================================

TEST_F(FileOperationsTest, ReadFromFileSuccess) {
    auto filepath = testDir / "read_test.bin";
    std::vector<uint8_t> originalData = {0x10, 0x20, 0x30, 0x40, 0x50};

    // Write file first
    std::ofstream outFile(filepath, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(originalData.data()),
                  static_cast<std::streamsize>(originalData.size()));
    outFile.close();

    std::string error;
    auto result = FileOperations::readFromFile(filepath, error);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(error.empty());
    EXPECT_EQ(*result, originalData);
}

TEST_F(FileOperationsTest, ReadFromFileNotFound) {
    auto filepath = testDir / "nonexistent_file.bin";
    std::string error;

    auto result = FileOperations::readFromFile(filepath, error);

    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(error.empty());
    EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(FileOperationsTest, ReadFromFileEmptyFile) {
    auto filepath = testDir / "empty_read_test.bin";

    // Create empty file
    std::ofstream outFile(filepath, std::ios::binary);
    outFile.close();

    std::string error;
    auto result = FileOperations::readFromFile(filepath, error);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(FileOperationsTest, ReadFromFileLargeFile) {
    auto filepath = testDir / "large_read_test.bin";
    std::vector<uint8_t> originalData(512 * 1024);  // 512KB
    for (size_t i = 0; i < originalData.size(); ++i) {
        originalData[i] = static_cast<uint8_t>((i * 7) % 256);
    }

    // Write file first
    std::ofstream outFile(filepath, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(originalData.data()),
                  static_cast<std::streamsize>(originalData.size()));
    outFile.close();

    std::string error;
    auto result = FileOperations::readFromFile(filepath, error);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), originalData.size());
    EXPECT_EQ(*result, originalData);
}

TEST_F(FileOperationsTest, ReadFromFileBinaryContent) {
    auto filepath = testDir / "binary_read_test.bin";
    std::vector<uint8_t> originalData;
    for (int i = 0; i < 256; ++i) {
        originalData.push_back(static_cast<uint8_t>(i));
    }

    std::ofstream outFile(filepath, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(originalData.data()),
                  static_cast<std::streamsize>(originalData.size()));
    outFile.close();

    std::string error;
    auto result = FileOperations::readFromFile(filepath, error);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(*result, originalData);
}

// =============================================================================
// deleteFile Tests - Branch coverage
// =============================================================================

TEST_F(FileOperationsTest, DeleteFileSuccess) {
    auto filepath = testDir / "delete_me.bin";

    // Create file first
    std::ofstream outFile(filepath, std::ios::binary);
    outFile << "test";
    outFile.close();

    EXPECT_TRUE(std::filesystem::exists(filepath));

    std::string error;
    bool result = FileOperations::deleteFile(filepath, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
    EXPECT_FALSE(std::filesystem::exists(filepath));
}

TEST_F(FileOperationsTest, DeleteFileNotFound) {
    auto filepath = testDir / "nonexistent_delete.bin";
    std::string error;

    // File doesn't exist - should return false
    bool result = FileOperations::deleteFile(filepath, error);

    EXPECT_FALSE(result);
}

TEST_F(FileOperationsTest, DeleteFileEmptyFile) {
    auto filepath = testDir / "empty_delete.bin";

    // Create empty file
    std::ofstream outFile(filepath, std::ios::binary);
    outFile.close();

    std::string error;
    bool result = FileOperations::deleteFile(filepath, error);

    EXPECT_TRUE(result);
    EXPECT_FALSE(std::filesystem::exists(filepath));
}

// =============================================================================
// copyFile Tests - Branch coverage
// =============================================================================

TEST_F(FileOperationsTest, CopyFileSuccess) {
    auto source = testDir / "copy_source.bin";
    auto destination = testDir / "copy_dest.bin";
    std::vector<uint8_t> data = {0xCA, 0xFE, 0xBA, 0xBE};

    // Create source file
    std::ofstream outFile(source, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    outFile.close();

    std::string error;
    bool result = FileOperations::copyFile(source, destination, error);

    EXPECT_TRUE(result);
    EXPECT_TRUE(error.empty());
    EXPECT_TRUE(std::filesystem::exists(destination));

    // Verify content
    std::ifstream inFile(destination, std::ios::binary);
    std::vector<uint8_t> readData((std::istreambuf_iterator<char>(inFile)),
                                  std::istreambuf_iterator<char>());
    EXPECT_EQ(readData, data);
}

TEST_F(FileOperationsTest, CopyFileSourceNotFound) {
    auto source = testDir / "nonexistent_source.bin";
    auto destination = testDir / "copy_dest2.bin";
    std::string error;

    bool result = FileOperations::copyFile(source, destination, error);

    EXPECT_FALSE(result);
    EXPECT_FALSE(error.empty());
    EXPECT_TRUE(error.find("copy") != std::string::npos ||
                error.find("Failed") != std::string::npos);
}

TEST_F(FileOperationsTest, CopyFileOverwriteExisting) {
    auto source = testDir / "copy_source2.bin";
    auto destination = testDir / "copy_dest_existing.bin";
    std::vector<uint8_t> sourceData = {0x11, 0x22, 0x33, 0x44};
    std::vector<uint8_t> destData = {0xFF, 0xEE};

    // Create source file
    std::ofstream sourceFile(source, std::ios::binary);
    sourceFile.write(reinterpret_cast<const char*>(sourceData.data()),
                     static_cast<std::streamsize>(sourceData.size()));
    sourceFile.close();

    // Create destination file with different content
    std::ofstream destFile(destination, std::ios::binary);
    destFile.write(reinterpret_cast<const char*>(destData.data()),
                   static_cast<std::streamsize>(destData.size()));
    destFile.close();

    std::string error;
    bool result = FileOperations::copyFile(source, destination, error);

    EXPECT_TRUE(result);

    // Verify destination now has source content
    std::ifstream inFile(destination, std::ios::binary);
    std::vector<uint8_t> readData((std::istreambuf_iterator<char>(inFile)),
                                  std::istreambuf_iterator<char>());
    EXPECT_EQ(readData, sourceData);
}

TEST_F(FileOperationsTest, CopyFileLargeFile) {
    auto source = testDir / "large_copy_source.bin";
    auto destination = testDir / "large_copy_dest.bin";
    std::vector<uint8_t> data(256 * 1024);  // 256KB
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>((i * 13) % 256);
    }

    std::ofstream outFile(source, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    outFile.close();

    std::string error;
    bool result = FileOperations::copyFile(source, destination, error);

    EXPECT_TRUE(result);
    EXPECT_EQ(std::filesystem::file_size(destination), data.size());
}

TEST_F(FileOperationsTest, CopyFileEmptyFile) {
    auto source = testDir / "empty_copy_source.bin";
    auto destination = testDir / "empty_copy_dest.bin";

    // Create empty source file
    std::ofstream outFile(source, std::ios::binary);
    outFile.close();

    std::string error;
    bool result = FileOperations::copyFile(source, destination, error);

    EXPECT_TRUE(result);
    EXPECT_EQ(std::filesystem::file_size(destination), 0);
}

// =============================================================================
// exists Tests - Branch coverage
// =============================================================================

TEST_F(FileOperationsTest, ExistsReturnsTrueForExistingFile) {
    auto filepath = testDir / "exists_test.bin";

    // Create file
    std::ofstream outFile(filepath, std::ios::binary);
    outFile << "test";
    outFile.close();

    EXPECT_TRUE(FileOperations::exists(filepath));
}

TEST_F(FileOperationsTest, ExistsReturnsFalseForNonexistent) {
    auto filepath = testDir / "nonexistent_exists_test.bin";

    EXPECT_FALSE(FileOperations::exists(filepath));
}

TEST_F(FileOperationsTest, ExistsReturnsTrueForDirectory) {
    EXPECT_TRUE(FileOperations::exists(testDir));
}

TEST_F(FileOperationsTest, ExistsReturnsFalseForNonexistentDirectory) {
    auto nonexistentDir = testDir / "nonexistent_subdir";

    EXPECT_FALSE(FileOperations::exists(nonexistentDir));
}

TEST_F(FileOperationsTest, ExistsEmptyPath) {
    std::filesystem::path emptyPath;

    EXPECT_FALSE(FileOperations::exists(emptyPath));
}

// =============================================================================
// Round-trip Tests - Write then Read
// =============================================================================

TEST_F(FileOperationsTest, RoundTripSmallData) {
    auto filepath = testDir / "roundtrip_small.bin";
    std::vector<uint8_t> originalData = {0x01, 0x02, 0x03, 0x04, 0x05};
    std::string error;

    bool writeResult = FileOperations::writeToFile(filepath, originalData, error);
    EXPECT_TRUE(writeResult);

    auto readResult = FileOperations::readFromFile(filepath, error);
    EXPECT_TRUE(readResult.has_value());
    EXPECT_EQ(*readResult, originalData);
}

TEST_F(FileOperationsTest, RoundTripLargeData) {
    auto filepath = testDir / "roundtrip_large.bin";
    std::vector<uint8_t> originalData(1024 * 100);  // 100KB
    for (size_t i = 0; i < originalData.size(); ++i) {
        originalData[i] = static_cast<uint8_t>((i * 17 + 3) % 256);
    }
    std::string error;

    bool writeResult = FileOperations::writeToFile(filepath, originalData, error);
    EXPECT_TRUE(writeResult);

    auto readResult = FileOperations::readFromFile(filepath, error);
    EXPECT_TRUE(readResult.has_value());
    EXPECT_EQ(*readResult, originalData);
}

TEST_F(FileOperationsTest, RoundTripEmptyData) {
    auto filepath = testDir / "roundtrip_empty.bin";
    std::vector<uint8_t> originalData;
    std::string error;

    bool writeResult = FileOperations::writeToFile(filepath, originalData, error);
    EXPECT_TRUE(writeResult);

    auto readResult = FileOperations::readFromFile(filepath, error);
    EXPECT_TRUE(readResult.has_value());
    EXPECT_TRUE(readResult->empty());
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(FileOperationsTest, WriteReadDeleteSequence) {
    auto filepath = testDir / "write_read_delete.bin";
    std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
    std::string error;

    // Write
    EXPECT_TRUE(FileOperations::writeToFile(filepath, data, error));
    EXPECT_TRUE(FileOperations::exists(filepath));

    // Read
    auto readResult = FileOperations::readFromFile(filepath, error);
    EXPECT_TRUE(readResult.has_value());
    EXPECT_EQ(*readResult, data);

    // Delete
    EXPECT_TRUE(FileOperations::deleteFile(filepath, error));
    EXPECT_FALSE(FileOperations::exists(filepath));

    // Read after delete should fail
    auto readAfterDelete = FileOperations::readFromFile(filepath, error);
    EXPECT_FALSE(readAfterDelete.has_value());
}

TEST_F(FileOperationsTest, CopyThenModifyOriginal) {
    auto source = testDir / "original.bin";
    auto copy = testDir / "copy.bin";
    std::vector<uint8_t> originalData = {0x01, 0x02, 0x03};
    std::vector<uint8_t> modifiedData = {0xFF, 0xFE, 0xFD, 0xFC};
    std::string error;

    // Write original
    (void)FileOperations::writeToFile(source, originalData, error);

    // Copy
    (void)FileOperations::copyFile(source, copy, error);

    // Modify original
    (void)FileOperations::writeToFile(source, modifiedData, error);

    // Verify copy still has original data
    auto copyResult = FileOperations::readFromFile(copy, error);
    EXPECT_TRUE(copyResult.has_value());
    EXPECT_EQ(*copyResult, originalData);

    // Verify original has new data
    auto sourceResult = FileOperations::readFromFile(source, error);
    EXPECT_TRUE(sourceResult.has_value());
    EXPECT_EQ(*sourceResult, modifiedData);
}

TEST_F(FileOperationsTest, MultipleWritesToSameFile) {
    auto filepath = testDir / "multiple_writes.bin";
    std::string error;

    for (int i = 0; i < 10; ++i) {
        std::vector<uint8_t> data(i + 1, static_cast<uint8_t>(i));
        EXPECT_TRUE(FileOperations::writeToFile(filepath, data, error));

        auto readResult = FileOperations::readFromFile(filepath, error);
        EXPECT_TRUE(readResult.has_value());
        EXPECT_EQ(readResult->size(), static_cast<size_t>(i + 1));
    }
}

TEST_F(FileOperationsTest, SpecialCharactersInPath) {
    auto filepath = testDir / "file with spaces.bin";
    std::vector<uint8_t> data = {0x01, 0x02};
    std::string error;

    bool result = FileOperations::writeToFile(filepath, data, error);
    EXPECT_TRUE(result);
    EXPECT_TRUE(FileOperations::exists(filepath));
}

}  // namespace rtype::game::config
