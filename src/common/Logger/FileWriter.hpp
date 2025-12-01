/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** FileWriter - File output handler for logger
*/

#ifndef SRC_COMMON_LOGGER_FILEWRITER_HPP_
#define SRC_COMMON_LOGGER_FILEWRITER_HPP_

#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>

namespace rtype {

/**
 * @brief Thread-safe file writer for logging
 *
 * Handles file operations for the logger with proper RAII resource management.
 */
class FileWriter {
   public:
    FileWriter() = default;
    ~FileWriter() { close(); }

    FileWriter(const FileWriter&) = delete;
    FileWriter& operator=(const FileWriter&) = delete;
    FileWriter(FileWriter&&) = delete;
    FileWriter& operator=(FileWriter&&) = delete;

    /**
     * @brief Open a file for logging
     * @param filepath Path to the log file
     * @param append If true, append to existing file; otherwise overwrite
     * @return true if file was opened successfully
     */
    bool open(const std::filesystem::path& filepath, bool append = true) {
        close();
        auto mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }
        _fileStream = std::make_unique<std::ofstream>(filepath, mode);
        if (!_fileStream->is_open()) {
            _fileStream.reset();
            return false;
        }
        _filePath = filepath;
        return true;
    }

    /**
     * @brief Close the log file
     */
    void close() {
        if (_fileStream) {
            _fileStream->close();
            _fileStream.reset();
        }
    }

    /**
     * @brief Check if file is open
     * @return true if file is open and ready for writing
     */
    [[nodiscard]] bool isOpen() const noexcept {
        return _fileStream && _fileStream->is_open();
    }

    /**
     * @brief Write a message to the file
     * @param message The message to write
     */
    void write(const std::string& message) {
        if (isOpen()) {
            *_fileStream << message << '\n';
            _fileStream->flush();
        }
    }

    /**
     * @brief Get the current log file path
     * @return Path to the current log file
     */
    [[nodiscard]] const std::filesystem::path& getFilePath() const noexcept {
        return _filePath;
    }

   private:
    std::unique_ptr<std::ofstream> _fileStream;
    std::filesystem::path _filePath;
};

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_FILEWRITER_HPP_
