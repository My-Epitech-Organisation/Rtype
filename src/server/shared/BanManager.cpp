/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** BanManager - Implementation
*/

#include "BanManager.hpp"

namespace rtype::server {

bool BanManager::isEndpointBanned(const Endpoint& endpoint) const {
    std::shared_lock lock(_mutex);
    if (_bannedEndpoints.count(endpoint) > 0) {
        return true;
    }
    return _bannedIps.count(endpoint.address) > 0;
}

bool BanManager::isIpBanned(const std::string& ip) const {
    std::shared_lock lock(_mutex);
    return _bannedIps.count(ip) > 0;
}

void BanManager::banEndpoint(const Endpoint& endpoint,
                             const std::string& playerName,
                             const std::string& reason) {
    std::unique_lock lock(_mutex);
    _bannedEndpoints.insert(endpoint);
    _banDetails[endpoint] = {endpoint.address, endpoint.port, playerName,
                             reason};
}

void BanManager::banIp(const std::string& ip, const std::string& playerName,
                       const std::string& reason) {
    std::unique_lock lock(_mutex);
    _bannedIps.insert(ip);
    _ipBanDetails[ip] = {ip, 0, playerName, reason};
}

void BanManager::unbanEndpoint(const Endpoint& endpoint) {
    std::unique_lock lock(_mutex);
    _bannedEndpoints.erase(endpoint);
    _banDetails.erase(endpoint);
}

void BanManager::unbanIp(const std::string& ip) {
    std::unique_lock lock(_mutex);
    _bannedIps.erase(ip);
    _ipBanDetails.erase(ip);
}

std::vector<BanManager::BannedEndpoint> BanManager::getBannedList() const {
    std::shared_lock lock(_mutex);
    std::vector<BannedEndpoint> result;
    result.reserve(_banDetails.size() + _ipBanDetails.size());
    for (const auto& pair : _banDetails) {
        result.push_back(pair.second);
    }
    for (const auto& pair : _ipBanDetails) {
        result.push_back(pair.second);
    }
    return result;
}

void BanManager::clearAllBans() {
    std::unique_lock lock(_mutex);
    _bannedEndpoints.clear();
    _banDetails.clear();
    _bannedIps.clear();
    _ipBanDetails.clear();
}

}  // namespace rtype::server
