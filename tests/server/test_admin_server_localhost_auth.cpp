#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerLocalhostAuth, LocalhostDeniedWithoutAuth) {
    AdminServer::Config cfg;
    cfg.localhostOnly = true;
    httplib::Request req;
    req.remote_addr = "127.0.0.1";
    EXPECT_FALSE(authenticateRequest(cfg, req, "u", "p"));
}

TEST(AdminServerLocalhostAuth, RemoteDeniedWithoutAuth) {
    AdminServer::Config cfg;
    cfg.localhostOnly = true;
    httplib::Request req;
    req.remote_addr = "8.8.8.8";
    EXPECT_FALSE(authenticateRequest(cfg, req, "u", "p"));
}

TEST(AdminServerLocalhostAuth, RemoteAllowedWithToken) {
    AdminServer::Config cfg;
    cfg.localhostOnly = true;
    cfg.token = "t";
    httplib::Request req;
    req.remote_addr = "8.8.8.8";
    req.headers["Authorization"] = "Bearer t";
    EXPECT_TRUE(authenticateRequest(cfg, req, "u", "p"));
}

TEST(AdminServerLocalhostAuth, RemoteAllowedWithBasic) {
    AdminServer::Config cfg;
    cfg.localhostOnly = true;
    httplib::Request req;
    req.remote_addr = "8.8.8.8";
    // "user:pass" => base64 = dXNlcjpwYXNz
    req.headers["Authorization"] = "Basic dXNlcjpwYXNz";
    EXPECT_TRUE(authenticateRequest(cfg, req, "user", "pass"));
}

TEST(AdminServerLocalhostAuth, RemoteAllowedWithCookie) {
    AdminServer::Config cfg;
    cfg.localhostOnly = true;
    cfg.sessionToken = "tkn";
    httplib::Request req;
    req.remote_addr = "8.8.8.8";
    req.headers["Cookie"] = std::string("admin_auth=") + cfg.sessionToken + "; other=ok";
    EXPECT_TRUE(authenticateRequest(cfg, req, "user", "pass"));
}

TEST(AdminServerLocalhostAuth, NotLocalhostModeRequiresAuth) {
    AdminServer::Config cfg;
    cfg.localhostOnly = false;
    httplib::Request req;
    req.remote_addr = "127.0.0.1";
    EXPECT_FALSE(authenticateRequest(cfg, req, "user", "pass"));
}

TEST(AdminServerLocalhostAuth, NotLocalhostModeAllowsBasicAuth) {
    AdminServer::Config cfg;
    cfg.localhostOnly = false;
    httplib::Request req;
    req.remote_addr = "192.0.2.1";
    req.headers["Authorization"] = "Basic dXNlcjpwYXNz"; // user:pass
    EXPECT_TRUE(authenticateRequest(cfg, req, "user", "pass"));
}
