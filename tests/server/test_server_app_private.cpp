#include <gtest/gtest.h>
#include "protocol/ByteOrderSpec.hpp"

// Private tests reference internal implementation and can cause linkage
// differences on MSVC/Windows builds. Guard the whole test file so that
// it is only enabled on non-Windows platforms where it has been validated.
#if !defined(_WIN32)

// Expose private members of ServerApp only while including its header
#define private public
#define protected public
#include "server/serverApp/ServerApp.hpp"
#undef private
#undef protected

using namespace rtype::server;
using namespace rtype::network;

class ServerAppPrivateTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

// Note: extractPacketFromData, getLoopTiming, performFixedUpdates, 
// calculateFrameTime and sleepUntilNextFrame have been moved to 
// ServerLoop and PacketProcessor classes. Those tests are now in
// test_ServerLoop.cpp.

TEST_F(ServerAppPrivateTest, Shutdown_OnlyPerformedOnce) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // First shutdown should complete successfully
    server.stop();
    EXPECT_FALSE(server.isRunning());

    // Second stop should be idempotent
    server.stop();
    EXPECT_FALSE(server.isRunning());
}

#else

// On Windows we disable these intrusive private tests to avoid linker issues
// with MSVC and keep a small placeholder test so the test suite behaves
// consistently across platforms.

TEST(WindowsPlaceholder, ServerAppPrivateTestsDisabledOnWindows) {
    SUCCEED();
}

#endif // !defined(_WIN32)
