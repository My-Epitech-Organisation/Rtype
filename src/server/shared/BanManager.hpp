/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** BanManager - Manages banned endpoints (IP:port pairs)
*/

#ifndef SRC_SERVER_SHARED_BANMANAGER_HPP_
#define SRC_SERVER_SHARED_BANMANAGER_HPP_

#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <rtype/common.hpp>

namespace rtype::server {

using rtype::Endpoint;

/**
 * @brief Manages banned client endpoints
 *
 * Thread-safe management of banned IP:port pairs.
 * Prevents banned clients from reconnecting to the server.
 */
class BanManager {
   public:
    /**
     * @brief Information about a banned endpoint
     */
    struct BannedEndpoint {
        std::string ip;
        uint16_t port;
        std::string playerName;
        std::string reason;
    };

    BanManager() = default;
    ~BanManager() = default;

    BanManager(const BanManager&) = delete;
    BanManager& operator=(const BanManager&) = delete;
    BanManager(BanManager&&) = delete;
    BanManager& operator=(BanManager&&) = delete;

    /**
     * @brief Check if an endpoint is banned
     * @param endpoint Network endpoint to check
     * @return true if banned, false otherwise
     */
    [[nodiscard]] bool isEndpointBanned(const Endpoint& endpoint) const;

    /**
     * @brief Check if an IP address is banned
     */
    [[nodiscard]] bool isIpBanned(const std::string& ip) const;

    /**
     * @brief Ban an endpoint
     * @param endpoint Network endpoint to ban
     * @param playerName Optional player name for reference
     * @param reason Ban reason
     */
    void banEndpoint(const Endpoint& endpoint,
                     const std::string& playerName = "",
                     const std::string& reason = "");

    /**
     * @brief Ban by IP address (all ports)
     */
    void banIp(const std::string& ip, const std::string& playerName = "",
               const std::string& reason = "");

    /**
     * @brief Unban an endpoint
     * @param endpoint Network endpoint to unban
     */
    void unbanEndpoint(const Endpoint& endpoint);

    /**
     * @brief Unban an IP address
     */
    void unbanIp(const std::string& ip);

    /**
     * @brief Get list of all banned endpoints
     * @return Vector of banned endpoint information
     */
    [[nodiscard]] std::vector<BannedEndpoint> getBannedList() const;

    /**
     * @brief Clear all bans
     */
    void clearAllBans();

   private:
    mutable std::shared_mutex _mutex;
    std::set<Endpoint> _bannedEndpoints;
    std::unordered_map<Endpoint, BannedEndpoint> _banDetails;
    std::unordered_set<std::string> _bannedIps;
    std::unordered_map<std::string, BannedEndpoint> _ipBanDetails;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_BANMANAGER_HPP_
