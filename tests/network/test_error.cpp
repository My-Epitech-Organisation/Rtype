/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit tests for Error.hpp - NetworkError and Result monad
*/

#include <gtest/gtest.h>

#include <string>

#include "core/Error.hpp"

using namespace rtype::network;

// ============================================================================
// NetworkError toString Tests
// ============================================================================

class NetworkErrorTest : public ::testing::Test {};

TEST_F(NetworkErrorTest, ToString_None) {
    EXPECT_EQ(toString(NetworkError::None), "Success");
}

TEST_F(NetworkErrorTest, ToString_ConnectionErrors) {
    EXPECT_EQ(toString(NetworkError::NotConnected), "Not connected");
    EXPECT_EQ(toString(NetworkError::ConnectionRefused), "Connection refused");
    EXPECT_EQ(toString(NetworkError::Timeout), "Operation timed out");
    EXPECT_EQ(toString(NetworkError::HostNotFound), "Host not found");
    EXPECT_EQ(toString(NetworkError::NetworkUnreachable), "Network unreachable");
    EXPECT_EQ(toString(NetworkError::AddressInUse), "Address already in use");
}

TEST_F(NetworkErrorTest, ToString_ProtocolErrors) {
    EXPECT_EQ(toString(NetworkError::InvalidMagic), "Invalid magic byte");
    EXPECT_EQ(toString(NetworkError::UnknownOpcode), "Unknown opcode");
    EXPECT_EQ(toString(NetworkError::PacketTooLarge), "Packet too large");
    EXPECT_EQ(toString(NetworkError::PacketTooSmall), "Packet too small");
    EXPECT_EQ(toString(NetworkError::MalformedPacket), "Malformed packet");
    EXPECT_EQ(toString(NetworkError::InvalidSequence), "Invalid sequence ID");
    EXPECT_EQ(toString(NetworkError::InvalidUserId), "Invalid user ID");
}

TEST_F(NetworkErrorTest, ToString_OperationErrors) {
    EXPECT_EQ(toString(NetworkError::Cancelled), "Operation cancelled");
    EXPECT_EQ(toString(NetworkError::WouldBlock), "Would block");
    EXPECT_EQ(toString(NetworkError::BufferFull), "Buffer full");
    EXPECT_EQ(toString(NetworkError::InternalError), "Internal error");
}

TEST_F(NetworkErrorTest, ToString_ReliabilityErrors) {
    EXPECT_EQ(toString(NetworkError::MaxRetriesExceeded), "Max retries exceeded");
    EXPECT_EQ(toString(NetworkError::AckTimeout), "ACK timeout");
}

TEST_F(NetworkErrorTest, ToString_UnknownError) {
    // Cast an invalid value to trigger default case
    auto unknownError = static_cast<NetworkError>(255);
    EXPECT_EQ(toString(unknownError), "Unknown error");
}

// ============================================================================
// Result<T, E> Tests
// ============================================================================

class ResultTest : public ::testing::Test {};

TEST_F(ResultTest, Ok_CreatesSuccessResult) {
    auto result = Result<int>::ok(42);
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(ResultTest, Err_CreatesErrorResult) {
    auto result = Result<int>::err(NetworkError::Timeout);
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::Timeout);
}

TEST_F(ResultTest, ImplicitConstruction_FromValue) {
    Result<int> result = 42;
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(ResultTest, ImplicitConstruction_FromError) {
    Result<int> result = NetworkError::BufferFull;
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::BufferFull);
}

TEST_F(ResultTest, BoolConversion_TrueForOk) {
    auto result = Result<int>::ok(10);
    EXPECT_TRUE(static_cast<bool>(result));
    if (result) {
        SUCCEED();
    } else {
        FAIL() << "Expected result to be truthy";
    }
}

TEST_F(ResultTest, BoolConversion_FalseForErr) {
    auto result = Result<int>::err(NetworkError::Cancelled);
    EXPECT_FALSE(static_cast<bool>(result));
    if (!result) {
        SUCCEED();
    } else {
        FAIL() << "Expected result to be falsy";
    }
}

TEST_F(ResultTest, Value_ReturnsValue) {
    auto result = Result<std::string>::ok("hello");
    EXPECT_EQ(result.value(), "hello");
}

TEST_F(ResultTest, Value_ConstRef) {
    const auto result = Result<std::string>::ok("const");
    EXPECT_EQ(result.value(), "const");
}

TEST_F(ResultTest, Value_RvalueRef) {
    auto result = Result<std::string>::ok("moved");
    std::string moved = std::move(result).value();
    EXPECT_EQ(moved, "moved");
}

TEST_F(ResultTest, ValueOr_ReturnsValueOnOk) {
    auto result = Result<int>::ok(42);
    EXPECT_EQ(result.valueOr(0), 42);
}

TEST_F(ResultTest, ValueOr_ReturnsDefaultOnErr) {
    auto result = Result<int>::err(NetworkError::Timeout);
    EXPECT_EQ(result.valueOr(99), 99);
}

TEST_F(ResultTest, ValueOr_Rvalue) {
    auto result = Result<std::string>::ok("value");
    std::string val = std::move(result).valueOr("default");
    EXPECT_EQ(val, "value");
}

TEST_F(ResultTest, ValueOr_Rvalue_ReturnsDefault) {
    auto result = Result<std::string>::err(NetworkError::Cancelled);
    std::string val = std::move(result).valueOr("default");
    EXPECT_EQ(val, "default");
}

TEST_F(ResultTest, Map_TransformsValue) {
    auto result = Result<int>::ok(10);
    auto mapped = result.map([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.isOk());
    EXPECT_EQ(mapped.value(), 20);
}

TEST_F(ResultTest, Map_PreservesError) {
    auto result = Result<int>::err(NetworkError::BufferFull);
    auto mapped = result.map([](int x) { return x * 2; });
    EXPECT_TRUE(mapped.isErr());
    EXPECT_EQ(mapped.error(), NetworkError::BufferFull);
}

TEST_F(ResultTest, Map_ChangesType) {
    auto result = Result<int>::ok(42);
    auto mapped = result.map([](int x) { return std::to_string(x); });
    EXPECT_TRUE(mapped.isOk());
    EXPECT_EQ(mapped.value(), "42");
}

TEST_F(ResultTest, MapErr_TransformsError) {
    auto result = Result<int>::err(NetworkError::Timeout);
    auto mapped = result.mapErr([](NetworkError) { return NetworkError::InternalError; });
    EXPECT_TRUE(mapped.isErr());
    EXPECT_EQ(mapped.error(), NetworkError::InternalError);
}

TEST_F(ResultTest, MapErr_PreservesValue) {
    auto result = Result<int>::ok(100);
    auto mapped = result.mapErr([](NetworkError) { return NetworkError::InternalError; });
    EXPECT_TRUE(mapped.isOk());
    EXPECT_EQ(mapped.value(), 100);
}

TEST_F(ResultTest, AndThen_ChainsOnOk) {
    auto result = Result<int>::ok(5);
    auto chained = result.andThen([](int x) -> Result<int> {
        return Result<int>::ok(x + 10);
    });
    EXPECT_TRUE(chained.isOk());
    EXPECT_EQ(chained.value(), 15);
}

TEST_F(ResultTest, AndThen_PropagatesError) {
    auto result = Result<int>::err(NetworkError::WouldBlock);
    auto chained = result.andThen([](int x) -> Result<int> {
        return Result<int>::ok(x + 10);
    });
    EXPECT_TRUE(chained.isErr());
    EXPECT_EQ(chained.error(), NetworkError::WouldBlock);
}

TEST_F(ResultTest, AndThen_CanReturnError) {
    auto result = Result<int>::ok(5);
    auto chained = result.andThen([](int) -> Result<int> {
        return Result<int>::err(NetworkError::InternalError);
    });
    EXPECT_TRUE(chained.isErr());
    EXPECT_EQ(chained.error(), NetworkError::InternalError);
}

TEST_F(ResultTest, AndThen_ChangesType) {
    auto result = Result<int>::ok(42);
    auto chained = result.andThen([](int x) -> Result<std::string> {
        return Result<std::string>::ok("Number: " + std::to_string(x));
    });
    EXPECT_TRUE(chained.isOk());
    EXPECT_EQ(chained.value(), "Number: 42");
}

// ============================================================================
// Result<void, E> Tests
// ============================================================================

class ResultVoidTest : public ::testing::Test {};

TEST_F(ResultVoidTest, Ok_CreatesSuccessResult) {
    auto result = Result<void>::ok();
    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.isErr());
}

TEST_F(ResultVoidTest, Err_CreatesErrorResult) {
    auto result = Result<void>::err(NetworkError::NotConnected);
    EXPECT_FALSE(result.isOk());
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::NotConnected);
}

TEST_F(ResultVoidTest, DefaultConstruction_IsOk) {
    Result<void> result;
    EXPECT_TRUE(result.isOk());
}

TEST_F(ResultVoidTest, ImplicitConstruction_FromError) {
    Result<void> result = NetworkError::AddressInUse;
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::AddressInUse);
}

TEST_F(ResultVoidTest, BoolConversion_TrueForOk) {
    auto result = Result<void>::ok();
    EXPECT_TRUE(static_cast<bool>(result));
}

TEST_F(ResultVoidTest, BoolConversion_FalseForErr) {
    auto result = Result<void>::err(NetworkError::Cancelled);
    EXPECT_FALSE(static_cast<bool>(result));
}

// ============================================================================
// Helper Function Tests
// ============================================================================

class HelperFunctionTest : public ::testing::Test {};

TEST_F(HelperFunctionTest, Ok_CreatesSuccessResult) {
    auto result = Ok(42);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(HelperFunctionTest, Ok_WithString) {
    auto result = Ok(std::string("test"));
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), "test");
}

TEST_F(HelperFunctionTest, Ok_Void) {
    auto result = Ok();
    EXPECT_TRUE(result.isOk());
}

TEST_F(HelperFunctionTest, Err_CreatesErrorResult) {
    auto result = Err<int>(NetworkError::Timeout);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::Timeout);
}

TEST_F(HelperFunctionTest, Err_Void) {
    auto result = Err(NetworkError::BufferFull);
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::BufferFull);
}

// ============================================================================
// Edge Cases and Complex Scenarios
// ============================================================================

class ResultEdgeCaseTest : public ::testing::Test {};

TEST_F(ResultEdgeCaseTest, ChainedMaps) {
    auto result = Ok(2);
    auto final_result = result
        .map([](int x) { return x * 3; })
        .map([](int x) { return x + 1; })
        .map([](int x) { return std::to_string(x); });
    
    EXPECT_TRUE(final_result.isOk());
    EXPECT_EQ(final_result.value(), "7");  // (2 * 3) + 1 = 7
}

TEST_F(ResultEdgeCaseTest, ChainedAndThen) {
    auto divide = [](int a, int b) -> Result<int> {
        if (b == 0) {
            return Err<int>(NetworkError::InternalError);
        }
        return Ok(a / b);
    };

    auto result = divide(10, 2).andThen([&divide](int x) {
        return divide(x, 1);
    });
    
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 5);
}

TEST_F(ResultEdgeCaseTest, ChainedAndThen_ErrorInMiddle) {
    auto divide = [](int a, int b) -> Result<int> {
        if (b == 0) {
            return Err<int>(NetworkError::InternalError);
        }
        return Ok(a / b);
    };

    auto result = divide(10, 0).andThen([&divide](int x) {
        return divide(x, 1);  // Should not be called
    });
    
    EXPECT_TRUE(result.isErr());
    EXPECT_EQ(result.error(), NetworkError::InternalError);
}

TEST_F(ResultEdgeCaseTest, MoveOnlyType) {
    auto result = Ok(std::make_unique<int>(42));
    EXPECT_TRUE(result.isOk());
    
    auto ptr = std::move(result).value();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

TEST_F(ResultEdgeCaseTest, ValueOr_WithMoveOnly) {
    auto result = Result<std::unique_ptr<int>>::err(NetworkError::Timeout);
    auto ptr = std::move(result).valueOr(std::make_unique<int>(99));
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 99);
}
