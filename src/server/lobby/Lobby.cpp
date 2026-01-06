/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Lobby - Implementation
*/

#include "Lobby.hpp"

#include <stdexcept>

#include "Logger/Logger.hpp"

namespace rtype::server {

Lobby::Lobby(const std::string& code, const Config& config)
    : code_(code),
      config_(config),
      shutdownFlagPtr_(std::make_shared<std::atomic<bool>>(false)),
      lastActivity_(std::chrono::steady_clock::now()) {
    if (code_.length() != 6) {
        throw std::invalid_argument("Lobby code must be exactly 6 characters");
    }
}

Lobby::~Lobby() { stop(); }

bool Lobby::start() {
    if (running_) {
        rtype::Logger::instance().warning(
            std::format("Lobby {} already running", code_));
        return false;
    }

    try {
        rtype::Logger::instance().info(
            std::format("Creating ServerApp for lobby {} on port {}...", code_,
                        config_.port));

        serverApp_ =
            std::make_unique<ServerApp>(config_.port, config_.maxPlayers,
                                        config_.tickRate, shutdownFlagPtr_,
                                        10,    // clientTimeoutSeconds
                                        false  // verbose
            );

        rtype::Logger::instance().info(std::format(
            "ServerApp created, starting thread for lobby {}...", code_));

        thread_ = std::make_unique<std::thread>(&Lobby::run, this);

        rtype::Logger::instance().info(std::format(
            "Thread created, waiting for lobby {} to initialize...", code_));

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (serverApp_) {
            actualPort_ = config_.port;
            running_ = true;
            rtype::Logger::instance().info(
                std::format("Lobby {} started successfully on port {}", code_,
                            actualPort_));
            return true;
        }

        rtype::Logger::instance().error(
            std::format("serverApp_ became null for lobby {}", code_));
        return false;
    } catch (const std::exception& e) {
        rtype::Logger::instance().error(
            std::format("Exception starting lobby {}: {}", code_, e.what()));
        return false;
    }
}

void Lobby::stop() {
    if (!running_) {
        return;
    }

    rtype::Logger::instance().info(std::format("Stopping lobby {}...", code_));

    shutdownFlagPtr_->store(true);

    if (thread_ && thread_->joinable()) {
        thread_->join();
    }

    serverApp_.reset();
    thread_.reset();
    running_ = false;
    actualPort_ = 0;

    rtype::Logger::instance().info(std::format("Lobby {} stopped", code_));
}

bool Lobby::isRunning() const { return running_; }

std::uint32_t Lobby::getPlayerCount() const {
    if (!serverApp_) {
        return 0;
    }
    return serverApp_->getConnectedClientCount();
}

bool Lobby::isEmpty() const { return getPlayerCount() == 0; }

std::chrono::seconds Lobby::getTimeSinceEmpty() const {
    if (!isEmpty()) {
        return std::chrono::seconds(0);
    }

    std::lock_guard<std::mutex> lock(activityMutex_);
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now -
                                                            lastActivity_);
}

void Lobby::updateActivity() {
    std::lock_guard<std::mutex> lock(activityMutex_);
    lastActivity_ = std::chrono::steady_clock::now();
}

void Lobby::run() {
    try {
        rtype::Logger::instance().info(
            std::format("Lobby {} thread started", code_));

        if (!serverApp_->run()) {
            rtype::Logger::instance().error(
                std::format("Lobby {} failed to run", code_));
        }

        rtype::Logger::instance().info(
            std::format("Lobby {} thread finished normally", code_));
    } catch (const std::exception& e) {
        rtype::Logger::instance().error(
            std::format("Lobby {} thread crashed: {}", code_, e.what()));
    }
}

}  // namespace rtype::server
