#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <optional>

#define private public
#include "client/network/NetworkClient.hpp"
#undef private

#include "transport/IAsyncSocket.hpp"
#include "protocol/Header.hpp"
#include "Serializer.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::client;
using namespace rtype::network;


// Minimal FakeSocket used for deterministic unit tests
class FakeSocket : public IAsyncSocket {
  public:
    FakeSocket() : open_(true), boundPort_(0) {}

    Result<void> bind(std::uint16_t port) override {
        boundPort_ = (port == 0) ? 12345 : port;
        return Ok();
    }

    bool isOpen() const noexcept override { return open_; }

    std::uint16_t localPort() const noexcept override { return boundPort_; }

    void asyncSendTo(const Buffer& data, const Endpoint& dest,
                     SendCallback handler) override {
        lastSend_ = data;
        lastDest_ = dest;
        if (handler) handler(Ok(data.size()));
    }

    void asyncReceiveFrom(std::shared_ptr<Buffer> /*buffer*/,
                          std::shared_ptr<Endpoint> /*sender*/,
                          ReceiveCallback /*handler*/) override {
        // Not needed for tests
    }

    void cancel() override { (void)0; }
    void close() override { open_ = false; }

    // Observability
    Buffer lastSend_;
    Endpoint lastDest_;

  private:
    bool open_;
    std::uint16_t boundPort_;
};

// Helper to build a packet buffer containing header + payload
static Buffer buildPacketBuffer(const Header& header, const Buffer& payload) {
    Buffer buf(kHeaderSize + payload.size());
    std::memcpy(buf.data(), &header, kHeaderSize);
    if (!payload.empty()) {
        std::memcpy(buf.data() + kHeaderSize, payload.data(), payload.size());
    }
    return buf;
}

TEST(NetworkClientHandlersTest, LobbyListEmptyTriggersCallback) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};
    client.onLobbyListReceived([&called](LobbyListEvent ev) {
        called = true;
        EXPECT_TRUE(ev.lobbies.empty());
    });

    // Build header for S_LOBBY_LIST with zero payload
    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_LOBBY_LIST);
    hdr.payloadSize = ByteOrderSpec::toNetwork(0);
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = 0;

    Buffer pkt = buildPacketBuffer(hdr, Buffer{});

    Endpoint sender{"127.0.0.1", 11111};
    client.test_processIncomingPacket(pkt, sender);

    // Dispatch queued callbacks
    client.test_dispatchCallbacks();

    EXPECT_TRUE(called.load());
}

TEST(NetworkClientHandlersTest, EntitySpawnInvokesCallback) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};

    client.onEntitySpawn([&called](EntitySpawnEvent ev) {
        called = true;
        EXPECT_EQ(ev.entityId, 42u);
        EXPECT_EQ(ev.type, EntityType::Bydos);
    });

    EntitySpawnPayload payload{};
    payload.entityId = 42;
    payload.type = static_cast<std::uint8_t>(EntityType::Bydos);
    payload.posX = 1.5f;
    payload.posY = -2.5f;

    // Manually construct payload in network format
    Buffer serialized(sizeof(EntitySpawnPayload));
    std::uint32_t idNet = ByteOrderSpec::toNetwork(42u);
    std::memcpy(serialized.data(), &idNet, sizeof(idNet));
    serialized[4] = static_cast<std::uint8_t>(EntityType::Bydos);
    float x = 1.5f;
    float y = -2.5f;
    std::memcpy(serialized.data() + 5, &x, sizeof(x));
    std::memcpy(serialized.data() + 9, &y, sizeof(y));

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_ENTITY_SPAWN);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(serialized.size()));
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = 0;

    Buffer pkt = buildPacketBuffer(hdr, serialized);
    Endpoint sender{"127.0.0.1", 22222};
    client.test_processIncomingPacket(pkt, sender);

    // Dispatch queued callbacks
    client.test_dispatchCallbacks();

    EXPECT_TRUE(called.load());
}

TEST(NetworkClientHandlersTest, CompressedPayloadDecompressionFailure) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};
    client.onLobbyListReceived([&called](LobbyListEvent) { called = true; });

    // Build header indicating compressed payload but payload is garbage
    Buffer garbage = {0xDE, 0xAD, 0xBE, 0xEF};

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_LOBBY_LIST);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(garbage.size()));
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = static_cast<std::uint8_t>(Flags::kCompressed);

    Buffer pkt = buildPacketBuffer(hdr, garbage);
    Endpoint sender{"127.0.0.1", 33333};
    client.test_processIncomingPacket(pkt, sender);

    // Decompression should fail and callback should not be invoked
    EXPECT_FALSE(called.load());
}

TEST(NetworkClientHandlersTest, LobbyListMultipleEntriesParsed) {
    auto sock = std::make_unique<FakeSocket>();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};
    client.onLobbyListReceived([&called](LobbyListEvent ev) {
        called = true;
        EXPECT_EQ(ev.lobbies.size(), 2u);
        EXPECT_EQ(ev.lobbies[0].code.substr(0,3), "ABC");
        EXPECT_EQ(ev.lobbies[1].port, 4242u);
        EXPECT_EQ(ev.lobbies[1].playerCount, 1u);
        EXPECT_EQ(ev.lobbies[1].maxPlayers, 4u);
        EXPECT_TRUE(ev.lobbies[1].isActive);
    });

    // Build payload: count + two LobbyInfo entries
    Buffer payload;
    payload.push_back(2); // count

    // First lobby: code "ABC   ", port 1234, players 0, max 2, inactive
    const char code1[6] = {'A','B','C',' ',' ',' '};
    payload.insert(payload.end(), code1, code1 + 6);
    std::uint16_t port1 = ByteOrderSpec::toNetwork((std::uint16_t)1234);
    payload.insert(payload.end(), reinterpret_cast<std::uint8_t*>(&port1), reinterpret_cast<std::uint8_t*>(&port1) + sizeof(port1));
    payload.push_back(0);
    payload.push_back(2);
    payload.push_back(0);

    // Second lobby: code "ZZZZZZ", port 4242, players 1, max 4, active
    const char code2[6] = {'Z','Z','Z','Z','Z','Z'};
    payload.insert(payload.end(), code2, code2 + 6);
    std::uint16_t port2 = ByteOrderSpec::toNetwork((std::uint16_t)4242);
    payload.insert(payload.end(), reinterpret_cast<std::uint8_t*>(&port2), reinterpret_cast<std::uint8_t*>(&port2) + sizeof(port2));
    payload.push_back(1);
    payload.push_back(4);
    payload.push_back(1);

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_LOBBY_LIST);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = 0;

    Buffer pkt = buildPacketBuffer(hdr, payload);
    Endpoint sender{"127.0.0.1", 33333};
    client.test_processIncomingPacket(pkt, sender);
    client.test_dispatchCallbacks();

    EXPECT_TRUE(called.load());
}

TEST(NetworkClientHandlersTest, EntityMoveBatchMultipleInvokesBatchCallback) {
    auto sock = std::make_unique<FakeSocket>();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};
    client.onEntityMoveBatch([&called](EntityMoveBatchEvent ev) {
        called = true;
        EXPECT_EQ(ev.entities.size(), 2u);
    });

    // Build batch with 2 entries
    Buffer payload;
    payload.push_back(2);
    EntityMovePayload e1{ByteOrderSpec::toNetwork(1u), 1.0f, 2.0f, 0.1f, 0.2f};
    EntityMovePayload e2{ByteOrderSpec::toNetwork(2u), 3.0f, 4.0f, 0.3f, 0.4f};
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&e1), reinterpret_cast<uint8_t*>(&e1) + sizeof(e1));
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&e2), reinterpret_cast<uint8_t*>(&e2) + sizeof(e2));

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_ENTITY_MOVE_BATCH);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = 0;

    Buffer pkt = buildPacketBuffer(hdr, payload);
    Endpoint sender{"127.0.0.1", 44444};
    client.test_processIncomingPacket(pkt, sender);
    client.test_dispatchCallbacks();

    EXPECT_TRUE(called.load());
}

// New tests for join lobby flow
TEST(NetworkClientHandlersTest, JoinLobbyResponseInvokesCallback) {
    auto sock = std::make_unique<FakeSocket>();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    std::atomic<bool> called{false};
    bool acceptedVal = false;
    uint8_t reasonVal = 0;

    client.onJoinLobbyResponse([&](bool accepted, uint8_t reason) {
        called = true;
        acceptedVal = accepted;
        reasonVal = reason;
    });

    rtype::network::JoinLobbyResponsePayload resp{};
    resp.accepted = 1;
    resp.reason = 7;

    Buffer payload(sizeof(resp));
    std::memcpy(payload.data(), &resp, sizeof(resp));

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_JOIN_LOBBY_RESPONSE);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(payload.size()));
    hdr.userId = 0;
    hdr.seqId = 0;
    hdr.ackId = 0;
    hdr.flags = 0;

    Buffer pkt = buildPacketBuffer(hdr, payload);
    Endpoint sender{"127.0.0.1", 55555};
    client.test_processIncomingPacket(pkt, sender);
    client.test_dispatchCallbacks();

    EXPECT_TRUE(called.load());
    EXPECT_TRUE(acceptedVal);
    EXPECT_EQ(reasonVal, 7u);
}

TEST(NetworkClientHandlersTest, AcceptThenReliablePacketSendsAck) {
    auto sock = std::make_unique<FakeSocket>();
    FakeSocket* raw = sock.get();
    NetworkClient::Config cfg{};
    NetworkClient client(cfg, std::move(sock), false);

    // First, simulate S_ACCEPT to set user id and server endpoint
    AcceptPayload accept{};
    accept.newUserId = ByteOrderSpec::toNetwork(42u);
    Buffer acceptPayload(sizeof(AcceptPayload));
    std::memcpy(acceptPayload.data(), &accept, sizeof(accept));

    Header accHdr{};
    accHdr.magic = kMagicByte;
    accHdr.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
    accHdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(acceptPayload.size()));
    accHdr.userId = 0;
    accHdr.seqId = 0;
    accHdr.ackId = 0;
    accHdr.flags = 0;

    Endpoint sender{"10.0.0.1", 4242};
    // Simulate connect path (no threads) so the state machine is in Connecting
    EXPECT_TRUE(client.connection_.connect());
    // Set server endpoint like a real connect would
    client.serverEndpoint_ = sender;
    {
        auto _pkt = buildPacketBuffer(accHdr, acceptPayload);
        client.test_processIncomingPacket(_pkt, sender);
    }

    // Now send a reliable packet (S_ENTITY_SPAWN) to trigger sendAck
    EntitySpawnPayload payload{};
    payload.entityId = 77;
    payload.type = static_cast<std::uint8_t>(EntityType::Bydos);
    payload.posX = 0.0f;
    payload.posY = 0.0f;

    Buffer serialized(sizeof(EntitySpawnPayload));
    std::uint32_t idNet = ByteOrderSpec::toNetwork(payload.entityId);
    std::memcpy(serialized.data(), &idNet, sizeof(idNet));
    serialized[4] = payload.type;
    float x = payload.posX; std::memcpy(serialized.data()+5, &x, sizeof(x));
    float y = payload.posY; std::memcpy(serialized.data()+9, &y, sizeof(y));

    Header hdr{};
    hdr.magic = kMagicByte;
    hdr.opcode = static_cast<std::uint8_t>(OpCode::S_ENTITY_SPAWN);
    hdr.payloadSize = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(serialized.size()));
    hdr.userId = ByteOrderSpec::toNetwork(kServerUserId);
    hdr.seqId = ByteOrderSpec::toNetwork(123u);
    hdr.ackId = 0;
    hdr.flags = rtype::network::Flags::kReliable;

    {
        auto _pkt2 = buildPacketBuffer(hdr, serialized);
        client.test_processIncomingPacket(_pkt2, sender);
    }

    // The fake socket should have recorded a send (the ACK)
    EXPECT_FALSE(raw->lastSend_.empty());
    Header outHdr{};
    std::memcpy(&outHdr, raw->lastSend_.data(), kHeaderSize);
    EXPECT_EQ(static_cast<OpCode>(outHdr.opcode), OpCode::ACK);
}
