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

#include <nlohmann/json.hpp>
#include <rtype/common.hpp>

#include "httplib.h"
#include "server/lobby/Lobby.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "server/serverApp/ServerApp.hpp"
using json = nlohmann::json;

namespace rtype::server {
using ::httplib::Request;
using ::httplib::Response;
using ::httplib::Server;

AdminServer::AdminServer(const Config& config, ServerApp* serverApp,
                         LobbyManager* lobbyManager)
    : _config(config),
      _serverApp(serverApp),
      _lobbyManager(lobbyManager),
      _httpServer(nullptr) {
    generateCredentials();
    static std::function<std::string(size_t)> makeToken = [](size_t length) {
        static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        static thread_local std::random_device rd;
        static thread_local std::mt19937 rng(rd());
        std::uniform_int_distribution<size_t> dist(0, std::strlen(alpha) - 1);
        std::string out;
        out.reserve(length);
        for (size_t i = 0; i < length; ++i) out.push_back(alpha[dist(rng)]);
        return out;
    };
    _config.sessionToken = makeToken(24);
}

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

    _serverThread = std::thread([this]() { runServer(); });

    const int attempts = 25;
    const auto sleepMs = std::chrono::milliseconds(20);
    for (int i = 0; i < attempts; ++i) {
        std::this_thread::sleep_for(sleepMs);
        if (_httpServer && static_cast<Server*>(_httpServer)->is_running()) {
            _running = true;
            LOG_INFO_CAT(::rtype::LogCategory::Network,
                         "[AdminServer] Started on port " << _config.port);
            return true;
        }
    }

    LOG_ERROR_CAT(::rtype::LogCategory::Network,
                  "[AdminServer] Failed to start on port "
                      << _config.port
                      << "; port may be in use or insufficient privileges");

    if (_serverThread.joinable()) {
        _serverThread.join();
    }

    if (_httpServer) {
        delete static_cast<Server*>(_httpServer);
        _httpServer = nullptr;
    }

    _running = false;
    return false;
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

void AdminServer::generateCredentials() {
    constexpr char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    constexpr char lower[] = "abcdefghijklmnopqrstuvwxyz";
    constexpr char digits[] = "0123456789";
    constexpr char special[] = "!@#$%^&*()-_+=";

    auto pick = [](const char* s) -> char {
        size_t len = std::strlen(s);
        static thread_local std::random_device rd;
        static thread_local std::mt19937 rng(rd());
        std::uniform_int_distribution<size_t> dist(0, len - 1);
        return s[dist(rng)];
    };

    auto make = [&](size_t length) {
        std::string out;
        out.reserve(length);
        out.push_back(pick(upper));
        out.push_back(pick(lower));
        out.push_back(pick(digits));
        out.push_back(pick(special));

        static const char allChars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$"
            "%^&*()-_+=";
        std::uniform_int_distribution<size_t> dist(0,
                                                   std::strlen(allChars) - 1);
        static thread_local std::random_device rd;
        static thread_local std::mt19937 rng(rd());
        while (out.size() < length) {
            out.push_back(allChars[dist(rng)]);
        }

        std::shuffle(out.begin(), out.end(), rng);
        return out;
    };

    _adminUser = make(12);
    _adminPass = make(16);

    if (_adminUser == _adminPass) {
        _adminPass = make(16);
        if (_adminUser == _adminPass) {
            _adminPass[0] = (_adminPass[0] == 'X') ? 'Y' : 'X';
        }
    }

    LOG_INFO_CAT(::rtype::LogCategory::Network,
                 "[AdminServer] Generated admin credentials: user='"
                     << _adminUser << "' pass='" << _adminPass << "'");
}

void AdminServer::runServer() noexcept {
    if (_httpServer) {
        const char* bindAddr = _config.localhostOnly ? "127.0.0.1" : "0.0.0.0";
        bool ok =
            static_cast<Server*>(_httpServer)->listen(bindAddr, _config.port);
        if (!ok) {
            LOG_ERROR_CAT(::rtype::LogCategory::Network,
                          "[AdminServer] listen() failed on " << bindAddr << ":"
                                                              << _config.port);
        }
    }
}

static std::string file_base64Decode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    auto is_base64 = [](unsigned char c) -> bool {
        return (isalnum(c) || (c == '+') || (c == '/'));
    };

    std::string ret;
    int in_len = static_cast<int>(input.size());
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (input[in_] != '=') && is_base64(input[in_])) {
        char_array_4[i++] = input[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = static_cast<unsigned char>(
                    base64_chars.find(char_array_4[i]));

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        int j;
        for (j = i; j < 4; j++) char_array_4[j] = 0;
        for (j = 0; j < 4; j++)
            char_array_4[j] =
                static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
        char_array_3[0] =
            (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

static std::string file_urlDecode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size()) {
            char hi = s[i + 1];
            char lo = s[i + 2];
            auto fromHex = [](char ch) -> int {
                if (ch >= '0' && ch <= '9') return ch - '0';
                if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
                if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
                return -1;
            };
            int hiV = fromHex(hi);
            int loV = fromHex(lo);
            if (hiV >= 0 && loV >= 0) {
                out.push_back(static_cast<char>((hiV << 4) | loV));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

bool authenticateRequest(const AdminServer::Config& config, const Request& req,
                         const std::string& adminUser,
                         const std::string& adminPass) {
    const auto& remote_addr = req.remote_addr;
    bool localhost = remote_addr.empty() || remote_addr == "localhost" ||
                     remote_addr.find("127.0.0.1") != std::string::npos ||
                     remote_addr.find("::1") != std::string::npos;

    (void)localhost;

    if (!config.token.empty()) {
        const auto& auth = req.get_header_value("Authorization");
        if (!auth.empty() && auth == ("Bearer " + config.token)) return true;
    }

    const auto cookie = req.get_header_value("Cookie");
    if (!cookie.empty() && !config.sessionToken.empty()) {
        std::string key = std::string("admin_auth=") + config.sessionToken;
        if (cookie.find(key) != std::string::npos) return true;
    }

    const auto auth = req.get_header_value("Authorization");
    if (!auth.empty()) {
        constexpr const char* kBasicPrefix = "Basic ";
        if (auth.rfind(kBasicPrefix, 0) == 0) {
            std::string payload = auth.substr(std::strlen(kBasicPrefix));
            std::string decoded = file_base64Decode(payload);
            if (!decoded.empty()) {
                auto pos = decoded.find(':');
                if (pos != std::string::npos) {
                    std::string user = decoded.substr(0, pos);
                    std::string pass = decoded.substr(pos + 1);
                    return (user == adminUser && pass == adminPass);
                }
            }
        }
    }

    return false;
}

void AdminServer::setupRoutes() {  // NOLINT(readability/fn_size)
    auto* server = static_cast<Server*>(_httpServer);

    registerAdminPageRoutes(server);

    registerMetricsRoutes(server);

    registerLobbyRoutes(server);

    registerBanRoutes(server);

    server->Post("/api/unban", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }

        std::string ip;
        std::uint16_t port = 0;

        try {
            auto j = json::parse(req.body);
            if (!j.contains("ip") || !j["ip"].is_string()) {
                res.set_content(
                    R"({"success":false,"error":"Missing or invalid 'ip'"})",
                    "application/json");
                res.status = 400;
                return;
            }
            ip = j["ip"].get<std::string>();
            if (j.contains("port") && j["port"].is_number_unsigned()) {
                port = static_cast<std::uint16_t>(j["port"].get<unsigned>());
            }
        } catch (const json::parse_error&) {
            res.set_content(R"({"success":false,"error":"Malformed JSON"})",
                            "application/json");
            res.status = 400;
            return;
        } catch (...) {
            res.set_content(R"({"success":false,"error":"Invalid request"})",
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }

        bool isPrivate = true;
        try {
            if (!req.body.empty()) {
                auto j = json::parse(req.body);
                if (j.contains("isPublic") && j["isPublic"].is_boolean()) {
                    isPrivate = !j["isPublic"].get<bool>();
                }
            }
        } catch (const json::parse_error&) {
            res.set_content(R"({"success":false,"error":"Malformed JSON"})",
                            "application/json");
            res.status = 400;
            return;
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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

    server->Get("/admin/login", [this](const Request& req, Response& res) {
        std::string errorMsg;
        try {
            if (req.has_param("error") && req.get_param_value("error") == "1") {
                errorMsg = "Invalid username or password.";
            }
        } catch (...) {
        }

        std::ostringstream s;
        s << R"(<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <title>Admin Login</title>
  <style>
    body { font-family: Arial, Helvetica, sans-serif; background: #f5f7fb; color: #222; }
    .login { max-width: 380px; margin: 6vh auto; background: #fff; padding: 24px; border-radius: 8px; box-shadow: 0 6px 18px rgba(12,18,26,0.08); }
    h1 { margin: 0 0 12px 0; font-size: 20px; }
    label { display:block; margin: 8px 0 2px 0; font-size: 13px; }
    input[type=text], input[type=password] { width:100%; padding:10px; border:1px solid #dfe6ef; border-radius:4px; box-sizing:border-box; }
    .submit { margin-top: 14px; background:#2b90ff; color:white; border:none; padding:10px 14px; border-radius:4px; cursor:pointer; }
    .submit:hover { background:#1a74d1; }
    .msg { margin: 12px 0; padding: 10px; border-radius: 4px; background: #fff4f4; color: #9a1d1d; border:1px solid #f2c0c0; }
    .hint { margin-top:12px; font-size:12px; color:#555; }
  </style>
</head>
<body>
  <div class="login">
    <h1>Admin Login</h1>
)";
        if (!errorMsg.empty()) {
            s << "    <div class=\"msg\">" << errorMsg << "</div>\n";
        }
        s << R"(    <form action="/admin/login" method="post">
      <label>Username</label>
      <input type="text" name="username" placeholder="admin username" required>
      <label>Password</label>
      <input type="password" name="password" placeholder="admin password" required>
      <input class="submit" type="submit" value="Login">
    </form>
    <div class="hint">Credentials are generated when the server starts and printed once to the server console; they are case-sensitive.</div>
  </div>
</body>
</html>)";

        res.set_content(s.str(), "text/html");
        res.status = 200;
    });

    server->Post("/admin/login", [this](const Request& req, Response& res) {
        auto parse = [](const std::string& s) {
            std::unordered_map<std::string, std::string> m;
            size_t pos = 0;
            while (pos < s.size()) {
                auto eq = s.find('=', pos);
                if (eq == std::string::npos) break;
                auto key = s.substr(pos, eq - pos);
                auto amp = s.find('&', eq + 1);
                auto val = s.substr(eq + 1, (amp == std::string::npos)
                                                ? std::string::npos
                                                : amp - (eq + 1));
                m[file_urlDecode(key)] = file_urlDecode(val);
                if (amp == std::string::npos) break;
                pos = amp + 1;
            }
            return m;
        };

        auto params = parse(req.body);
        const auto itU = params.find("username");
        const auto itP = params.find("password");
        if (itU == params.end() || itP == params.end()) {
            res.set_content("Missing username or password", "text/plain");
            res.status = 400;
            return;
        }

        LOG_INFO_CAT(
            ::rtype::LogCategory::Network,
            "[AdminServer] Login attempt for user='" << itU->second << "'");

        if (itU->second == _adminUser && itP->second == _adminPass) {
            std::string cookie = std::string("admin_auth=") + _config.sessionToken + "; HttpOnly; Path=/";
            res.set_header("Set-Cookie", cookie);
            res.status = 302;
            res.set_header("Location", "/admin");
            LOG_INFO_CAT(::rtype::LogCategory::Network,
                         "[AdminServer] Admin login successful for user='"
                             << itU->second << "'");
            return;
        }

        LOG_INFO_CAT(::rtype::LogCategory::Network,
                     "[AdminServer] Admin login failed for user='"
                         << itU->second << "'");
        res.status = 302;
        res.set_header("Location", "/admin/login?error=1");
    });

    server->Get("/admin", [this](const Request& req, Response& res) {
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
            res.status = 302;
            res.set_header("Location", "/admin/login");
            return;
        }

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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
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
        if (!authenticateRequest(_config, req, _adminUser, _adminPass)) {
            res.set_content(R"({"error":"Unauthorized"})", "application/json");
            res.status = 401;
            return;
        }
        std::string ip;
        std::uint16_t port = 0;
        std::uint32_t clientId = 0;
        std::string banReason = "Admin ban";

        try {
            if (!req.body.empty()) {
                auto j = json::parse(req.body);
                if (j.contains("clientId") &&
                    j["clientId"].is_number_unsigned()) {
                    clientId = j["clientId"].get<std::uint32_t>();
                }
                if (j.contains("ip") && j["ip"].is_string()) {
                    ip = j["ip"].get<std::string>();
                }
                if (j.contains("port") && j["port"].is_number_unsigned()) {
                    port =
                        static_cast<std::uint16_t>(j["port"].get<unsigned>());
                }
                if (j.contains("reason") && j["reason"].is_string()) {
                    banReason = j["reason"].get<std::string>();
                }
            }
        } catch (const json::parse_error&) {
            res.set_content(R"({"success":false,"error":"Malformed JSON"})",
                            "application/json");
            res.status = 400;
            return;
        } catch (...) {
            res.set_content(R"({"success":false,"error":"Invalid request"})",
                            "application/json");
            res.status = 400;
            return;
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
