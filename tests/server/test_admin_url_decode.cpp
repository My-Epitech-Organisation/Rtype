#include "server/shared/AdminServer.hpp"
#include <gtest/gtest.h>

using namespace rtype::server;

TEST(AdminUrlDecode, BasicPlusAndPercent) {
    EXPECT_EQ(urlDecodeForAdminTests("a+b"), "a b");
    EXPECT_EQ(urlDecodeForAdminTests("foo%20bar"), "foo bar");
    EXPECT_EQ(urlDecodeForAdminTests("percent%21"), "percent!");
}

TEST(AdminUrlDecode, MalformedPercent) {
    // If percent-encoding is malformed we keep the literal percent
    EXPECT_EQ(urlDecodeForAdminTests("bad%ZZ"), "bad%ZZ");
}

TEST(AdminFormParsing, Complex) {
    // Simulate a full urlencoded body
    std::string body = "username=My%2BUser%21&password=p%40ss%23word";
    // naive parse
    auto parse = [](const std::string& s) {
        std::unordered_map<std::string, std::string> m;
        size_t pos = 0;
        while (pos < s.size()) {
            auto eq = s.find('=', pos);
            if (eq == std::string::npos) break;
            auto key = s.substr(pos, eq - pos);
            auto amp = s.find('&', eq + 1);
            auto val = s.substr(eq + 1, (amp == std::string::npos) ? std::string::npos : amp - (eq + 1));
            m[urlDecodeForAdminTests(key)] = urlDecodeForAdminTests(val);
            if (amp == std::string::npos) break;
            pos = amp + 1;
        }
        return m;
    };

    auto m = parse(body);
    EXPECT_EQ(m["username"], "My+User!");
    EXPECT_EQ(m["password"], "p@ss#word");
}
