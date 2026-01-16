/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DevConsole - Implementation
*/

#include "DevConsole.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <sstream>

#ifdef __linux__
#include <fstream>
#include <string>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

#include <rtype/ecs.hpp>

#include "GameAction.hpp"
#include "Graphic/Accessibility.hpp"
#include "Graphic/AudioLib/AudioLib.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/PowerUpComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "network/NetworkClient.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::client {

namespace {

#ifdef __linux__
// Read process CPU times from /proc/self/stat
// Returns (utime, stime) in clock ticks
std::pair<unsigned long, unsigned long> readProcStat() {
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open()) {
        return {0, 0};
    }

    std::string line;
    std::getline(stat, line);

    // Find the end of (comm) - the process name can contain spaces
    auto commEnd = line.rfind(')');
    if (commEnd == std::string::npos || commEnd + 2 >= line.size()) {
        return {0, 0};
    }

    // Parse fields after (comm)
    std::istringstream iss(line.substr(commEnd + 2));
    std::string field;

    // Skip fields 3-13 to get to utime (field 14)
    for (int i = 3; i <= 13; ++i) {
        iss >> field;
    }

    unsigned long utime = 0;
    unsigned long stime = 0;
    iss >> utime >> stime;  // Fields 14 and 15

    return {utime, stime};
}
#endif

// System metrics for resource monitoring
struct SystemMetrics {
    float cpuPercent{0.0f};
    std::size_t memoryMB{0};
    bool cpuAvailable{false};
    bool memAvailable{false};
};

SystemMetrics getSystemMetrics(float cachedCpuPercent) {
    SystemMetrics m;
#ifdef __linux__
    std::ifstream status("/proc/self/status");
    std::string line;
    while (std::getline(status, line)) {
        if (line.rfind("VmRSS:", 0) == 0) {
            // "VmRSS:     12345 kB"
            try {
                std::size_t kb = std::stoul(line.substr(6));
                m.memoryMB = kb / 1024;
                m.memAvailable = true;
            } catch (...) {
                // Parsing failed
            }
            break;
        }
    }
    // Use pre-calculated CPU percentage from sampling
    m.cpuPercent = cachedCpuPercent;
    m.cpuAvailable = true;
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        m.memoryMB = pmc.WorkingSetSize / (1024 * 1024);
        m.memAvailable = true;
    }
    m.cpuAvailable = false;  // Complex on Windows
    (void)cachedCpuPercent;  // Unused on Windows
#else
    (void)cachedCpuPercent;  // Unused on other platforms
#endif
    return m;
}

constexpr rtype::display::Color kConsoleBgColor{0, 0, 0, 200};
constexpr rtype::display::Color kConsoleTextColor{0, 255, 0, 255};
constexpr rtype::display::Color kConsoleErrorColor{255, 80, 80, 255};
constexpr rtype::display::Color kConsoleInputBgColor{30, 30, 30, 255};
constexpr rtype::display::Color kConsoleCursorColor{255, 255, 255, 255};
constexpr rtype::display::Color kConsolePromptColor{100, 200, 255, 255};

constexpr std::string_view kFontName = "main_font";
constexpr std::string_view kPrompt = "> ";

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &time);
#else
    localtime_r(&time, &tm_buf);
#endif
    std::ostringstream oss;
    oss << "[" << std::setfill('0') << std::setw(2) << tm_buf.tm_hour << ":"
        << std::setw(2) << tm_buf.tm_min << ":" << std::setw(2) << tm_buf.tm_sec
        << "] ";
    return oss.str();
}
}  // namespace

DevConsole::DevConsole(std::shared_ptr<rtype::display::IDisplay> display,
                       std::shared_ptr<NetworkClient> networkClient,
                       std::shared_ptr<ECS::Registry> registry,
                       std::shared_ptr<AudioLib> audioLib, float* deltaTimePtr,
                       std::shared_ptr<KeyboardActions> keybinds)
    : display_(std::move(display)),
      networkClient_(std::move(networkClient)),
      registry_(std::move(registry)),
      audioLib_(std::move(audioLib)),
      deltaTimePtr_(deltaTimePtr),
      keybinds_(std::move(keybinds)) {
    registerDefaultCommands();

    cvars_["cl_show_fps"] = "0";
    cvars_["cl_show_ping"] = "0";
    cvars_["cl_show_hitboxes"] = "0";
    cvars_["cl_mute_audio"] = "0";
    cvars_["cl_show_entities"] = "0";
    cvars_["net_graph"] = "0";
    cvars_["god_mode"] = "0";
    cvars_["cl_show_position"] = "0";
    cvars_["cl_show_resources"] = "0";
    cvars_["cl_show_lagometer"] = "0";
    cvars_["cl_show_proc"] = "0";

    print(
        "Developer Console initialized. Press F1 to toggle. Type 'help' for "
        "commands.");
}

void DevConsole::toggle() {
    visible_ = !visible_;
    if (visible_) {
        LOG_DEBUG("[DevConsole] Console OPENED");
        inputBuffer_.clear();
        cursorPos_ = 0;
        historyIndex_ = -1;
    } else {
        LOG_DEBUG("[DevConsole] Console CLOSED");
    }
}

bool DevConsole::handleEvent(const rtype::display::Event& event) {
    if (event.type == rtype::display::EventType::KeyPressed) {
        if (keybinds_) {
            auto toggleKey =
                keybinds_->getKeyBinding(GameAction::TOGGLE_CONSOLE);
            if (toggleKey.has_value() && event.key.code == toggleKey.value()) {
                toggle();
                return true;
            }
        }
        if (event.key.code == rtype::display::Key::Tilde) {
            toggle();
            return true;
        }
    }

    if (!visible_) {
        return false;
    }

    if (event.type == rtype::display::EventType::KeyPressed) {
        return handleKeyPressed(event);
    }

    if (event.type == rtype::display::EventType::TextEntered) {
        return handleTextEntered(event.text.unicode);
    }

    return true;
}

bool DevConsole::handleKeyPressed(const rtype::display::Event& event) {
    switch (event.key.code) {
        case rtype::display::Key::Return:
            executeCurrentInput();
            return true;

        case rtype::display::Key::BackSpace:
            if (cursorPos_ > 0 && !inputBuffer_.empty()) {
                inputBuffer_.erase(cursorPos_ - 1, 1);
                cursorPos_--;
            }
            return true;

        case rtype::display::Key::Delete:
            if (cursorPos_ < inputBuffer_.length()) {
                inputBuffer_.erase(cursorPos_, 1);
            }
            return true;

        case rtype::display::Key::Left:
            if (cursorPos_ > 0) {
                cursorPos_--;
            }
            return true;

        case rtype::display::Key::Right:
            if (cursorPos_ < inputBuffer_.length()) {
                cursorPos_++;
            }
            return true;

        case rtype::display::Key::Up:
            navigateHistory(-1);
            return true;

        case rtype::display::Key::Down:
            navigateHistory(1);
            return true;

        case rtype::display::Key::Home:
            cursorPos_ = 0;
            return true;

        case rtype::display::Key::End:
            cursorPos_ = inputBuffer_.length();
            return true;

        case rtype::display::Key::PageUp:
            if (scrollOffset_ + 5 < outputHistory_.size()) {
                scrollOffset_ += 5;
            } else {
                scrollOffset_ =
                    outputHistory_.size() > 0 ? outputHistory_.size() - 1 : 0;
            }
            return true;

        case rtype::display::Key::PageDown:
            if (scrollOffset_ >= 5) {
                scrollOffset_ -= 5;
            } else {
                scrollOffset_ = 0;
            }
            return true;

        case rtype::display::Key::Escape:
            toggle();
            return true;

        default:
            return true;
    }
}

bool DevConsole::handleTextEntered(std::uint32_t unicode) {
    if (unicode < 32 || unicode == 127 || unicode == '~' || unicode == '`') {
        return true;
    }

    if (inputBuffer_.length() >= kMaxInputLength) {
        return true;
    }

    char c = static_cast<char>(unicode);
    inputBuffer_.insert(cursorPos_, 1, c);
    cursorPos_++;

    return true;
}

void DevConsole::update(float dt) {
    overlayUpdateTimer_ += dt;
    if (overlayUpdateTimer_ >= kOverlayUpdateInterval) {
        overlayUpdateTimer_ = 0.f;

        if (deltaTimePtr_ != nullptr && *deltaTimePtr_ > 0.0001f) {
            cachedFPS_ = static_cast<int>(1.0f / *deltaTimePtr_);
        }

        if (networkClient_ != nullptr && networkClient_->isConnected()) {
            cachedPing_ = networkClient_->latencyMs();

            // Update ping history for jitter calculation
            pingHistory_.push_back(cachedPing_);
            if (pingHistory_.size() > kPingHistorySize) {
                pingHistory_.pop_front();
            }

            // Calculate jitter (standard deviation of ping)
            if (pingHistory_.size() >= 2) {
                float sum = std::accumulate(pingHistory_.begin(),
                                            pingHistory_.end(), 0.0f);
                float mean = sum / static_cast<float>(pingHistory_.size());
                float variance = 0.0f;
                for (auto ping : pingHistory_) {
                    float diff = static_cast<float>(ping) - mean;
                    variance += diff * diff;
                }
                variance /= static_cast<float>(pingHistory_.size());
                cachedJitter_ = std::sqrt(variance);
            }

            // Cache player position and active power-ups
            if (registry_ != nullptr) {
                auto userId = networkClient_->userId();
                cachedProcs_.clear();

                auto view = registry_->view<
                    rtype::games::rtype::shared::TransformComponent,
                    rtype::games::rtype::shared::NetworkIdComponent>();
                view.each([&](ECS::Entity e,
                              const rtype::games::rtype::shared::
                                  TransformComponent& t,
                              const rtype::games::rtype::shared::
                                  NetworkIdComponent& n) {
                    if (n.networkId == userId) {
                        cachedPlayerX_ = t.x;
                        cachedPlayerY_ = t.y;

                        // Check for active power-ups
                        if (registry_->hasComponent<rtype::games::rtype::shared::
                                                        ActivePowerUpComponent>(
                                e)) {
                            const auto& proc =
                                registry_->getComponent<rtype::games::rtype::
                                                            shared::
                                                                ActivePowerUpComponent>(
                                    e);
                            if (proc.type !=
                                rtype::games::rtype::shared::PowerUpType::None) {
                                CachedProc cp;
                                switch (proc.type) {
                                    case rtype::games::rtype::shared::
                                        PowerUpType::SpeedBoost:
                                        cp.name = "Speed";
                                        cp.multiplier = proc.speedMultiplier;
                                        break;
                                    case rtype::games::rtype::shared::
                                        PowerUpType::Shield:
                                        cp.name = "Shield";
                                        cp.multiplier = 1.f;
                                        break;
                                    case rtype::games::rtype::shared::
                                        PowerUpType::RapidFire:
                                        cp.name = "RapidFire";
                                        cp.multiplier = proc.fireRateMultiplier;
                                        break;
                                    case rtype::games::rtype::shared::
                                        PowerUpType::DoubleDamage:
                                        cp.name = "Damage";
                                        cp.multiplier = proc.damageMultiplier;
                                        break;
                                    default:
                                        cp.name = "Buff";
                                        cp.multiplier = 1.f;
                                        break;
                                }
                                cp.remainingTime = proc.remainingTime;
                                cachedProcs_.push_back(cp);
                            }
                        }
                    }
                });
            }
        }

        if (registry_ != nullptr) {
            cachedEntityCount_ = registry_->countComponents<
                rtype::games::rtype::shared::TransformComponent>();
        }

#ifdef __linux__
        // CPU sampling - calculate percentage from clock tick delta
        auto [utime, stime] = readProcStat();
        auto now = std::chrono::steady_clock::now();

        if (lastCpuSample_.valid && (utime > 0 || stime > 0)) {
            auto elapsed = std::chrono::duration<float>(
                               now - lastCpuSample_.timestamp)
                               .count();

            if (elapsed > 0.001f) {  // Avoid division by zero
                unsigned long ticksDelta =
                    (utime + stime) -
                    (lastCpuSample_.utime + lastCpuSample_.stime);

                static const long clockTicks = sysconf(_SC_CLK_TCK);
                static const long numCores = sysconf(_SC_NPROCESSORS_ONLN);
                float rawPercent =
                    (static_cast<float>(ticksDelta) /
                     static_cast<float>(clockTicks) / elapsed) *
                    100.0f;

                // Normalize by number of cores to get 0-100% range
                cachedCpuPercent_ = rawPercent / static_cast<float>(numCores > 0 ? numCores : 1);
                cachedCpuPercent_ = std::clamp(cachedCpuPercent_, 0.0f, 100.0f);
            }
        }

        lastCpuSample_.utime = utime;
        lastCpuSample_.stime = stime;
        lastCpuSample_.timestamp = now;
        lastCpuSample_.valid = true;
#endif
    }

    if (!visible_) {
        return;
    }

    cursorBlinkTimer_ += dt;
    if (cursorBlinkTimer_ >= kCursorBlinkRate) {
        cursorBlinkTimer_ = 0.f;
        cursorVisible_ = !cursorVisible_;
    }
}

void DevConsole::render() {
    if (!display_) {
        return;
    }

    auto viewCenter = display_->getViewCenter();
    auto viewSize = display_->getViewSize();
    display_->resetView();

    renderOverlays();

    if (visible_) {
        renderBackground();
        renderOutput();
        renderInputLine();
    }

    display_->setView(viewCenter, viewSize);
}

void DevConsole::renderBackground() {
    auto windowSize = display_->getWindowSize();
    float consoleHeight =
        static_cast<float>(windowSize.y) * kConsoleHeightRatio;

    display_->drawRectangle({0.f, 0.f},
                            {static_cast<float>(windowSize.x), consoleHeight},
                            kConsoleBgColor, kConsoleBgColor, 0.f);

    display_->drawRectangle(
        {0.f, consoleHeight - kInputLineHeight},
        {static_cast<float>(windowSize.x), kInputLineHeight},
        kConsoleInputBgColor, kConsoleInputBgColor, 0.f);
}

void DevConsole::renderOutput() {
    auto windowSize = display_->getWindowSize();
    float consoleHeight =
        static_cast<float>(windowSize.y) * kConsoleHeightRatio;
    float lineHeight = static_cast<float>(kFontSize) + 6.f;

    float outputAreaTop = kTextPadding;
    float outputAreaBottom = consoleHeight - kInputLineHeight - kTextPadding;
    float outputAreaHeight = outputAreaBottom - outputAreaTop;

    auto maxVisibleLines =
        static_cast<std::size_t>(outputAreaHeight / lineHeight);

    if (outputHistory_.empty() || maxVisibleLines == 0) {
        return;
    }

    std::vector<const OutputLine*> linesToDraw;
    std::size_t skipCount = scrollOffset_;

    for (auto it = outputHistory_.rbegin(); it != outputHistory_.rend(); ++it) {
        if (skipCount > 0) {
            --skipCount;
            continue;
        }
        linesToDraw.push_back(&(*it));
        if (linesToDraw.size() >= maxVisibleLines) {
            break;
        }
    }

    std::reverse(linesToDraw.begin(), linesToDraw.end());

    float y = outputAreaTop;
    for (const auto* line : linesToDraw) {
        rtype::display::Color color = kConsoleTextColor;
        if (line->isError) {
            color = kConsoleErrorColor;
        } else if (line->isInput) {
            color = kConsolePromptColor;  // Cyan for input lines
        }

        display_->drawText(line->text, std::string(kFontName),
                           {kTextPadding, y}, kFontSize, color);
        y += lineHeight;
    }
}

void DevConsole::renderInputLine() {
    auto windowSize = display_->getWindowSize();
    float consoleHeight =
        static_cast<float>(windowSize.y) * kConsoleHeightRatio;
    float inputY = consoleHeight - kInputLineHeight +
                   (kInputLineHeight - static_cast<float>(kFontSize)) / 2.f;

    display_->drawText(std::string(kPrompt), std::string(kFontName),
                       {kTextPadding, inputY}, kFontSize, kConsolePromptColor);

    auto promptBounds = display_->getTextBounds(
        std::string(kPrompt), std::string(kFontName), kFontSize);
    float textStartX = kTextPadding + promptBounds.x;

    display_->drawText(inputBuffer_, std::string(kFontName),
                       {textStartX, inputY}, kFontSize, kConsoleTextColor);

    if (cursorVisible_) {
        std::string textBeforeCursor = inputBuffer_.substr(0, cursorPos_);
        auto cursorBounds = display_->getTextBounds(
            textBeforeCursor, std::string(kFontName), kFontSize);
        float cursorX = textStartX + cursorBounds.x;

        display_->drawRectangle({cursorX, inputY},
                                {2.f, static_cast<float>(kFontSize)},
                                kConsoleCursorColor, kConsoleCursorColor, 0.f);
    }
}

void DevConsole::renderOverlays() {
    auto windowSize = display_->getWindowSize();
    constexpr float kOverlayFontSize = 14;
    constexpr float kOverlayLineHeight = 18.f;
    constexpr float kOverlayPadding = 10.f;
    constexpr rtype::display::Color kOverlayColor{0, 255, 0, 255};
    constexpr rtype::display::Color kOverlayBgColor{0, 0, 0, 150};

    float y = kOverlayPadding;
    float maxWidth = 0.f;
    std::vector<std::string> lines;

    if (getCvar("cl_show_fps") == "1" && cachedFPS_ > 0) {
        lines.push_back("FPS: " + std::to_string(cachedFPS_));
    }

    if (getCvar("cl_show_ping") == "1" && networkClient_ != nullptr &&
        networkClient_->isConnected()) {
        lines.push_back("Ping: " + std::to_string(cachedPing_) + "ms");
    }

    if (getCvar("cl_show_entities") == "1" && registry_ != nullptr) {
        lines.push_back("Entities: " + std::to_string(cachedEntityCount_));
    }

    // World Position overlay
    if (getCvar("cl_show_position") == "1" &&
        (cachedPlayerX_ != 0.f || cachedPlayerY_ != 0.f)) {
        std::ostringstream oss;
        oss << "Pos: X=" << std::fixed << std::setprecision(1) << cachedPlayerX_
            << " Y=" << cachedPlayerY_;
        lines.push_back(oss.str());
    }

    // Resources overlay
    if (getCvar("cl_show_resources") == "1") {
        auto metrics = getSystemMetrics(cachedCpuPercent_);
        if (metrics.memAvailable) {
            std::string line = "RAM: " + std::to_string(metrics.memoryMB) + " MB";
            if (metrics.cpuAvailable) {
                line = "CPU: " +
                       std::to_string(static_cast<int>(metrics.cpuPercent)) +
                       "% | " + line;
            }
            lines.push_back(line);
        } else {
            lines.push_back("Resources: N/A (Linux/Windows only)");
        }
    }

    // Lagometer overlay
    if (getCvar("cl_show_lagometer") == "1" && networkClient_ != nullptr &&
        networkClient_->isConnected()) {
        std::ostringstream oss;
        oss << "Ping: " << cachedPing_ << "ms | Jitter: "
            << static_cast<char>(0xB1)  // ± symbol
            << static_cast<int>(cachedJitter_) << "ms";
        lines.push_back(oss.str());
    }

    // Active power-ups (proc) overlay
    if (getCvar("cl_show_proc") == "1") {
        if (cachedProcs_.empty()) {
            lines.push_back("Procs: None");
        } else {
            for (const auto& proc : cachedProcs_) {
                std::ostringstream oss;
                oss << proc.name;
                if (proc.multiplier != 1.f) {
                    oss << " x" << std::fixed << std::setprecision(1)
                        << proc.multiplier;
                }
                if (proc.remainingTime > 0.f) {
                    oss << " (" << std::fixed << std::setprecision(1)
                        << proc.remainingTime << "s)";
                }
                lines.push_back(oss.str());
            }
        }
    }

    for (const auto& line : lines) {
        auto bounds = display_->getTextBounds(line, std::string(kFontName),
                                              kOverlayFontSize);
        if (bounds.x > maxWidth) {
            maxWidth = bounds.x;
        }
    }

    if (!lines.empty()) {
        float bgHeight = static_cast<float>(lines.size()) * kOverlayLineHeight +
                         kOverlayPadding;
        float bgWidth = maxWidth + kOverlayPadding * 2;
        display_->drawRectangle({kOverlayPadding - 5.f, kOverlayPadding - 5.f},
                                {bgWidth, bgHeight}, kOverlayBgColor,
                                kOverlayBgColor, 0.f);
    }

    for (const auto& line : lines) {
        display_->drawText(line, std::string(kFontName), {kOverlayPadding, y},
                           kOverlayFontSize, kOverlayColor);
        y += kOverlayLineHeight;
    }
}

void DevConsole::registerCommand(
    const std::string& name, const std::string& description,
    std::function<std::string(const std::vector<std::string>&)> handler) {
    commands_[toLower(name)] = {description, std::move(handler)};
}

void DevConsole::execute(const std::string& commandLine) {
    if (commandLine.empty()) {
        return;
    }

    if (commandHistory_.empty() || commandHistory_.back() != commandLine) {
        commandHistory_.push_back(commandLine);
        if (commandHistory_.size() > kMaxHistoryLines) {
            commandHistory_.pop_front();
        }
    }

    // Print input line with special flag for coloring
    std::string timestamp = getTimestamp();
    outputHistory_.push_back(
        {timestamp + std::string(kPrompt) + commandLine, false, true});
    if (outputHistory_.size() > kMaxHistoryLines) {
        outputHistory_.pop_front();
    }

    auto args = parseArgs(commandLine);
    if (args.empty()) {
        return;
    }

    std::string cmdName = toLower(args[0]);
    auto it = commands_.find(cmdName);

    if (it == commands_.end()) {
        printError("Unknown command: " + args[0]);
        return;
    }

    args.erase(args.begin());

    std::string result = it->second.second(args);
    if (!result.empty()) {
        print(result);
    }
}

void DevConsole::executeCurrentInput() {
    if (inputBuffer_.empty()) {
        return;
    }

    execute(inputBuffer_);
    inputBuffer_.clear();
    cursorPos_ = 0;
    historyIndex_ = -1;
    scrollOffset_ = 0;
}

void DevConsole::navigateHistory(int direction) {
    if (commandHistory_.empty()) {
        return;
    }

    if (direction < 0) {
        if (historyIndex_ < 0) {
            historyIndex_ = static_cast<int>(commandHistory_.size()) - 1;
        } else if (historyIndex_ > 0) {
            historyIndex_--;
        }
    } else {
        if (historyIndex_ >= 0) {
            historyIndex_++;
            if (historyIndex_ >= static_cast<int>(commandHistory_.size())) {
                historyIndex_ = -1;
                inputBuffer_.clear();
                cursorPos_ = 0;
                return;
            }
        }
    }

    if (historyIndex_ >= 0 &&
        historyIndex_ < static_cast<int>(commandHistory_.size())) {
        inputBuffer_ = commandHistory_[static_cast<std::size_t>(historyIndex_)];
        cursorPos_ = inputBuffer_.length();
    }
}

std::vector<std::string> DevConsole::parseArgs(const std::string& input) {
    std::vector<std::string> args;
    std::istringstream stream(input);
    std::string token;

    while (stream >> token) {
        args.push_back(token);
    }

    return args;
}

void DevConsole::print(const std::string& message) {
    std::string timestamp = getTimestamp();
    std::istringstream stream(message);
    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        if (firstLine) {
            outputHistory_.push_back({timestamp + line, false});
            firstLine = false;
        } else {
            outputHistory_.push_back({"           " + line, false});
        }
        if (outputHistory_.size() > kMaxHistoryLines) {
            outputHistory_.pop_front();
        }
    }
}

void DevConsole::printError(const std::string& message) {
    std::string timestamp = getTimestamp();
    std::istringstream stream(message);
    std::string line;
    bool firstLine = true;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        if (firstLine) {
            outputHistory_.push_back({timestamp + line, true});
            firstLine = false;
        } else {
            outputHistory_.push_back({"           " + line, true});
        }
        if (outputHistory_.size() > kMaxHistoryLines) {
            outputHistory_.pop_front();
        }
    }
}

void DevConsole::setCvar(const std::string& name, const std::string& value) {
    cvars_[toLower(name)] = value;
}

std::string DevConsole::getCvar(const std::string& name) const {
    auto it = cvars_.find(toLower(name));
    if (it != cvars_.end()) {
        return it->second;
    }
    return "";
}

void DevConsole::registerDefaultCommands() {
    registerCommand(
        "help", "Display available commands or help for a specific command",
        [this](const std::vector<std::string>& args) -> std::string {
            if (args.empty()) {
                std::string result = "Available commands:\n";
                for (const auto& [name, info] : commands_) {
                    result += "  " + name + " - " + info.first + "\n";
                }
                return result;
            }

            std::string cmdName = toLower(args[0]);
            auto it = commands_.find(cmdName);
            if (it != commands_.end()) {
                return it->first + ": " + it->second.first;
            }
            return "Unknown command: " + args[0];
        });

    registerCommand("clear", "Clear the console output",
                    [this](const std::vector<std::string>&) -> std::string {
                        outputHistory_.clear();
                        scrollOffset_ = 0;
                        return "";
                    });

    registerCommand("quit", "Quit the game",
                    [this](const std::vector<std::string>&) -> std::string {
                        if (display_) {
                            display_->close();
                        }
                        return "Goodbye!";
                    });

    registerCommand(
        "set", "Set a console variable (usage: set <name> <value>)",
        [this](const std::vector<std::string>& args) -> std::string {
            if (args.size() < 2) {
                return "Usage: set <name> <value>";
            }
            setCvar(args[0], args[1]);
            return args[0] + " = " + args[1];
        });

    registerCommand(
        "get", "Get a console variable value (usage: get <name>)",
        [this](const std::vector<std::string>& args) -> std::string {
            if (args.empty()) {
                return "Usage: get <name>";
            }
            std::string value = getCvar(args[0]);
            if (value.empty()) {
                return "CVar not found: " + args[0];
            }
            return args[0] + " = " + value;
        });

    registerCommand("list", "List all console variables",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string result = "Console Variables:\n";
                        for (const auto& [name, value] : cvars_) {
                            result += "  " + name + " = " + value + "\n";
                        }
                        return result;
                    });

    registerCommand(
        "god", "Toggle god mode (invincibility) - requires localhost",
        [this](const std::vector<std::string>&) -> std::string {
            if (!networkClient_ || !networkClient_->isConnected()) {
                return "Error: Not connected to server";
            }

            bool sent = networkClient_->sendAdminCommand(
                static_cast<std::uint8_t>(network::AdminCommandType::GodMode),
                2);

            return sent ? "God mode request sent..." : "Failed to send request";
        });

    registerCommand("echo", "Print a message to the console",
                    [](const std::vector<std::string>& args) -> std::string {
                        std::string result;
                        for (const auto& arg : args) {
                            if (!result.empty()) {
                                result += " ";
                            }
                            result += arg;
                        }
                        return result;
                    });

    registerCommand("fps", "Toggle FPS display overlay",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_fps");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_fps", newVal);
                        return newVal == "1" ? "FPS display ON"
                                             : "FPS display OFF";
                    });

    registerCommand("ping", "Toggle ping/latency display overlay",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_ping");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_ping", newVal);
                        return newVal == "1" ? "Ping display ON"
                                             : "Ping display OFF";
                    });

    registerCommand("mute", "Toggle audio mute (music + SFX)",
                    [this](const std::vector<std::string>&) -> std::string {
                        if (!audioLib_) {
                            return "Error: Audio not available";
                        }

                        std::string current = getCvar("cl_mute_audio");
                        if (current == "0") {
                            savedMusicVolume_ = audioLib_->getMusicVolume();
                            savedSFXVolume_ = audioLib_->getSFXVolume();
                            audioLib_->setMusicVolume(0.0f);
                            audioLib_->setSFXVolume(0.0f);
                            setCvar("cl_mute_audio", "1");
                            return "Audio MUTED";
                        } else {
                            audioLib_->setMusicVolume(savedMusicVolume_);
                            audioLib_->setSFXVolume(savedSFXVolume_);
                            setCvar("cl_mute_audio", "0");
                            return "Audio UNMUTED";
                        }
                    });

    registerCommand("entities", "Toggle entity count display overlay",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_entities");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_entities", newVal);
                        return newVal == "1" ? "Entity count ON"
                                             : "Entity count OFF";
                    });

    registerCommand(
        "hitbox", "Toggle hitbox display",
        [this](const std::vector<std::string>&) -> std::string {
            std::string current = getCvar("cl_show_hitboxes");
            std::string newVal = (current == "1") ? "0" : "1";
            setCvar("cl_show_hitboxes", newVal);

            if (!registry_) {
                LOG_WARNING("[DevConsole] hitbox: registry_ is null!");
                return "Error: Registry not available";
            }

            if (!registry_->hasSingleton<AccessibilitySettings>()) {
                LOG_WARNING(
                    "[DevConsole] hitbox: AccessibilitySettings singleton not "
                    "found!");
                return "Error: AccessibilitySettings not initialized";
            }

            auto& acc = registry_->getSingleton<AccessibilitySettings>();
            acc.showHitboxes = (newVal == "1");
            LOG_DEBUG(
                "[DevConsole] hitbox: Set showHitboxes=" << acc.showHitboxes);

            return newVal == "1" ? "Hitboxes ON" : "Hitboxes OFF";
        });

    registerCommand("position", "Toggle player position display",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_position");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_position", newVal);
                        return newVal == "1" ? "Position display ON"
                                             : "Position display OFF";
                    });

    registerCommand("resources", "Toggle CPU/RAM usage display",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_resources");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_resources", newVal);
                        return newVal == "1" ? "Resources display ON"
                                             : "Resources display OFF";
                    });

    registerCommand("lagometer", "Toggle network lagometer (ping + jitter)",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_lagometer");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_lagometer", newVal);
                        return newVal == "1" ? "Lagometer ON" : "Lagometer OFF";
                    });

    registerCommand("proc", "Toggle active power-ups display",
                    [this](const std::vector<std::string>&) -> std::string {
                        std::string current = getCvar("cl_show_proc");
                        std::string newVal = (current == "1") ? "0" : "1";
                        setCvar("cl_show_proc", newVal);
                        return newVal == "1" ? "Proc display ON"
                                             : "Proc display OFF";
                    });
}

}  // namespace rtype::client
