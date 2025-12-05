/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Main server application header
*/

#ifndef SRC_SERVER_SERVERAPP_HPP_
#define SRC_SERVER_SERVERAPP_HPP_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include "../common/Logger.hpp"
#include "../common/SafeQueue/SafeQueue.hpp"
#include "../common/Types.hpp"
#include "../network/Packet.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "IGameConfig.hpp"
#include "ServerMetrics.hpp"

namespace rtype::server {

using rtype::ClientId;
using rtype::Endpoint;

/**
 * @brief Main server application class
 *
 * Manages the server main loop, client connections, and game state.
 * Handles client timeout detection and notifies other clients when
 * a client disconnects (including crashes).
 *
 * Features:
 * - Main while(running) loop with configurable tick rate
 * - Client connection tracking with unique IDs
 * - O(1) endpoint-to-client lookup using dual hash maps
 * - Timeout/disconnect detection
 * - Server continues running even if no clients are connected
 * - Automatic notification to other clients when a client disconnects
 * - Thread-safe client management using shared_mutex for read-heavy workloads
 *
 * @note Network functionality is stubbed until rtype_network is fully
 * implemented.
 *
 * Usage:
 * @code
 * std::atomic<bool> shutdown{false};
 * rtype::server::ServerApp server(4242, 8, 60, shutdown);
 * server.run(); // Blocking call
 * @endcode
 */
class ServerApp {
   public:
    static constexpr uint32_t DEFAULT_CLIENT_TIMEOUT_SECONDS = 10;

    // @brief Maximum physics/logic updates per frame to prevent spiral of
    // death
    // @details When the game loop falls behind (e.g., due to a lag spike),
    // limiting updates per frame prevents spending too long catching up,
    // which would cause further frame drops and create a feedback loop.
    static constexpr uint32_t MAX_UPDATES_PER_FRAME = 5;

    // @brief Maximum frame time in milliseconds before clamping
    // @details Prevents spiral of death during severe lag spikes. If a frame
    // takes longer than this, we clamp it to avoid accumulating too much
    // time in the accumulator, which would cause excessive catch-up updates.
    // 250ms allows ~4 FPS minimum before time clamping kicks in.
    static constexpr uint32_t MAX_FRAME_TIME_MS = 250;

    // @brief Percentage of calculated sleep time to actually sleep
    // @details We sleep for only 95% of the remaining frame time to account
    // for OS scheduler granularity and potential timing inaccuracies.
    // This prevents oversleeping past the target frame time.
    static constexpr uint32_t SLEEP_TIME_SAFETY_PERCENT = 95;

    // @brief Minimum sleep threshold in microseconds
    // @details Below this threshold, busy-waiting is more accurate than
    // sleeping. Sleep syscalls have overhead and OS scheduler granularity
    // (typically 1-15ms on most systems) makes very short sleeps unreliable.
    static constexpr uint32_t MIN_SLEEP_THRESHOLD_US = 100;

    /**
     * @brief Construct a new ServerApp with manual configuration
     * @param port Port to listen on
     * @param maxPlayers Maximum number of concurrent players
     * @param tickRate Server tick rate in Hz
     * @param shutdownFlag Shared pointer to external shutdown flag (set by
     * signal handler)
     * @param clientTimeoutSeconds Client timeout in seconds (default: 10)
     * @param verbose Enable verbose debug output (default: false)
     */
    explicit ServerApp(
        uint16_t port, size_t maxPlayers, uint32_t tickRate,
        std::shared_ptr<std::atomic<bool>> shutdownFlag,
        uint32_t clientTimeoutSeconds = DEFAULT_CLIENT_TIMEOUT_SECONDS,
        bool verbose = false);

    /**
     * @brief Construct a new ServerApp with game configuration
     * @param gameConfig Game-specific configuration (takes ownership)
     * @param shutdownFlag Shared pointer to external shutdown flag
     * @param verbose Enable verbose debug output (default: false)
     */
    explicit ServerApp(std::unique_ptr<IGameConfig> gameConfig,
                       std::shared_ptr<std::atomic<bool>> shutdownFlag,
                       bool verbose = false);

    /**
     * @brief Destructor - ensures clean shutdown
     * @note Does NOT modify external shutdown flag to avoid side effects
     */
    ~ServerApp();

    ServerApp(const ServerApp&) = delete;
    ServerApp& operator=(const ServerApp&) = delete;
    ServerApp(ServerApp&&) = delete;
    ServerApp& operator=(ServerApp&&) = delete;

    /**
     * @brief Start the server main loop
     *
     * This is a blocking call that runs until the shutdown flag is set.
     * The server will continue running even if no clients are connected.
     *
     * @return true if server ran and shut down gracefully, false if
     * initialization failed
     */
    [[nodiscard]] bool run();

    /**
     * @brief Signal the server to stop
     *
     * This can be called from another thread to gracefully stop the server.
     */
    void stop() noexcept;

    /**
     * @brief Check if the server is currently running
     * @return true if the server main loop is active
     */
    [[nodiscard]] bool isRunning() const noexcept;

    /**
     * @brief Get the number of currently connected clients
     * @return Number of connected clients
     */
    [[nodiscard]] size_t getConnectedClientCount() const noexcept;

    /**
     * @brief Get list of all connected client IDs
     * @return Vector of client IDs
     */
    [[nodiscard]] std::vector<ClientId> getConnectedClientIds() const;

    /**
     * @brief Get client information by ID (thread-safe copy)
     * @param clientId The client ID to look up
     * @return Optional containing client info if found
     */
    [[nodiscard]] std::optional<Client> getClientInfo(ClientId clientId) const;

    /**
     * @brief Get server metrics (thread-safe)
     * @return Reference to current metrics
     */
    [[nodiscard]] const ServerMetrics& getMetrics() const noexcept {
        return *_metrics;
    }

    /**
     * @brief Get client manager (for testing/advanced usage)
     * @return Reference to client manager
     */
    [[nodiscard]] ClientManager& getClientManager() noexcept {
        return _clientManager;
    }

    /**
     * @brief Get client manager (const version)
     * @return Const reference to client manager
     */
    [[nodiscard]] const ClientManager& getClientManager() const noexcept {
        return _clientManager;
    }

    /**
     * @brief Get game configuration (if available)
     * @return Pointer to game config, or nullptr if not set
     */
    [[nodiscard]] IGameConfig* getGameConfig() noexcept {
        return _gameConfig.get();
    }

    /**
     * @brief Get game configuration (const version)
     * @return Const pointer to game config, or nullptr if not set
     */
    [[nodiscard]] const IGameConfig* getGameConfig() const noexcept {
        return _gameConfig.get();
    }

    /**
     * @brief Check if game configuration is available
     * @return true if game config is loaded
     */
    [[nodiscard]] bool hasGameConfig() const noexcept {
        return _gameConfig != nullptr && _gameConfig->isInitialized();
    }

    /**
     * @brief Reload server configuration (hot-reload)
     * @return true if reload was successful
     */
    [[nodiscard]] bool reloadConfiguration();

   private:
    // @brief Configuration for the main loop timing
    struct LoopTiming {
        std::chrono::nanoseconds fixedDeltaNs;
        std::chrono::nanoseconds maxFrameTime;
        uint32_t maxUpdatesPerFrame;
    };

    // @brief State for the main loop
    struct LoopState {
        std::chrono::steady_clock::time_point previousTime;
        std::chrono::nanoseconds accumulator{0};
    };

    /**
     * @brief Initialize server resources
     * @return true if initialization succeeded
     */
    [[nodiscard]] bool initialize();

    /**
     * @brief Clean up server resources
     */
    void shutdown() noexcept;

    /**
     * @brief Log server startup information
     */
    void logStartupInfo() const noexcept;

    /**
     * @brief Create timing configuration for the main loop
     * @return LoopTiming configuration struct
     */
    [[nodiscard]] LoopTiming createLoopTiming() const noexcept;

    /**
     * @brief Calculate frame time and clamp if necessary
     * @param state Current loop state (previousTime will be updated)
     * @param timing Loop timing configuration
     * @return Clamped frame time in nanoseconds
     */
    [[nodiscard]] std::chrono::nanoseconds calculateFrameTime(
        std::shared_ptr<LoopState> state, const LoopTiming& timing) noexcept;

    /**
     * @brief Perform fixed timestep updates
     * @param state Current loop state (accumulator will be updated)
     * @param timing Loop timing configuration
     */
    void performFixedUpdates(std::shared_ptr<LoopState> state,
                             const LoopTiming& timing) noexcept;

    /**
     * @brief Sleep to maintain target frame rate
     * @param frameStartTime When the current frame started
     * @param timing Loop timing configuration
     */
    void sleepUntilNextFrame(
        std::chrono::steady_clock::time_point frameStartTime,
        const LoopTiming& timing) noexcept;

    /**
     * @brief Process all incoming network data
     */
    void processIncomingData() noexcept;

    /**
     * @brief Process a single packet from a client
     * @param clientId The client that sent the packet
     * @param packet The packet data
     */
    void processPacket(ClientId clientId,
                       const rtype::network::Packet& packet) noexcept;

    /**
     * @brief Start the network thread for receiving packets
     * @return true if network thread started successfully
     */
    [[nodiscard]] bool startNetworkThread();

    /**
     * @brief Stop the network thread
     */
    void stopNetworkThread() noexcept;

    /**
     * @brief Network thread function that receives packets and pushes to queue
     */
    void networkThreadFunction() noexcept;

    /**
     * @brief Update game state (ECS tick)
     */
    void update() noexcept;

    /**
     * @brief Send game state updates to all clients
     */
    void broadcastGameState() noexcept;

    uint16_t _port;                  ///< Server port
    uint32_t _tickRate;              ///< Server tick rate in Hz
    uint32_t _clientTimeoutSeconds;  ///< Client timeout in seconds
    bool _verbose;                   ///< Enable verbose debug output
    std::shared_ptr<std::atomic<bool>>
        _shutdownFlag;  ///< External shutdown flag shared pointer
    std::atomic<bool> _hasShutdown{false};  ///< Guard against double shutdown

    std::shared_ptr<ServerMetrics> _metrics;   ///< Server performance metrics
    ClientManager _clientManager;              ///< Client connection manager
    std::unique_ptr<IGameConfig> _gameConfig;  ///< Game-specific configuration

    // Network thread and packet queue for producer-consumer pattern
    SafeQueue<std::pair<Endpoint, rtype::network::Packet>>
        _incomingPackets;        ///< Thread-safe queue for incoming packets
    std::thread _networkThread;  ///< Network I/O thread
    std::atomic<bool> _networkThreadRunning{
        false};  ///< Flag to control network thread

    // TODO(Clem): Add network socket when rtype_network is fully implemented
    // std::unique_ptr<network::UdpSocket> _socket;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_HPP_
