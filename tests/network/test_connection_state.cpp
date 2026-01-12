/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Connection State Machine Unit Tests
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <thread>

#include "connection/Connection.hpp"
#include "connection/ConnectionEvents.hpp"
#include "connection/ConnectionState.hpp"
#include "connection/ConnectionStateMachine.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::network;
using namespace std::chrono_literals;

class ConnectionStateTest : public ::testing::Test {};

TEST_F(ConnectionStateTest, StateToString) {
    EXPECT_EQ(toString(ConnectionState::Disconnected), "Disconnected");
    EXPECT_EQ(toString(ConnectionState::Connecting), "Connecting");
    EXPECT_EQ(toString(ConnectionState::Connected), "Connected");
    EXPECT_EQ(toString(ConnectionState::Disconnecting), "Disconnecting");
}

TEST_F(ConnectionStateTest, StateToStringUnknownValue) {
    // Test unknown value to cover the default case in toString
    auto unknownState = static_cast<ConnectionState>(255);
    EXPECT_EQ(toString(unknownState), "Unknown");
}

TEST_F(ConnectionStateTest, CanInitiateConnectOnlyFromDisconnected) {
    EXPECT_TRUE(canInitiateConnect(ConnectionState::Disconnected));
    EXPECT_FALSE(canInitiateConnect(ConnectionState::Connecting));
    EXPECT_FALSE(canInitiateConnect(ConnectionState::Connected));
    EXPECT_FALSE(canInitiateConnect(ConnectionState::Disconnecting));
}

TEST_F(ConnectionStateTest, CanReceiveAcceptOnlyFromConnecting) {
    EXPECT_FALSE(canReceiveAccept(ConnectionState::Disconnected));
    EXPECT_TRUE(canReceiveAccept(ConnectionState::Connecting));
    EXPECT_FALSE(canReceiveAccept(ConnectionState::Connected));
    EXPECT_FALSE(canReceiveAccept(ConnectionState::Disconnecting));
}

TEST_F(ConnectionStateTest, CanInitiateDisconnectFromConnectedOrConnecting) {
    EXPECT_FALSE(canInitiateDisconnect(ConnectionState::Disconnected));
    EXPECT_TRUE(canInitiateDisconnect(ConnectionState::Connecting));
    EXPECT_TRUE(canInitiateDisconnect(ConnectionState::Connected));
    EXPECT_FALSE(canInitiateDisconnect(ConnectionState::Disconnecting));
}

TEST_F(ConnectionStateTest, CanFinalizeDisconnectOnlyFromDisconnecting) {
    EXPECT_FALSE(canFinalizeDisconnect(ConnectionState::Disconnected));
    EXPECT_FALSE(canFinalizeDisconnect(ConnectionState::Connecting));
    EXPECT_FALSE(canFinalizeDisconnect(ConnectionState::Connected));
    EXPECT_TRUE(canFinalizeDisconnect(ConnectionState::Disconnecting));
}

TEST_F(ConnectionStateTest, CanSendDataOnlyWhenConnected) {
    EXPECT_FALSE(canSendData(ConnectionState::Disconnected));
    EXPECT_FALSE(canSendData(ConnectionState::Connecting));
    EXPECT_TRUE(canSendData(ConnectionState::Connected));
    EXPECT_FALSE(canSendData(ConnectionState::Disconnecting));
}

TEST_F(ConnectionStateTest, IsTerminalStateOnlyDisconnected) {
    EXPECT_TRUE(isTerminalState(ConnectionState::Disconnected));
    EXPECT_FALSE(isTerminalState(ConnectionState::Connecting));
    EXPECT_FALSE(isTerminalState(ConnectionState::Connected));
    EXPECT_FALSE(isTerminalState(ConnectionState::Disconnecting));
}

class ConnectionStateMachineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        config_.connectTimeout = 50ms;
        config_.disconnectTimeout = 50ms;
        config_.heartbeatTimeout = 100ms;
        config_.maxConnectRetries = 3;
    }

    ConnectionStateMachine::Config config_;
};

TEST_F(ConnectionStateMachineTest, InitialStateIsDisconnected) {
    ConnectionStateMachine fsm(config_);
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_TRUE(fsm.isDisconnected());
    EXPECT_FALSE(fsm.isConnected());
    EXPECT_FALSE(fsm.userId().has_value());
}

TEST_F(ConnectionStateMachineTest, InitiateConnectTransitionsToConnecting) {
    ConnectionStateMachine fsm(config_);

    auto result = fsm.initiateConnect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Connecting);
    EXPECT_EQ(fsm.retryCount(), 0);
}

TEST_F(ConnectionStateMachineTest, CannotConnectWhileConnecting) {
    ConnectionStateMachine fsm(config_);
    auto r1 = fsm.initiateConnect();
    ASSERT_TRUE(r1.isOk());

    auto result = fsm.initiateConnect();
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidStateTransition);
}

TEST_F(ConnectionStateMachineTest, HandleAcceptTransitionsToConnected) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());

    const std::uint32_t userId = 12345;
    auto result = fsm.handleAccept(userId);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Connected);
    EXPECT_TRUE(fsm.isConnected());
    EXPECT_EQ(fsm.userId(), userId);
}

TEST_F(ConnectionStateMachineTest, CannotAcceptWhenDisconnected) {
    ConnectionStateMachine fsm(config_);

    auto result = fsm.handleAccept(123);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidStateTransition);
}

TEST_F(ConnectionStateMachineTest, InitiateDisconnectFromConnected) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());
    fsm.handleAccept(123);

    auto result = fsm.initiateDisconnect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnecting);
}

TEST_F(ConnectionStateMachineTest, InitiateDisconnectFromConnecting) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());

    auto result = fsm.initiateDisconnect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnecting);
}

TEST_F(ConnectionStateMachineTest, CannotDisconnectWhenAlreadyDisconnected) {
    ConnectionStateMachine fsm(config_);

    auto result = fsm.initiateDisconnect();
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidStateTransition);
}

TEST_F(ConnectionStateMachineTest, HandleDisconnectAckCompletesDisconnect) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());
    fsm.handleAccept(123);
    fsm.initiateDisconnect();

    auto result = fsm.handleDisconnectAck();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_EQ(fsm.lastDisconnectReason(), DisconnectReason::LocalRequest);
}

TEST_F(ConnectionStateMachineTest, HandleRemoteDisconnect) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());
    fsm.handleAccept(123);

    auto result = fsm.handleRemoteDisconnect(DisconnectReason::RemoteRequest);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_EQ(fsm.lastDisconnectReason(), DisconnectReason::RemoteRequest);
}

TEST_F(ConnectionStateMachineTest, ForceDisconnect) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());
    fsm.handleAccept(123);

    fsm.forceDisconnect(DisconnectReason::ProtocolError);
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_EQ(fsm.lastDisconnectReason(), DisconnectReason::ProtocolError);
}

TEST_F(ConnectionStateMachineTest, TimeoutInConnectingTriggersRetry) {
    ConnectionStateMachine fsm(config_);
    auto r = fsm.initiateConnect();
    ASSERT_TRUE(r.isOk());

    std::this_thread::sleep_for(config_.connectTimeout + 10ms);

    auto result = fsm.update();
    EXPECT_EQ(result, ConnectionStateMachine::UpdateResult::ShouldRetryConnect);
    EXPECT_EQ(fsm.retryCount(), 1);
    EXPECT_EQ(fsm.state(), ConnectionState::Connecting);
}

TEST_F(ConnectionStateMachineTest, MaxRetriesExceededDisconnects) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();

    for (int i = 0; i <= config_.maxConnectRetries; ++i) {
        std::this_thread::sleep_for(config_.connectTimeout + 10ms);
        auto result = fsm.update();

        if (i < config_.maxConnectRetries) {
            EXPECT_EQ(result,
                      ConnectionStateMachine::UpdateResult::ShouldRetryConnect);
        } else {
            EXPECT_EQ(result,
                      ConnectionStateMachine::UpdateResult::ConnectionTimedOut);
            EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
            EXPECT_EQ(fsm.lastDisconnectReason(),
                      DisconnectReason::MaxRetriesExceeded);
        }
    }
}

TEST_F(ConnectionStateMachineTest, HeartbeatTimeoutDisconnects) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();
    fsm.handleAccept(123);

    std::this_thread::sleep_for(config_.heartbeatTimeout + 10ms);

    auto result = fsm.update();
    EXPECT_EQ(result, ConnectionStateMachine::UpdateResult::ConnectionTimedOut);
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_EQ(fsm.lastDisconnectReason(), DisconnectReason::Timeout);
}

TEST_F(ConnectionStateMachineTest, RecordActivityResetsHeartbeat) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();
    fsm.handleAccept(123);

    std::this_thread::sleep_for(config_.heartbeatTimeout / 2);
    fsm.recordActivity();
    std::this_thread::sleep_for(config_.heartbeatTimeout / 2);

    auto result = fsm.update();
    EXPECT_EQ(result, ConnectionStateMachine::UpdateResult::NoAction);
    EXPECT_TRUE(fsm.isConnected());
}

TEST_F(ConnectionStateMachineTest, DisconnectTimeoutCompletesDisconnect) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();
    fsm.handleAccept(123);
    fsm.initiateDisconnect();

    std::this_thread::sleep_for(config_.disconnectTimeout + 10ms);

    auto result = fsm.update();
    EXPECT_EQ(result, ConnectionStateMachine::UpdateResult::DisconnectComplete);
    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
}

TEST_F(ConnectionStateMachineTest, ResetClearsState) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();
    fsm.handleAccept(123);

    fsm.reset();

    EXPECT_EQ(fsm.state(), ConnectionState::Disconnected);
    EXPECT_FALSE(fsm.userId().has_value());
    EXPECT_EQ(fsm.retryCount(), 0);
    EXPECT_FALSE(fsm.lastDisconnectReason().has_value());
}

TEST_F(ConnectionStateMachineTest, CallbacksAreInvoked) {
    ConnectionStateMachine fsm(config_);

    bool stateChangeCalled = false;
    bool connectedCalled = false;
    bool disconnectedCalled = false;
    std::uint32_t receivedUserId = 0;
    DisconnectReason receivedReason{};

    ConnectionCallbacks callbacks;
    callbacks.onStateChange = [&](ConnectionState, ConnectionState) {
        stateChangeCalled = true;
    };
    callbacks.onConnected = [&](std::uint32_t id) {
        connectedCalled = true;
        receivedUserId = id;
    };
    callbacks.onDisconnected = [&](DisconnectReason reason) {
        disconnectedCalled = true;
        receivedReason = reason;
    };

    fsm.setCallbacks(callbacks);
    fsm.initiateConnect();
    EXPECT_TRUE(stateChangeCalled);

    fsm.handleAccept(999);
    EXPECT_TRUE(connectedCalled);
    EXPECT_EQ(receivedUserId, 999);

    fsm.forceDisconnect(DisconnectReason::RemoteRequest);
    EXPECT_TRUE(disconnectedCalled);
    EXPECT_EQ(receivedReason, DisconnectReason::RemoteRequest);
}

TEST_F(ConnectionStateMachineTest, TimeInCurrentStateIncreases) {
    ConnectionStateMachine fsm(config_);
    fsm.initiateConnect();

    auto initial = fsm.timeInCurrentState();
    std::this_thread::sleep_for(20ms);
    auto later = fsm.timeInCurrentState();

    EXPECT_GT(later, initial);
}

class DisconnectReasonTest : public ::testing::Test {};

TEST_F(DisconnectReasonTest, ReasonToString) {
    EXPECT_EQ(toString(DisconnectReason::LocalRequest), "LocalRequest");
    EXPECT_EQ(toString(DisconnectReason::RemoteRequest), "RemoteRequest");
    EXPECT_EQ(toString(DisconnectReason::Timeout), "Timeout");
    EXPECT_EQ(toString(DisconnectReason::MaxRetriesExceeded),
              "MaxRetriesExceeded");
    EXPECT_EQ(toString(DisconnectReason::ProtocolError), "ProtocolError");
}

// =============================================================================
// Connection Tests (with sender validation)
// =============================================================================

class ConnectionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        connection_ = std::make_unique<Connection>();
        serverEndpoint_ = Endpoint{"192.168.1.100", 4242};
        wrongEndpoint_ = Endpoint{"10.0.0.1", 9999};
    }

    Buffer buildAcceptPacket(std::uint32_t userId) {
        Header header;
        header.magic = kMagicByte;
        header.opcode = static_cast<std::uint8_t>(OpCode::S_ACCEPT);
        header.payloadSize = ByteOrderSpec::toNetwork(
            static_cast<std::uint16_t>(sizeof(AcceptPayload)));
        header.userId = ByteOrderSpec::toNetwork(userId);
        header.seqId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(1));
        header.ackId = ByteOrderSpec::toNetwork(static_cast<std::uint16_t>(0));
        header.flags = Flags::kReliable | Flags::kIsAck;
        header.reserved = {0, 0, 0};

        AcceptPayload payload;
        payload.newUserId = ByteOrderSpec::toNetwork(userId);

        Buffer packet(kHeaderSize + sizeof(AcceptPayload));
        std::memcpy(packet.data(), &header, kHeaderSize);
        std::memcpy(packet.data() + kHeaderSize, &payload, sizeof(AcceptPayload));
        return packet;
    }

    std::unique_ptr<Connection> connection_;
    Endpoint serverEndpoint_;
    Endpoint wrongEndpoint_;
};

TEST_F(ConnectionTest, InitialStateIsDisconnected) {
    EXPECT_TRUE(connection_->isDisconnected());
    EXPECT_FALSE(connection_->isConnected());
    EXPECT_EQ(connection_->state(), ConnectionState::Disconnected);
}

TEST_F(ConnectionTest, ConnectTransitionsToConnecting) {
    auto result = connection_->connect();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(connection_->state(), ConnectionState::Connecting);
}

TEST_F(ConnectionTest, AcceptCapturesServerEndpoint) {
    (void)connection_->connect();

    Buffer acceptPacket = buildAcceptPacket(42);
    auto result = connection_->processPacket(acceptPacket, serverEndpoint_);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(connection_->isConnected());
    EXPECT_EQ(connection_->userId().value(), 42);
}

TEST_F(ConnectionTest, RejectsPacketsFromWrongSenderAfterAccept) {
    (void)connection_->connect();

    // First accept from server establishes the endpoint
    Buffer acceptPacket = buildAcceptPacket(42);
    (void)connection_->processPacket(acceptPacket, serverEndpoint_);

    // Now try to receive from a different endpoint - should be rejected
    Buffer secondPacket = buildAcceptPacket(99);
    auto result = connection_->processPacket(secondPacket, wrongEndpoint_);

    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InvalidSender);
}

TEST_F(ConnectionTest, AcceptsPacketsFromCorrectServerAfterAccept) {
    (void)connection_->connect();

    Buffer acceptPacket = buildAcceptPacket(42);
    (void)connection_->processPacket(acceptPacket, serverEndpoint_);

    // Packets from same server should still be accepted
    // (even if duplicate - that's a different error)
    Buffer secondPacket = buildAcceptPacket(42);
    auto result = connection_->processPacket(secondPacket, serverEndpoint_);

    // Should not be InvalidSender (might be DuplicatePacket or InvalidStateTransition)
    if (result.isErr()) {
        EXPECT_NE(result.error(), NetworkError::InvalidSender);
    }
}

TEST_F(ConnectionTest, ResetClearsServerEndpoint) {
    (void)connection_->connect();

    Buffer acceptPacket = buildAcceptPacket(42);
    (void)connection_->processPacket(acceptPacket, serverEndpoint_);

    connection_->reset();

    EXPECT_TRUE(connection_->isDisconnected());
    EXPECT_FALSE(connection_->userId().has_value());

    // After reset, should accept from any endpoint again
    (void)connection_->connect();
    Buffer newAccept = buildAcceptPacket(99);
    auto result = connection_->processPacket(newAccept, wrongEndpoint_);

    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(connection_->userId().value(), 99);
}
