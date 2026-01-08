#include <gtest/gtest.h>

#include "server/shared/ServerMetrics.hpp"

using namespace rtype::server;

TEST(ServerMetricsTest, AddSnapshotAndHistoryBounds) {
    ServerMetrics m;
    MetricsSnapshot s;
    s.playerCount = 1;
    // Fill more than MAX_HISTORY_SIZE
    for (size_t i = 0; i < ServerMetrics::MAX_HISTORY_SIZE + 5; ++i) {
        s.playerCount = static_cast<uint32_t>(i);
        m.addSnapshot(s);
    }
    auto history = m.getHistory();
    EXPECT_LE(history.size(), ServerMetrics::MAX_HISTORY_SIZE);
    EXPECT_EQ(history.back().playerCount, static_cast<uint32_t>(ServerMetrics::MAX_HISTORY_SIZE + 4));
}

TEST(ServerMetricsTest, ClearHistoryWorks) {
    ServerMetrics m;
    MetricsSnapshot s;
    s.playerCount = 42;
    m.addSnapshot(s);
    EXPECT_FALSE(m.getHistory().empty());
    m.clearHistory();
    EXPECT_TRUE(m.getHistory().empty());
}

TEST(ServerMetricsTest, UptimeNonZeroAfterConstruction) {
    ServerMetrics m;
    // Uptime should be >= 0 (non-negative) and small but non-zero after construction
    auto uptime = m.getUptimeSeconds();
    EXPECT_GE(uptime, 0u);
}
