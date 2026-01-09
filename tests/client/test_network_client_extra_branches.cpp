/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Extra branch coverage tests for NetworkClient
*/

#include <gtest/gtest.h>
#include <memory>
#include <cstring>

#include "NetworkClient.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "Serializer.hpp"
#include "transport/IAsyncSocket.hpp"

using namespace rtype::client;
using namespace rtype::network;

class MockSocket : public IAsyncSocket {
public:
    MockSocket() : open_(true) {}
    Result<void> bind(std::uint16_t) override { return Ok(); }
    bool isOpen() const noexcept override { return open_; }
    std::uint16_t localPort() const noexcept override { return 4242; }
    void asyncSendTo(const Buffer& data, const Endpoint&, SendCallback handler) override {
        lastSent_ = data;
        if (handler) handler(Ok(data.size()));
    }
    void asyncReceiveFrom(std::shared_ptr<Buffer> buffer, std::shared_ptr<Endpoint> sender, ReceiveCallback handler) override {
        pendingReceive_ = {buffer, sender, handler};
    }
    void pushIncoming(const std::vector<std::uint8_t>& pkt, const Endpoint& ep) {
        if (pendingReceive_.handler) {
            auto b = pendingReceive_.buffer;
            auto s = pendingReceive_.sender;
            auto h = pendingReceive_.handler;
            pendingReceive_ = {};
            b->resize(pkt.size());
            std::memcpy(b->data(), pkt.data(), pkt.size());
            if (s) *s = ep;
            if (h) h(Ok(pkt.size()));
        }
    }
    void cancel() override {}
    void close() override { open_ = false; }
    
    std::vector<std::uint8_t> lastSent_;
    
private:
    struct Pending {
        std::shared_ptr<Buffer> buffer;
        std::shared_ptr<Endpoint> sender;
        ReceiveCallback handler;
    } pendingReceive_;
    bool open_;
};

static Buffer buildPacket(OpCode opcode, const Buffer& payload, uint32_t userId = 1) {
    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(opcode);
    header.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    header.userId = ByteOrderSpec::toNetwork(userId);
    header.seqId = 0;
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};
    Buffer pkt(kHeaderSize + payload.size());
    std::memcpy(pkt.data(), &header, kHeaderSize);
    if (!payload.empty()) std::memcpy(pkt.data() + kHeaderSize, payload.data(), payload.size());
    return pkt;
}

TEST(NetworkClientExtraBranchTest, SendChatMessageSuccess) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_TRUE(client.sendChatMessage("test"));
}

TEST(NetworkClientExtraBranchTest, PingSuccess) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_TRUE(client.ping());
}

TEST(NetworkClientExtraBranchTest, DisconnectWhenConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_TRUE(client.isConnected());
    client.disconnect();
    EXPECT_FALSE(client.isConnected());
}

TEST(NetworkClientExtraBranchTest, ValidGameStart) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    float receivedCountdown = 0.0f;
    client.onGameStart([&](float countdown) { receivedCountdown = countdown; });
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    GameStartPayload gs;
    gs.countdownDuration = ByteOrderSpec::toNetwork(3.0f);
    Buffer gsPayload(sizeof(GameStartPayload));
    std::memcpy(gsPayload.data(), &gs, sizeof(gs));
    mockPtr->pushIncoming(buildPacket(OpCode::S_GAME_START, gsPayload), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_FLOAT_EQ(receivedCountdown, 3.0f);
}

TEST(NetworkClientExtraBranchTest, ValidPlayerReadyState) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    uint32_t receivedUserId = 0;
    bool receivedReady = false;
    client.onPlayerReadyStateChanged([&](uint32_t userId, bool ready) {
        receivedUserId = userId;
        receivedReady = ready;
    });
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    PlayerReadyStatePayload pr;
    pr.userId = ByteOrderSpec::toNetwork(10u);
    pr.isReady = 1;
    Buffer prPayload(sizeof(PlayerReadyStatePayload));
    std::memcpy(prPayload.data(), &pr, sizeof(pr));
    mockPtr->pushIncoming(buildPacket(OpCode::S_PLAYER_READY_STATE, prPayload), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_EQ(receivedUserId, 10u);
    EXPECT_TRUE(receivedReady);
}

TEST(NetworkClientExtraBranchTest, ValidUpdateState) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    GameStateEvent receivedState{};
    bool stateCalled = false;
    client.onGameStateChange([&](GameStateEvent state) {
        stateCalled = true;
        receivedState = state;
    });
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    UpdateStatePayload us;
    us.state = static_cast<std::uint8_t>(GameState::Playing);
    Buffer usPayload(sizeof(UpdateStatePayload));
    std::memcpy(usPayload.data(), &us, sizeof(us));
    mockPtr->pushIncoming(buildPacket(OpCode::S_UPDATE_STATE, usPayload), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_TRUE(stateCalled);
}

TEST(NetworkClientExtraBranchTest, ValidEntityDestroy) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    uint32_t destroyedId = 0;
    client.onEntityDestroy([&](uint32_t id) { destroyedId = id; });
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    EntityDestroyPayload ed;
    ed.entityId = ByteOrderSpec::toNetwork(999u);
    Buffer edPayload(sizeof(EntityDestroyPayload));
    std::memcpy(edPayload.data(), &ed, sizeof(ed));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ENTITY_DESTROY, edPayload), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_EQ(destroyedId, 999u);
}

TEST(NetworkClientExtraBranchTest, ValidEntityHealth) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    EntityHealthEvent receivedHealth{};
    bool healthCalled = false;
    client.onEntityHealth([&](EntityHealthEvent event) {
        healthCalled = true;
        receivedHealth = event;
    });
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    EntityHealthPayload eh;
    eh.entityId = ByteOrderSpec::toNetwork(555u);
    eh.current = ByteOrderSpec::toNetwork(75);
    eh.max = ByteOrderSpec::toNetwork(100);
    Buffer ehPayload(sizeof(EntityHealthPayload));
    std::memcpy(ehPayload.data(), &eh, sizeof(eh));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ENTITY_HEALTH, ehPayload), {"127.0.0.1", 4242});
    client.poll();
    
    EXPECT_TRUE(healthCalled);
    EXPECT_EQ(receivedHealth.entityId, 555u);
    EXPECT_EQ(receivedHealth.current, 75);
    EXPECT_EQ(receivedHealth.max, 100);
}

TEST(NetworkClientExtraBranchTest, ConnectThenMultipleSends) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();
    NetworkClient client(cfg, std::move(mock), false);
    
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(42u)};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();
    
    // Multiple operations
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(client.sendInput(static_cast<uint8_t>(i)));
        EXPECT_TRUE(client.ping());
        EXPECT_TRUE(client.sendReady(i % 2 == 0));
        client.poll();
    }
}
