#include <gtest/gtest.h>

#include <memory>
#include <queue>
#include <vector>

#include "NetworkClient.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "Serializer.hpp"
#include "transport/IAsyncSocket.hpp"

using namespace rtype::client;
using namespace rtype::network;

// Simple mock socket for synchronous testing
class MockSocket : public IAsyncSocket {
   public:
    MockSocket() : open_(true) {}

    Result<void> bind(std::uint16_t) override { return Ok(); }
    bool isOpen() const noexcept override { return open_; }
    std::uint16_t localPort() const noexcept override { return 4242; }

    void asyncSendTo(const Buffer& data, const Endpoint& dest,
                     SendCallback handler) override {
        lastSent_ = data;
        lastDest_ = dest;
        if (handler) handler(Ok(data.size()));
    }

    void asyncReceiveFrom(std::shared_ptr<Buffer> buffer,
                          std::shared_ptr<Endpoint> sender,
                          ReceiveCallback handler) override {
        // If we have queued incoming, deliver immediately, otherwise stash
        if (!incoming_.empty()) {
            auto pkt = incoming_.front();
            incoming_.pop();
            buffer->resize(pkt.first.size());
            std::memcpy(buffer->data(), pkt.first.data(), pkt.first.size());
            if (sender) *sender = pkt.second;
            if (handler) handler(Ok(pkt.first.size()));
        } else {
            pendingReceive_ = {buffer, sender, handler};
        }
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
            return;
        }
        incoming_.push({pkt, ep});
    }

    void cancel() override {}
    void close() override { open_ = false; }

    std::vector<std::uint8_t> lastSent_;
    Endpoint lastDest_;

   private:
    struct Pending {
        std::shared_ptr<Buffer> buffer;
        std::shared_ptr<Endpoint> sender;
        ReceiveCallback handler;
    } pendingReceive_;

    bool open_;
    std::queue<std::pair<std::vector<std::uint8_t>, Endpoint>> incoming_;
};

static Buffer buildPacket(OpCode opcode, const Buffer& payload, uint32_t userId = 1) {
    Header header;
    header.magic = kMagicByte;
    header.opcode = static_cast<std::uint8_t>(opcode);
    header.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    header.userId = ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(userId));
    header.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
    header.ackId = 0;
    header.flags = 0;
    header.reserved = {0, 0, 0};

    Buffer pkt(kHeaderSize + payload.size());
    std::memcpy(pkt.data(), &header, kHeaderSize);
    if (!payload.empty()) std::memcpy(pkt.data() + kHeaderSize, payload.data(), payload.size());
    return pkt;
}

TEST(NetworkClientTest, ConnectAndAcceptInvokesOnConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool called = false;
    std::uint32_t gotId = 0;
    client.onConnected([&](std::uint32_t myId) {
        called = true;
        gotId = myId;
    });

    // Start connect (will call bind and startReceive)
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));

    // Simulate server S_ACCEPT packet
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(42))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    auto pkt = buildPacket(OpCode::S_ACCEPT, payload, /*userId=*/0);

    Endpoint ep{"127.0.0.1", 4242};
    mockPtr->pushIncoming(pkt, ep);

    // Poll to dispatch callbacks
    client.poll();

    EXPECT_TRUE(called);
    EXPECT_EQ(gotId, 42u);
    EXPECT_TRUE(client.isConnected());
}

TEST(NetworkClientTest, SendInputSendsPacketWhenConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    auto pkt = buildPacket(OpCode::S_ACCEPT, payload, 0);
    Endpoint ep{"127.0.0.1", 4242};
    mockPtr->pushIncoming(pkt, ep);
    client.poll();

    EXPECT_TRUE(client.isConnected());
    // Now send input
    EXPECT_TRUE(client.sendInput(InputMask::kUp | InputMask::kShoot));
    // Verify last sent opcode is C_INPUT
    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header h; std::memcpy(&h, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(h.opcode), OpCode::C_INPUT);
}

TEST(NetworkClientTest, EntitySpawnAndDestroyCallbacks) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool spawnCalled = false;
    std::uint32_t spawnedId = 0;
    client.onEntitySpawn([&](EntitySpawnEvent ev) {
        spawnCalled = true;
        spawnedId = ev.entityId;
    });

    std::vector<std::uint32_t> destroyed;
    client.onEntityDestroy([&](std::uint32_t id) { destroyed.push_back(id); });

    // Accept connection
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(9))};
    Buffer acc(sizeof(AcceptPayload)); std::memcpy(acc.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, acc, 0), {"127.0.0.1", 4242});
    client.poll();

    // Send spawn packet (reliable) to exercise ACK path
    EntitySpawnPayload spawn{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(123)), static_cast<std::uint8_t>(EntityType::Player), 1.0f, 2.0f};
    Buffer sp(sizeof(EntitySpawnPayload)); std::memcpy(sp.data(), &spawn, sizeof(spawn));
    auto pkt_spawn = buildPacket(OpCode::S_ENTITY_SPAWN, sp, 0);
    Header hspawn; std::memcpy(&hspawn, pkt_spawn.data(), kHeaderSize);
    hspawn.flags |= Flags::kReliable;
    hspawn.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(55));
    std::memcpy(pkt_spawn.data(), &hspawn, kHeaderSize);
    mockPtr->pushIncoming(pkt_spawn, {"127.0.0.1", 4242});

    // Deliver first packet and check ACK sent
    client.poll();
    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header ackHdr; std::memcpy(&ackHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(ackHdr.opcode), OpCode::ACK);

    // Send move packet
    bool moveCalled = false; EntityMoveEvent gotMove{};
    client.onEntityMove([&](EntityMoveEvent e){ moveCalled = true; gotMove = e; });
    EntityMovePayload move{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(123)), 3.0f, 4.0f, 0.5f, -0.5f};
    Buffer mp(sizeof(EntityMovePayload)); std::memcpy(mp.data(), &move, sizeof(move));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ENTITY_MOVE, mp, 0), {"127.0.0.1", 4242});

    // Health
    bool healthCalled = false; EntityHealthEvent gotHealth{};
    client.onEntityHealth([&](EntityHealthEvent e){ healthCalled = true; gotHealth = e; });
    EntityHealthPayload eh{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(123)), ByteOrderSpec::toNetwork(static_cast<std::int32_t>(10)), ByteOrderSpec::toNetwork(static_cast<std::int32_t>(20))};
    Buffer ehb(sizeof(EntityHealthPayload)); std::memcpy(ehb.data(), &eh, sizeof(eh));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ENTITY_HEALTH, ehb, 0), {"127.0.0.1", 4242});

    // Powerup
    bool powerCalled = false; PowerUpEvent gotPower{};
    client.onPowerUpEvent([&](PowerUpEvent p){ powerCalled = true; gotPower = p; });
    PowerUpEventPayload pu{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(9)), 2, 5.0f};
    Buffer pub(sizeof(PowerUpEventPayload)); std::memcpy(pub.data(), &pu, sizeof(pu));
    mockPtr->pushIncoming(buildPacket(OpCode::S_POWERUP_EVENT, pub, 0), {"127.0.0.1", 4242});

    // Position
    bool posCalled = false; float gotX = 0, gotY = 0;
    client.onPositionCorrection([&](float x,float y){ posCalled = true; gotX = x; gotY = y; });
    UpdatePosPayload up{7.5f, -2.5f};
    Buffer upb(sizeof(UpdatePosPayload)); std::memcpy(upb.data(), &up, sizeof(up));
    mockPtr->pushIncoming(buildPacket(OpCode::S_UPDATE_POS, upb, 0), {"127.0.0.1", 4242});

    // State
    bool stateCalled = false; GameStateEvent gotState{};
    client.onGameStateChange([&](GameStateEvent e){ stateCalled = true; gotState = e; });
    UpdateStatePayload us{static_cast<std::uint8_t>(GameState::Running)};
    Buffer usb(sizeof(UpdateStatePayload)); std::memcpy(usb.data(), &us, sizeof(us));
    mockPtr->pushIncoming(buildPacket(OpCode::S_UPDATE_STATE, usb, 0), {"127.0.0.1", 4242});

    // Game over
    bool goCalled = false; GameOverEvent gotGO{};
    client.onGameOver([&](GameOverEvent e){ goCalled = true; gotGO = e; });
    GameOverPayload go{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(9000))};
    Buffer gob(sizeof(GameOverPayload)); std::memcpy(gob.data(), &go, sizeof(go));
    mockPtr->pushIncoming(buildPacket(OpCode::S_GAME_OVER, gob, 0), {"127.0.0.1", 4242});

    // Game start
    bool gsCalled = false; float gotDuration = 0;
    client.onGameStart([&](float d){ gsCalled = true; gotDuration = d; });
    GameStartPayload gs{3.5f};
    Buffer gsb(sizeof(GameStartPayload)); std::memcpy(gsb.data(), &gs, sizeof(gs));
    mockPtr->pushIncoming(buildPacket(OpCode::S_GAME_START, gsb, 0), {"127.0.0.1", 4242});

    // Player ready state
    bool prsCalled = false; uint32_t readyUser = 0; bool readyState = false;
    client.onPlayerReadyStateChanged([&](std::uint32_t uid, bool r){ prsCalled = true; readyUser = uid; readyState = r; });
    PlayerReadyStatePayload prs{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(9)), 1};
    Buffer prsb(sizeof(PlayerReadyStatePayload)); std::memcpy(prsb.data(), &prs, sizeof(prs));
    mockPtr->pushIncoming(buildPacket(OpCode::S_PLAYER_READY_STATE, prsb, 0), {"127.0.0.1", 4242});

    // Destroy
    EntityDestroyPayload d{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(123))};
    Buffer dp(sizeof(EntityDestroyPayload)); std::memcpy(dp.data(), &d, sizeof(d));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ENTITY_DESTROY, dp, 0), {"127.0.0.1", 4242});

    // Deliver queued packets
    client.poll();

    EXPECT_TRUE(moveCalled);
    EXPECT_TRUE(healthCalled);
    EXPECT_TRUE(powerCalled);
    EXPECT_TRUE(posCalled);
    EXPECT_TRUE(stateCalled);
    EXPECT_TRUE(goCalled);
    EXPECT_TRUE(gsCalled);
    EXPECT_TRUE(prsCalled);
    EXPECT_TRUE(spawnCalled);
    EXPECT_EQ(spawnedId, 123u);
    ASSERT_EQ(destroyed.size(), 1u);
    EXPECT_EQ(destroyed[0], 123u);

    EXPECT_TRUE(spawnCalled);
    EXPECT_EQ(spawnedId, 123u);
    ASSERT_EQ(destroyed.size(), 1u);
    EXPECT_EQ(destroyed[0], 123u);
}

TEST(NetworkClientTest, ProcessIncomingTooSmallPacketIgnored) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Clear any sends
    mockPtr->lastSent_.clear();

    // Push a tiny packet smaller than header
    std::vector<std::uint8_t> smallPkt(2, 0x00);
    mockPtr->pushIncoming(smallPkt, {"127.0.0.1", 4242});

    client.poll();

    // No ack or send should have occurred
    EXPECT_EQ(mockPtr->lastSent_.size(), 0u);
}

TEST(NetworkClientTest, TruncatedReliableSpawnSendsAckButNoCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool spawnCalled = false;
    client.onEntitySpawn([&](EntitySpawnEvent) { spawnCalled = true; });

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Build a spawn header for a reliable packet but provide truncated payload
    EntitySpawnPayload goodSpawn{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(321)), static_cast<std::uint8_t>(EntityType::Player), 1.0f, 2.0f};
    Buffer fullPayload(sizeof(EntitySpawnPayload)); std::memcpy(fullPayload.data(), &goodSpawn, sizeof(goodSpawn));

    // Truncate payload by removing last bytes
    Buffer truncated(fullPayload.begin(), fullPayload.begin() + (fullPayload.size() - 2));

    auto pkt_spawn = buildPacket(OpCode::S_ENTITY_SPAWN, truncated, 0);
    Header hspawn; std::memcpy(&hspawn, pkt_spawn.data(), kHeaderSize);
    hspawn.flags |= Flags::kReliable;
    hspawn.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(99));
    std::memcpy(pkt_spawn.data(), &hspawn, kHeaderSize);

    // Clear lastSent and push
    mockPtr->lastSent_.clear();
    mockPtr->pushIncoming(pkt_spawn, {"127.0.0.1", 4242});

    client.poll();

    // ACK should have been sent
    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header ackHdr; std::memcpy(&ackHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(ackHdr.opcode), OpCode::ACK);

    // But spawn callback should NOT be invoked due to truncated payload
    EXPECT_FALSE(spawnCalled);
}

TEST(NetworkClientTest, SendFunctionsReturnFalseWhenNotConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();

    NetworkClient client(cfg, std::move(mock), false);

    // Not connected: send calls should return false
    EXPECT_FALSE(client.sendInput(InputMask::kUp));
    EXPECT_FALSE(client.ping());
    EXPECT_FALSE(client.sendReady(true));
}

TEST(NetworkClientTest, UnknownOpcodeIgnoredWithAckFlag) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept to establish state
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Clear sends
    mockPtr->lastSent_.clear();

    // Construct a header with an unknown opcode and ACK flag set
    network::Header hdr{};
    hdr.magic = network::kMagicByte;
    hdr.opcode = 0xFF; // invalid/unknown opcode
    hdr.payloadSize = network::ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
    hdr.userId = network::ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(0));
    hdr.seqId = network::ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
    hdr.ackId = network::ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(5));
    hdr.flags = network::Flags::kIsAck;

    Buffer pkt(network::kHeaderSize);
    std::memcpy(pkt.data(), &hdr, network::kHeaderSize);

    mockPtr->pushIncoming(pkt, {"127.0.0.1", 4242});

    // Should not crash and no new send should occur
    client.poll();
    EXPECT_EQ(mockPtr->lastSent_.size(), 0u);
}

TEST(NetworkClientTest, DisconnectOpcodeResetsConnection) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    EXPECT_TRUE(client.isConnected());

    // Send DISCONNECT from server
    mockPtr->pushIncoming(buildPacket(OpCode::DISCONNECT, {}, 0), {"127.0.0.1", 4242});
    client.poll();

    EXPECT_FALSE(client.isConnected());
    EXPECT_FALSE(client.userId().has_value());
}

TEST(NetworkClientTest, PingSendsPacket) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Clear sends
    mockPtr->lastSent_.clear();

    EXPECT_TRUE(client.ping());
    client.poll();

    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header sentHdr; std::memcpy(&sentHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(sentHdr.opcode), OpCode::PING);
}

TEST(NetworkClientTest, ReliablePacketBeforeAcceptBuildAckPacketNullopt) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect but do not send/receive S_ACCEPT
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));

    // Build a reliable spawn packet
    EntitySpawnPayload spawn{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(321)), static_cast<std::uint8_t>(EntityType::Player), 1.0f, 2.0f};
    Buffer sp(sizeof(EntitySpawnPayload)); std::memcpy(sp.data(), &spawn, sizeof(spawn));

    auto pkt_spawn = buildPacket(OpCode::S_ENTITY_SPAWN, sp, 0);
    Header hspawn; std::memcpy(&hspawn, pkt_spawn.data(), kHeaderSize);
    hspawn.flags |= Flags::kReliable;
    hspawn.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(77));
    std::memcpy(pkt_spawn.data(), &hspawn, kHeaderSize);

    // Clear lastSent and push
    mockPtr->lastSent_.clear();
    mockPtr->pushIncoming(pkt_spawn, {"127.0.0.1", 4242});

    client.poll();

    // No ACK should have been sent because buildAckPacket returned nullopt (no user id yet)
    EXPECT_EQ(mockPtr->lastSent_.size(), 0u);
}

TEST(NetworkClientTest, SendReadySendsPacketWhenConnected) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Clear sends
    mockPtr->lastSent_.clear();

    EXPECT_TRUE(client.sendReady(true));
    client.poll();

    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header sentHdr; std::memcpy(&sentHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(sentHdr.opcode), OpCode::C_READY);
}

TEST(NetworkClientTest, AddRemoveConnectedCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool called = false;
    auto id = client.addConnectedCallback([&](std::uint32_t) { called = true; });
    client.removeConnectedCallback(id);

    // Connect and accept - removed callback should not be invoked
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    EXPECT_FALSE(called);
}

TEST(NetworkClientTest, AddRemoveDisconnectedCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool called = false;
    auto id = client.addDisconnectedCallback([&](DisconnectReason) { called = true; });
    client.removeDisconnectedCallback(id);

    // Connect, accept and then send DISCONNECT - removed callback should not be invoked
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload)); std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    mockPtr->pushIncoming(buildPacket(OpCode::DISCONNECT, {}, 0), {"127.0.0.1", 4242});
    client.poll();

    EXPECT_FALSE(called);
}

// Additional truncated reliable packet tests to increase branch coverage
TEST(NetworkClientTest, TruncatedReliableHealthSendsAckButNoCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool healthCalled = false;
    client.onEntityHealth([&](EntityHealthEvent) { healthCalled = true; });

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Build a health payload but truncate it
    EntityHealthPayload health{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(123)), ByteOrderSpec::toNetwork(static_cast<std::int32_t>(10)), ByteOrderSpec::toNetwork(static_cast<std::int32_t>(20))};
    Buffer full(sizeof(EntityHealthPayload)); std::memcpy(full.data(), &health, sizeof(health));
    Buffer truncated(full.begin(), full.begin() + (full.size() - 2));

    auto pkt = buildPacket(OpCode::S_ENTITY_HEALTH, truncated, 0);
    Header hh; std::memcpy(&hh, pkt.data(), kHeaderSize);
    hh.flags |= Flags::kReliable;
    hh.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1234));
    std::memcpy(pkt.data(), &hh, kHeaderSize);

    mockPtr->lastSent_.clear();
    mockPtr->pushIncoming(pkt, {"127.0.0.1", 4242});

    client.poll();

    // ACK should have been sent but callback not invoked
    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header ackHdr; std::memcpy(&ackHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(ackHdr.opcode), OpCode::ACK);
    EXPECT_FALSE(healthCalled);
}

TEST(NetworkClientTest, TruncatedReliableDestroySendsAckButNoCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    std::vector<std::uint32_t> destroyed;
    client.onEntityDestroy([&](std::uint32_t id) { destroyed.push_back(id); });

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Build destroy payload and truncate it
    EntityDestroyPayload d{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(321))};
    Buffer full(sizeof(EntityDestroyPayload)); std::memcpy(full.data(), &d, sizeof(d));
    Buffer truncated(full.begin(), full.begin() + (full.size() - 1));

    auto pkt = buildPacket(OpCode::S_ENTITY_DESTROY, truncated, 0);
    Header hd; std::memcpy(&hd, pkt.data(), kHeaderSize);
    hd.flags |= Flags::kReliable;
    hd.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(4321));
    std::memcpy(pkt.data(), &hd, kHeaderSize);

    mockPtr->lastSent_.clear();
    mockPtr->pushIncoming(pkt, {"127.0.0.1", 4242});

    client.poll();

    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header ackHdr; std::memcpy(&ackHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(ackHdr.opcode), OpCode::ACK);
    EXPECT_TRUE(destroyed.empty());
}

TEST(NetworkClientTest, TruncatedReliableUpdateStateSendsAckButNoCallback) {
    NetworkClient::Config cfg;
    auto mock = std::make_unique<MockSocket>();
    MockSocket* mockPtr = mock.get();

    NetworkClient client(cfg, std::move(mock), false);

    bool stateCalled = false; GameStateEvent gotState{};
    client.onGameStateChange([&](GameStateEvent e){ stateCalled = true; gotState = e; });

    // Connect and accept
    EXPECT_TRUE(client.connect("127.0.0.1", 4242));
    AcceptPayload ap{ByteOrderSpec::toNetwork(static_cast<std::uint32_t>(7))};
    Buffer payload(sizeof(AcceptPayload));
    std::memcpy(payload.data(), &ap, sizeof(AcceptPayload));
    mockPtr->pushIncoming(buildPacket(OpCode::S_ACCEPT, payload, 0), {"127.0.0.1", 4242});
    client.poll();

    // Build update state payload and truncate it
    UpdateStatePayload us{static_cast<std::uint8_t>(GameState::Running)};
    Buffer full(sizeof(UpdateStatePayload)); std::memcpy(full.data(), &us, sizeof(us));
    Buffer truncated(full.begin(), full.begin() + (full.size() - 1));

    auto pkt = buildPacket(OpCode::S_UPDATE_STATE, truncated, 0);
    Header hu; std::memcpy(&hu, pkt.data(), kHeaderSize);
    hu.flags |= Flags::kReliable;
    hu.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(2222));
    std::memcpy(pkt.data(), &hu, kHeaderSize);

    mockPtr->lastSent_.clear();
    mockPtr->pushIncoming(pkt, {"127.0.0.1", 4242});

    client.poll();

    ASSERT_GT(mockPtr->lastSent_.size(), 0u);
    Header ackHdr; std::memcpy(&ackHdr, mockPtr->lastSent_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(ackHdr.opcode), OpCode::ACK);
    EXPECT_FALSE(stateCalled);
}
