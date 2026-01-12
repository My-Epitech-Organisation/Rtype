/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DevConsole - In-game developer console
*/

#ifndef SRC_CLIENT_DEVCONSOLE_DEVCONSOLE_HPP_
#define SRC_CLIENT_DEVCONSOLE_DEVCONSOLE_HPP_

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "rtype/display/IDisplay.hpp"

namespace rtype::client {

// Forward declaration
class NetworkClient;

/**
 * @brief In-game developer console for debugging and runtime configuration.
 *
 * Provides a command-line interface overlay that can be toggled with the
 * tilde (~) key. Supports custom commands, console variables (CVars),
 * and displays output history.
 */
class DevConsole {
   public:
    static constexpr std::size_t kMaxHistoryLines = 50;
    static constexpr std::size_t kMaxInputLength = 256;
    static constexpr float kConsoleHeightRatio = 0.4f;

    // Visual constants
    static constexpr unsigned int kFontSize = 16;
    static constexpr float kInputLineHeight = 30.f;
    static constexpr float kTextPadding = 10.f;
    static constexpr float kCursorBlinkRate = 0.5f;

    /**
     * @brief Construct a new DevConsole.
     * @param display The display interface for rendering
     * @param networkClient Optional network client for server commands
     */
    explicit DevConsole(
        std::shared_ptr<rtype::display::IDisplay> display,
        std::shared_ptr<NetworkClient> networkClient = nullptr);

    ~DevConsole() = default;
    DevConsole(const DevConsole&) = delete;
    DevConsole& operator=(const DevConsole&) = delete;
    DevConsole(DevConsole&&) = default;
    DevConsole& operator=(DevConsole&&) = default;

    /**
     * @brief Toggle console visibility.
     */
    void toggle();

    /**
     * @brief Check if console is visible.
     * @return true if visible
     */
    [[nodiscard]] bool isVisible() const { return visible_; }

    /**
     * @brief Handle input events.
     * @param event The display event
     * @return true if event was consumed by the console
     */
    bool handleEvent(const rtype::display::Event& event);

    /**
     * @brief Update console state (cursor blink, etc).
     * @param dt Delta time in seconds
     */
    void update(float dt);

    /**
     * @brief Render the console overlay.
     */
    void render();

    /**
     * @brief Register a new command.
     * @param name Command name (case-insensitive)
     * @param description Help text for the command
     * @param handler Function to execute (returns output string)
     */
    void registerCommand(
        const std::string& name, const std::string& description,
        std::function<std::string(const std::vector<std::string>&)> handler);

    /**
     * @brief Execute a command string.
     * @param commandLine Full command with arguments
     */
    void execute(const std::string& commandLine);

    /**
     * @brief Print a message to the console output.
     * @param message Message to print
     */
    void print(const std::string& message);

    /**
     * @brief Print an error message (displayed in red).
     * @param message Error message to print
     */
    void printError(const std::string& message);

    /**
     * @brief Set a console variable.
     * @param name CVar name
     * @param value Value as string
     */
    void setCvar(const std::string& name, const std::string& value);

    /**
     * @brief Get a console variable value.
     * @param name CVar name
     * @return Value as string, empty if not found
     */
    [[nodiscard]] std::string getCvar(const std::string& name) const;

    /**
     * @brief Get all console variables.
     * @return Map of all CVars
     */
    [[nodiscard]] const std::map<std::string, std::string>& getAllCvars() const {
        return cvars_;
    }

    /**
     * @brief Set the network client for server commands.
     * @param networkClient The network client
     */
    void setNetworkClient(std::shared_ptr<NetworkClient> networkClient) {
        networkClient_ = std::move(networkClient);
    }

   private:
    struct OutputLine {
        std::string text;
        bool isError{false};
    };

    void executeCurrentInput();
    void navigateHistory(int direction);
    std::vector<std::string> parseArgs(const std::string& input);
    void registerDefaultCommands();

    void renderBackground();
    void renderOutput();
    void renderInputLine();

    bool handleKeyPressed(const rtype::display::Event& event);
    bool handleTextEntered(std::uint32_t unicode);

    std::shared_ptr<rtype::display::IDisplay> display_;
    std::shared_ptr<NetworkClient> networkClient_;
    bool visible_{false};

    // Input state
    std::string inputBuffer_;
    std::size_t cursorPos_{0};
    float cursorBlinkTimer_{0.f};
    bool cursorVisible_{true};

    // History
    std::deque<OutputLine> outputHistory_;
    std::deque<std::string> commandHistory_;
    int historyIndex_{-1};
    std::size_t scrollOffset_{0};

    // Commands: name -> (description, handler)
    std::map<std::string,
             std::pair<std::string,
                       std::function<std::string(const std::vector<std::string>&)>>>
        commands_;

    // Console variables
    std::map<std::string, std::string> cvars_;
};

}  // namespace rtype::client

#endif  // SRC_CLIENT_DEVCONSOLE_DEVCONSOLE_HPP_
