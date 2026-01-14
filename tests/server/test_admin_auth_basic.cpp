#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "httplib.h"

using namespace rtype::server;

static std::string base64Encode(const std::string& in) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

static std::string urlEncode(const std::string& s) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : s) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::uppercase << std::setw(2) << int((unsigned char)c) << std::nouppercase;
        }
    }

    return escaped.str();
}

TEST(AdminServerAuthBasic, BasicAuth_AllowsBansEndpoint) {
    AdminServer::Config cfg;
    cfg.port = 9301;
    cfg.token = ""; // no bearer token
    cfg.localhostOnly = false; // require auth

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    // Get generated credentials
    auto user = server.getAdminUserForTests();
    auto pass = server.getAdminPassForTests();
    ASSERT_FALSE(user.empty());
    ASSERT_FALSE(pass.empty());

    // Build Basic auth header
    std::string creds = user + ":" + pass;
    std::string b64 = base64Encode(creds);
    httplib::Headers auth{{"Authorization", std::string("Basic ") + b64}};

    httplib::Client cli("127.0.0.1", cfg.port);
    auto res = cli.Get("/api/bans", auth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"bans\":"), std::string::npos);

    server.stop();
}

TEST(AdminServerAuthBasic, LoginSetsCookie_ThenCookieAllowsAdminPage) {
    AdminServer::Config cfg;
    cfg.port = 9302;
    cfg.token = "";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    auto user = server.getAdminUserForTests();
    auto pass = server.getAdminPassForTests();

    httplib::Client cli("127.0.0.1", cfg.port);

    std::string body = std::string("username=") + urlEncode(user) + "&password=" + urlEncode(pass);
    auto res = cli.Post("/admin/login", body, "application/x-www-form-urlencoded");
    ASSERT_NE(res, nullptr);
    // Successful login should redirect to /admin
    EXPECT_EQ(res->status, 302);

    // Now access /admin with cookie header using the session token issued by the server
    auto token = server.getSessionTokenForTests();
    httplib::Headers cookie{{"Cookie", std::string("admin_auth=") + token}};
    auto res2 = cli.Get("/admin", cookie);
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 200);
    EXPECT_NE(res2->body.find("<html"), std::string::npos);

    server.stop();
}

TEST(AdminServerAuthBasic, LoginFailure_RedirectsToErrorAndShowsMessage) {
    AdminServer::Config cfg;
    cfg.port = 9303;
    cfg.token = "";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());

    httplib::Client cli("127.0.0.1", cfg.port);
    std::string body = "username=wrong&password=wrong";
    auto res = cli.Post("/admin/login", body, "application/x-www-form-urlencoded");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 302);
    auto loc = res->get_header_value("Location");
    EXPECT_NE(loc.find("error=1"), std::string::npos);

    auto res2 = cli.Get("/admin/login?error=1");
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 200);

    server.stop();
}

TEST(AdminServerAuthBasic, AdminPage_Unauthenticated_RedirectsToLogin) {
    AdminServer::Config cfg;
    cfg.port = 9304;
    cfg.token = "";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());

    httplib::Client cli("127.0.0.1", cfg.port);
    auto res = cli.Get("/admin");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 302);
    EXPECT_EQ(res->get_header_value("Location"), "/admin/login");

    server.stop();
}

TEST(AdminServerAuthBasic, BearerToken_AllowsMetrics_ButServerMissing_Returns500) {
    AdminServer::Config cfg;
    cfg.port = 9305;
    cfg.token = "secrettoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers auth{{"Authorization", std::string("Bearer ") + cfg.token}};
    auto res = cli.Get("/api/metrics", auth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    httplib::Headers bad{{"Authorization", std::string("Bearer wrong")}};
    auto res2 = cli.Get("/api/metrics", bad);
    ASSERT_NE(res2, nullptr);
    EXPECT_EQ(res2->status, 401);

    server.stop();
}

TEST(AdminServerAuthBasic, LocalhostOnly_RequiresAuth) {
    AdminServer::Config cfg;
    cfg.port = 9306;
    cfg.token = "";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());

    httplib::Client cli("127.0.0.1", cfg.port);
    auto res = cli.Get("/api/bans");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);

    server.stop();
}
