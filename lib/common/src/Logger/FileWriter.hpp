/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** FileWriter - File output handler for logger
*/

#ifndef SRC_COMMON_LOGGER_FILEWRITER_HPP_
#define SRC_COMMON_LOGGER_FILEWRITER_HPP_

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>

namespace rtype {

/**
 * @brief Thread-safe file writer for logging
 *
 * Handles file operations for the logger with proper RAII resource management.
 * All public methods are protected by a mutex for thread safety.
 */
class FileWriter {
   public:
    FileWriter() = default;
    ~FileWriter() {
        std::lock_guard<std::mutex> lock(_mutex);
        closeInternal();
    }

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
        std::lock_guard<std::mutex> lock(_mutex);
        closeInternal();
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
        std::lock_guard<std::mutex> lock(_mutex);
        closeInternal();
    }

    /**
     * @brief Check if file is open
     * @return true if file is open and ready for writing
     */
    [[nodiscard]] bool isOpen() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return isOpenInternal();
    }

    /**
     * @brief Write a message to the file
     * @param message The message to write
     */
    void write(const std::string& message) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (isOpenInternal()) {
            *_fileStream << message << '\n';
            _fileStream->flush();
        }
    }

    /**
     * @brief Get the current log file path
     * @return Path to the current log file
     */
    [[nodiscard]] std::filesystem::path getFilePath() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _filePath;
    }

   private:
    /**
     * @brief Internal close without locking (caller must hold mutex)
     */
    void closeInternal() {
        if (_fileStream) {
            _fileStream->flush();
            _fileStream->close();
#ifdef _WIN32
            // On Windows, give the OS time to flush buffers to disk
            // This prevents read-after-write issues in tests
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
#endif
            _fileStream.reset();
        }
    }

    /**
     * @brief Internal isOpen check without locking (caller must hold mutex)
     */
    [[nodiscard]] bool isOpenInternal() const noexcept {
        return _fileStream && _fileStream->is_open();
    }

    mutable std::mutex _mutex;
    std::unique_ptr<std::ofstream> _fileStream;
    std::filesystem::path _filePath;
};

}  // namespace rtype

#endif  // SRC_COMMON_LOGGER_FILEWRITER_HPP_
