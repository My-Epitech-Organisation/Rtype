/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** AdminServer - Implementation
*/
// NOLINT(build/namespaces)

#include "AdminServer.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

#include <rtype/common.hpp>

#include "httplib.h"
#include "server/lobby/Lobby.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "server/serverApp/ServerApp.hpp"

namespace rtype::server {
using ::httplib::Request;
using ::httplib::Response;
using ::httplib::Server;

AdminServer::AdminServer(const Config& config, ServerApp* serverApp,
                         LobbyManager* lobbyManager)
    : _config(config),
      _serverApp(serverApp),
      _lobbyManager(lobbyManager),
      _httpServer(nullptr) {}

AdminServer::~AdminServer() {
    stop();
    if (_httpServer) {
        delete static_cast<Server*>(_httpServer);
        _httpServer = nullptr;
    }
}

bool AdminServer::start() {
    if (_running) {
        return true;
    }

    _httpServer = new Server();
    setupRoutes();

    _running = true;
    _serverThread = std::thread([this]() { runServer(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    LOG_INFO_CAT(::rtype::LogCategory::Network,
                 "[AdminServer] Started on port " << _config.port);
    return true;
}

void AdminServer::stop() {
    if (!_running) {
        return;
    }

    _running = false;
    if (_httpServer) {
        static_cast<Server*>(_httpServer)->stop();
    }

    if (_serverThread.joinable()) {
        _serverThread.join();
    }

    LOG_INFO_CAT(::rtype::LogCategory::Network, "[AdminServer] Stopped");
}

bool AdminServer::isRunning() const noexcept {
    return _running && _httpServer &&
           static_cast<Server*>(_httpServer)->is_running();
}

void AdminServer::runServer() noexcept {
    if (_httpServer) {
        static_cast<Server*>(_httpServer)->listen("0.0.0.0", _config.port);
    }
}

bool authenticateRequest(const AdminServer::Config& config,
                         const Request& req) {
    if (config.localhostOnly) {
        const auto& remote_addr = req.remote_addr;
        bool localhost = remote_addr == "127.0.0.1" ||
                         remote_addr == "localhost" || remote_addr == "::1";
        if (!localhost) {
            return false;
        }
    }

    if (!config.token.empty()) {
        const auto& auth = req.get_header_value("Authorization");
        if (auth.empty() || auth != ("Bearer " + config.token)) {
            return false;
        }
    }

    return true;
}

void AdminServer::setupRoutes() {  // NOLINT(readability/fn_size)
    auto* server = static_cast<Server*>(_httpServer);

    registerAdminPageRoutes(server);

    registerMetricsRoutes(server);

    registerLobbyRoutes(server);

    registerBanRoutes(server);

    server->Post("/api/unban", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        std::string body = req.body;
        std::string ip;
        std::uint16_t port = 0;
        if (auto pos = body.find("\"ip\""); pos != std::string::npos) {
            auto start = body.find("\"", pos + 4);
            auto end = (start != std::string::npos) ? body.find("\"", start + 1)
                                                    : std::string::npos;
            if (start != std::string::npos && end != std::string::npos) {
                ip = body.substr(start + 1, end - start - 1);
            }
        }
        if (auto pos = body.find("\"port\""); pos != std::string::npos) {
            auto colon = body.find(":", pos);
            if (colon != std::string::npos) {
                try {
                    port = static_cast<std::uint16_t>(
                        std::stoul(body.substr(colon + 1)));
                } catch (...) {
                }
            }
        }
        if (ip.empty()) {
            res.set_content(R"({"success":false,"error":"Missing ip"})",
                            "application/json");
            res.status = 400;
            return;
        }
        Endpoint ep;
        ep.address = ip;
        ep.port = port;
        if (_lobbyManager) {
            auto lobbies = _lobbyManager->getAllLobbies();
            for (auto* lobby : lobbies) {
                if (auto* sa = lobby->getServerApp()) {
                    if (port == 0) {
                        sa->getBanManager().unbanIp(ip);
                    } else {
                        sa->getBanManager().unbanEndpoint(ep);
                    }
                }
            }
        } else if (_serverApp) {
            if (port == 0) {
                _serverApp->getBanManager().unbanIp(ip);
            } else {
                _serverApp->getBanManager().unbanEndpoint(ep);
            }
        }
        res.set_content(R"({"success":true})", "application/json");
        res.status = 200;
    });

    server->Post("/api/lobby/create", [this](const Request& req,
                                             Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        std::string body = req.body;
        bool isPrivate = true;
        if (auto pos = body.find("\"isPublic\""); pos != std::string::npos) {
            auto colon = body.find(":", pos);
            if (colon != std::string::npos) {
                isPrivate = body.find("true", colon) == std::string::npos;
            }
        }
        if (!_lobbyManager) {
            res.set_content(
                R"({"success":false,"error":"Lobby manager not available"})",
                "application/json");
            res.status = 500;
            return;
        }
        std::string code = _lobbyManager->createLobby(isPrivate);
        if (code.empty()) {
            res.set_content(
                R"({"success":false,"error":"Failed to create lobby"})",
                "application/json");
            res.status = 500;
        } else {
            std::ostringstream oss;
            oss << "{\"success\":true,\"code\":\"" << code << "\"}";
            res.set_content(oss.str(), "application/json");
            res.status = 200;
        }
    });

    server->Post("/api/lobby/:code/delete", [this](const Request& req,
                                                   Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        auto lobbyCode = req.path_params.at("code");
        if (!_lobbyManager) {
            res.set_content(
                R"({\"success\":false,\"error\":\"Lobby manager not available\"})",
                "application/json");
            res.status = 500;
            return;
        }
        LOG_INFO(
            std::string("[AdminServer] Lobby delete requested for code: [") +
            lobbyCode + "]");
        bool deleted = _lobbyManager->deleteLobby(lobbyCode);
        if (deleted) {
            res.set_content(R"({\"success\":true})", "application/json");
            res.status = 200;
        } else {
            res.set_content(
                R"({\"success\":false,\"error\":\"Lobby not found\"})",
                "application/json");
            res.status = 404;
        }
    });
}

void AdminServer::registerMetricsRoutes(void* serverPtr) {
    auto* server = static_cast<Server*>(serverPtr);
    server->Get("/api/metrics", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        if (!_serverApp) {
            res.set_content(R"({"error":"Server not available"})",
                            "application/json");
            res.status = 500;
            return;
        }
        auto json = buildMetricsJson();
        res.set_content(json, "application/json");
        res.status = 200;
    });

    server->Post("/api/metrics/reset", [this](const Request& req,
                                              Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        if (_serverApp) {
            const_cast<ServerMetrics&>(_serverApp->getMetrics()).clearHistory();
        }
        res.set_content(R"({"success":true})", "application/json");
        res.status = 200;
    });
}

std::string AdminServer::buildMetricsJson() const {
    if (!_serverApp) return R"({})";

    const auto& baseMetrics = _serverApp->getMetrics();
    auto uptime = baseMetrics.getUptimeSeconds();

    std::uint32_t totalPlayerCount = 0;
    std::uint32_t lobbyCount = 0;
    std::uint64_t totalPacketsReceived = 0;
    std::uint64_t totalPacketsSent = 0;
    std::uint64_t totalPacketsDropped = 0;
    std::uint64_t totalBytesReceived = 0;
    std::uint64_t totalBytesSent = 0;
    std::uint64_t totalTickOverruns = 0;
    std::uint64_t totalConnections = 0;
    std::uint64_t totalConnectionsRejected = 0;

    if (_lobbyManager) {
        auto lobbies = _lobbyManager->getAllLobbies();
        lobbyCount = static_cast<std::uint32_t>(lobbies.size());

        for (auto* lobby : lobbies) {
            if (!lobby) continue;

            totalPlayerCount += lobby->getPlayerCount();

            auto* serverApp = lobby->getServerApp();
            if (serverApp) {
                const auto& lobbyMetrics = serverApp->getMetrics();
                totalPacketsReceived += lobbyMetrics.packetsReceived.load(
                    std::memory_order_relaxed);
                totalPacketsSent +=
                    lobbyMetrics.packetsSent.load(std::memory_order_relaxed);
                totalPacketsDropped +=
                    lobbyMetrics.packetsDropped.load(std::memory_order_relaxed);
                totalBytesReceived +=
                    lobbyMetrics.bytesReceived.load(std::memory_order_relaxed);
                totalBytesSent +=
                    lobbyMetrics.bytesSent.load(std::memory_order_relaxed);
                totalTickOverruns +=
                    lobbyMetrics.tickOverruns.load(std::memory_order_relaxed);
                totalConnections += lobbyMetrics.totalConnections.load(
                    std::memory_order_relaxed);
                totalConnectionsRejected +=
                    lobbyMetrics.connectionsRejected.load(
                        std::memory_order_relaxed);
            }
        }
    }

    totalPacketsReceived +=
        baseMetrics.packetsReceived.load(std::memory_order_relaxed);
    totalPacketsSent += baseMetrics.packetsSent.load(std::memory_order_relaxed);
    totalPacketsDropped +=
        baseMetrics.packetsDropped.load(std::memory_order_relaxed);
    totalBytesReceived +=
        baseMetrics.bytesReceived.load(std::memory_order_relaxed);
    totalBytesSent += baseMetrics.bytesSent.load(std::memory_order_relaxed);
    totalTickOverruns +=
        baseMetrics.tickOverruns.load(std::memory_order_relaxed);
    totalConnections +=
        baseMetrics.totalConnections.load(std::memory_order_relaxed);
    totalConnectionsRejected +=
        baseMetrics.connectionsRejected.load(std::memory_order_relaxed);

    std::ostringstream oss;
    oss << R"({)"
        << R"("playerCount":)" << totalPlayerCount << ","
        << R"("uptime":)" << uptime << ","
        << R"("lobbyCount":)" << lobbyCount << ","
        << R"("packetsReceived":)" << totalPacketsReceived << ","
        << R"("packetsSent":)" << totalPacketsSent << ","
        << R"("packetsDropped":)" << totalPacketsDropped << ","
        << R"("bytesReceived":)" << totalBytesReceived << ","
        << R"("bytesSent":)" << totalBytesSent << ","
        << R"("tickOverruns":)" << totalTickOverruns << ","
        << R"("connectionsRejected":)" << totalConnectionsRejected << ","
        << R"("totalConnections":)" << totalConnections << ",";

    oss << R"("history":[)";
    auto history = baseMetrics.getHistory();
    for (size_t i = 0; i < history.size(); ++i) {
        if (i > 0) oss << ",";
        const auto& snap = history[i];
        auto time_since_epoch =
            snap.timestamp.time_since_epoch().count() / 1000000000;
        oss << "{"
            << R"("timestamp":)" << time_since_epoch << ","
            << R"("playerCount":)" << snap.playerCount << ","
            << R"("packetsReceived":)" << snap.packetsReceived << ","
            << R"("packetsSent":)" << snap.packetsSent << ","
            << R"("bytesReceived":)" << snap.bytesReceived << ","
            << R"("bytesSent":)" << snap.bytesSent << ","
            << R"("packetLossPercent":)" << snap.packetLossPercent << ","
            << R"("tickOverruns":)" << snap.tickOverruns << "}";
    }
    oss << R"(])";
    oss << R"(})";

    return oss.str();
}

void AdminServer::registerAdminPageRoutes(void* serverPtr) {
    auto* server = static_cast<Server*>(serverPtr);
    server->Get("/admin", [this](const Request& req, Response& res) {
        std::ifstream file("assets/admin.html");
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.set_content(R"(<h1>Admin Dashboard Not Found</h1>)",
                            "text/html");
            res.status = 404;
        }
    });
}

void AdminServer::registerLobbyRoutes(void* serverPtr) {
    auto* server = static_cast<Server*>(serverPtr);
    server->Get("/api/lobbies", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }

        std::ostringstream oss;
        oss << R"({"lobbies":[)";

        if (_lobbyManager) {
            auto lobbies = _lobbyManager->getActiveLobbyList();
            for (size_t i = 0; i < lobbies.size(); ++i) {
                if (i > 0) oss << ",";
                const auto& lobby = lobbies[i];
                bool isPublic = lobby.code.empty() ||
                                (lobby.code.find_first_not_of("0123456789") ==
                                 std::string::npos);
                oss << "{"
                    << R"("code":")" << lobby.code << R"(",)"
                    << R"("port":)" << lobby.port << ","
                    << R"("playerCount":)" << lobby.playerCount << ","
                    << R"("maxPlayers":)" << lobby.maxPlayers << ","
                    << R"("active":)" << (lobby.isActive ? "true" : "false")
                    << ","
                    << R"("isPublic":)" << (isPublic ? "true" : "false") << ","
                    << R"("difficulty":"Normal")"
                    << "}";
            }
        }

        oss << R"(]})";
        res.set_content(oss.str(), "application/json");
        res.status = 200;
    });

    server->Get("/api/lobbies/:code/players", [this](const Request& req,
                                                     Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }

        auto lobbyCode = req.path_params.at("code");
        std::ostringstream oss;
        oss << R"({"players":[)";

        if (_lobbyManager) {
            auto* lobby = _lobbyManager->findLobbyByCode(lobbyCode);
            if (lobby) {
                auto* serverApp = lobby->getServerApp();
                if (serverApp) {
                    auto clientIds = serverApp->getConnectedClientIds();
                    for (size_t i = 0; i < clientIds.size(); ++i) {
                        if (i > 0) oss << ",";
                        auto endpointOpt =
                            serverApp->getClientEndpoint(clientIds[i]);
                        oss << "{"
                            << R"("id":)" << clientIds[i] << ","
                            << R"("lobbyCode":")" << lobbyCode << R"(",)"
                            << R"("ip":")"
                            << (endpointOpt ? endpointOpt->address : "unknown")
                            << R"(",)"
                            << R"("ping":0,)"
                            << R"("isReady":true,)"
                            << R"("joined":)"
                            << std::chrono::system_clock::now()
                                       .time_since_epoch()
                                       .count() /
                                   1000000000
                            << "}";
                    }
                }
            }
        }

        oss << R"(]})";
        res.set_content(oss.str(), "application/json");
        res.status = 200;
    });

    server->Get("/api/players", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }

        std::ostringstream oss;
        oss << R"({"players":[)";
        bool first = true;

        if (_lobbyManager) {
            auto lobbies = _lobbyManager->getAllLobbies();
            for (auto* lobby : lobbies) {
                if (!lobby) continue;

                auto* serverApp = lobby->getServerApp();
                if (!serverApp) continue;

                auto clientIds = serverApp->getConnectedClientIds();
                for (auto clientId : clientIds) {
                    if (!first) oss << ",";
                    first = false;

                    auto endpointOpt = serverApp->getClientEndpoint(clientId);
                    oss << "{"
                        << R"("id":)" << clientId << ","
                        << R"("lobbyCode":")" << lobby->getCode() << R"(",)"
                        << R"("ip":")"
                        << (endpointOpt ? endpointOpt->address : "unknown")
                        << R"(",)"
                        << R"("ping":0,)"
                        << R"("isReady":true,)"
                        << R"("joined":)"
                        << std::chrono::system_clock::now()
                                   .time_since_epoch()
                                   .count() /
                               1000000000
                        << "}";
                }
            }
        }

        oss << R"(]})";
        res.set_content(oss.str(), "application/json");
        res.status = 200;
    });
}

void AdminServer::registerBanRoutes(void* serverPtr) {
    auto* server = static_cast<Server*>(serverPtr);
    server->Get("/api/bans", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        std::vector<BanManager::BannedEndpoint> bans;
        if (_lobbyManager) {
            bans = _lobbyManager->getBanManager()->getBannedList();
        } else if (_serverApp) {
            bans = _serverApp->getBanManager().getBannedList();
        }

        std::ostringstream oss;
        oss << R"({"bans":[)";
        for (size_t i = 0; i < bans.size(); ++i) {
            if (i > 0) oss << ",";
            const auto& b = bans[i];
            oss << "{"
                << R"("ip":")" << b.ip << R"(",)"
                << R"("port":)" << b.port << ","
                << R"("playerName":")" << b.playerName << R"(",)"
                << R"("reason":")" << b.reason << R"(")"
                << "}";
        }
        oss << "]}";
        res.set_content(oss.str(), "application/json");
        res.status = 200;
    });

    server->Post("/api/kick/:clientId", [this](const Request& req,
                                               Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        auto idStr = req.path_params.at("clientId");
        std::uint32_t clientId = 0;
        try {
            clientId = static_cast<std::uint32_t>(std::stoul(idStr));
        } catch (...) {
            clientId = 0;
        }
        bool kicked = false;
        if (_lobbyManager) {
            auto lobbies = _lobbyManager->getAllLobbies();
            for (auto* lobby : lobbies) {
                if (!lobby) continue;
                auto* serverApp = lobby->getServerApp();
                if (!serverApp) continue;
                auto ids = serverApp->getConnectedClientIds();
                if (std::find(ids.begin(), ids.end(), clientId) != ids.end()) {
                    kicked = serverApp->kickClient(clientId);
                    break;
                }
            }
        } else if (_serverApp) {
            kicked = _serverApp->kickClient(clientId);
        }
        if (kicked) {
            res.set_content(R"({"success":true})", "application/json");
            res.status = 200;
        } else {
            res.set_content(R"({"success":false,"error":"Client not found"})",
                            "application/json");
            res.status = 404;
        }
    });

    server->Post("/api/ban", [this, server](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        std::string body = req.body;
        std::string ip;
        std::uint16_t port = 0;
        std::uint32_t clientId = 0;
        std::string banReason = "Admin ban";
        if (auto pos = body.find("\"clientId\""); pos != std::string::npos) {
            auto colon = body.find(":", pos);
            if (colon != std::string::npos) {
                try {
                    clientId = static_cast<std::uint32_t>(
                        std::stoul(body.substr(colon + 1)));
                } catch (...) {
                }
            }
        }
        if (auto pos = body.find("\"ip\""); pos != std::string::npos) {
            auto start = body.find("\"", pos + 4);
            auto end = (start != std::string::npos) ? body.find("\"", start + 1)
                                                    : std::string::npos;
            if (start != std::string::npos && end != std::string::npos) {
                ip = body.substr(start + 1, end - start - 1);
            }
        }
        if (auto pos = body.find("\"port\""); pos != std::string::npos) {
            auto colon = body.find(":", pos);
            if (colon != std::string::npos) {
                try {
                    port = static_cast<std::uint16_t>(
                        std::stoul(body.substr(colon + 1)));
                } catch (...) {
                }
            }
        }
        if (auto pos = body.find("\"reason\""); pos != std::string::npos) {
            auto start = body.find("\"", pos + 8);
            auto end = (start != std::string::npos) ? body.find("\"", start + 1)
                                                    : std::string::npos;
            if (start != std::string::npos && end != std::string::npos) {
                banReason = body.substr(start + 1, end - start - 1);
            }
        }
        Endpoint ep;
        bool haveEndpoint = false;
        bool ipOnly = false;
        if (clientId != 0) {
            if (_lobbyManager) {
                auto lobbies = _lobbyManager->getAllLobbies();
                for (auto* lobby : lobbies) {
                    if (!lobby) continue;
                    auto* serverApp = lobby->getServerApp();
                    if (!serverApp) continue;
                    auto epOpt = serverApp->getClientEndpoint(clientId);
                    if (epOpt) {
                        ep = *epOpt;
                        haveEndpoint = true;
                        break;
                    }
                    auto clientInfoOpt = serverApp->getClientInfo(clientId);
                    if (clientInfoOpt) {
                        ep = clientInfoOpt->endpoint;
                        haveEndpoint = true;
                        break;
                    }
                }
            } else if (_serverApp) {
                auto epOpt = _serverApp->getClientEndpoint(clientId);
                if (epOpt) {
                    ep = *epOpt;
                    haveEndpoint = true;
                } else if (auto ci = _serverApp->getClientInfo(clientId)) {
                    ep = ci->endpoint;
                    haveEndpoint = true;
                }
            }
            LOG_INFO(std::string("[AdminServer] Ban request for clientId: ") +
                     std::to_string(clientId) +
                     ", resolved=" + (haveEndpoint ? "true" : "false") +
                     (haveEndpoint ? (std::string(", ep=") + ep.address + ":" +
                                      std::to_string(ep.port))
                                   : std::string("")));
        } else if (!ip.empty()) {
            ep.address = ip;
            ep.port = port;
            ipOnly = (port == 0);
        }
        if (!haveEndpoint && ip.empty()) {
            res.set_content(
                R"({"success":false,"error":"Endpoint not resolved"})",
                "application/json");
            res.status = 400;
            return;
        }
        auto applyBanAndDisconnect = [&](ServerApp* sa) {
            if (!sa) return;
            if (clientId != 0 || (!ip.empty() && !ipOnly && ep.port != 0)) {
                sa->getBanManager().banEndpoint(ep, "", banReason);
                auto ids = sa->getConnectedClientIds();
                for (auto id : ids) {
                    auto epOpt = sa->getClientEndpoint(id);
                    if (epOpt && epOpt->address == ep.address &&
                        epOpt->port == ep.port) {
                        sa->kickClient(id);
                    }
                }
            } else if (!ip.empty()) {
                sa->getBanManager().banIp(ip, "", banReason);
                auto ids = sa->getConnectedClientIds();
                for (auto id : ids) {
                    auto epOpt = sa->getClientEndpoint(id);
                    if (epOpt && epOpt->address == ip) {
                        sa->kickClient(id);
                    }
                }
            }
        };

        if (_lobbyManager) {
            auto lobbies = _lobbyManager->getAllLobbies();
            for (auto* lobby : lobbies) {
                applyBanAndDisconnect(lobby ? lobby->getServerApp() : nullptr);
            }
        } else if (_serverApp) {
            applyBanAndDisconnect(_serverApp);
        }
        res.set_content(R"({"success":true})", "application/json");
        res.status = 200;
    });
}
}  // namespace rtype::server
