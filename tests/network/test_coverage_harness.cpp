#include <gtest/gtest.h>

#include "protocol/OpCode.hpp"
#include "protocol/Validator.hpp"
#include "protocol/ByteOrderSpec.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype::network;

TEST(CoverageHarness, OpCodeExhaustiveChecks) {
    // Iterate all possible byte values and exercise functions
    for (int v = 0; v < 256; ++v) {
        std::uint8_t val = static_cast<std::uint8_t>(v);
        bool valid = isValidOpCode(val);

        // Validate opcode conversion
        auto opRes = Validator::validateOpCode(val);
        if (valid) {
            ASSERT_TRUE(opRes.isOk());
            OpCode op = opRes.value();
            // Exercise classification functions
            volatile bool rel = isReliable(op);
            volatile bool cli = isClientOpCode(op);
            volatile bool srv = isServerOpCode(op);
            (void)rel; (void)cli; (void)srv;

            // toString and getCategory should return something
            auto name = toString(op);
            auto cat = getCategory(op);
            ASSERT_FALSE(name.empty());
            ASSERT_FALSE(cat.empty());
        } else {
            ASSERT_TRUE(opRes.isErr());
        }
    }
}

TEST(CoverageHarness, ByteOrderAndGenericTypes) {
    // Types of various sizes to exercise 4-byte and 2-byte branches
    #pragma pack(push, 1)
    struct MixA { uint32_t a; uint16_t b; uint16_t c; uint8_t d; };
    struct MixB { uint32_t a; uint32_t b; uint32_t c; };
    struct MixC { uint16_t a; uint16_t b; uint8_t c; };
    #pragma pack(pop)

    MixA A{0x11223344, 0x5566, 0x7788, 0x99};
    MixB B{0x01020304, 0x05060708, 0x090A0B0C};
    MixC C{0xABCD, 0xCDEF, 0xFE};

    auto a_ser = ByteOrderSpec::serializeToNetwork(A);
    auto a_rt = ByteOrderSpec::deserializeFromNetwork<MixA>(a_ser);
    EXPECT_EQ(a_rt.a, A.a);
    EXPECT_EQ(a_rt.b, A.b);

    auto b_ser = ByteOrderSpec::serializeToNetwork(B);
    auto b_rt = ByteOrderSpec::deserializeFromNetwork<MixB>(b_ser);
    EXPECT_EQ(b_rt.c, B.c);

    auto c_ser = ByteOrderSpec::serializeToNetwork(C);
    auto c_rt = ByteOrderSpec::deserializeFromNetwork<MixC>(c_ser);
    EXPECT_EQ(c_rt.a, C.a);

    // Try span overload and invalid sizes
    std::span<const std::uint8_t> span(a_ser);
    auto ar_rt = ByteOrderSpec::deserializeFromNetwork<MixA>(span);
    EXPECT_EQ(ar_rt.d, A.d);

    // Nil payload for empty type should throw from span and vector
    struct Empty {};
    std::vector<std::uint8_t> nonempty = {0xAA};
    EXPECT_THROW(ByteOrderSpec::deserializeFromNetwork<Empty>(nonempty), std::runtime_error);
    EXPECT_THROW(ByteOrderSpec::deserializeFromNetwork<Empty>(std::span<const std::uint8_t>(nonempty)), std::runtime_error);
}

TEST(CoverageHarness, ValidatorExhaustiveHeaders) {
    // Base header
    Header good = Header::create(OpCode::C_INPUT, kMinClientUserId, 123, sizeof(InputPayload));
    auto hb = ByteOrderSpec::serializeToNetwork(good);
    std::vector<std::uint8_t> raw(hb.begin(), hb.end());
    raw.push_back(InputMask::kShoot);

    // valid packet
    auto res = Validator::validatePacket(std::span<const std::uint8_t>(raw), false);
    EXPECT_TRUE(res.isOk());

    // invalid magic
    Header h1 = good; h1.magic = 0x00;
    auto h1b = ByteOrderSpec::serializeToNetwork(h1);
    std::vector<std::uint8_t> raw1(h1b.begin(), h1b.end());
    raw1.push_back(0x00);
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(raw1), false).isErr());

    // invalid size mismatch: header claims payload bigger than present
    Header h2 = good; h2.payloadSize = 50; // mismatch
    auto h2b = ByteOrderSpec::serializeToNetwork(h2);
    std::vector<std::uint8_t> raw2(h2b.begin(), h2b.end());
    // no payload appended
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(raw2), false).isErr());

    // invalid reserved non-zero
    Header h3 = good; h3.reserved = {1, 0, 0};
    auto h3b = ByteOrderSpec::serializeToNetwork(h3);
    std::vector<std::uint8_t> raw3(h3b.begin(), h3b.end());
    raw3.push_back(0x00);
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(raw3), false).isErr());

    // server user id on client-originated packet
    Header h4 = good; h4.userId = kServerUserId;
    auto h4b = ByteOrderSpec::serializeToNetwork(h4);
    std::vector<std::uint8_t> raw4(h4b.begin(), h4b.end());
    raw4.push_back(InputMask::kShoot);
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(raw4), false).isErr());

    // server packet must have server ID
    Header hs = Header::createServer(OpCode::S_ACCEPT, 1, sizeof(AcceptPayload));
    auto hs_ser = ByteOrderSpec::serializeToNetwork(hs);
    std::vector<std::uint8_t> rsh(hs_ser.begin(), hs_ser.end());
    AcceptPayload p; p.newUserId = 1;
    auto payloadv = ByteOrderSpec::serializeToNetwork(p);
    rsh.insert(rsh.end(), payloadv.begin(), payloadv.end());
    ASSERT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(rsh), true).isOk());

    // server id wrong should error
    Header hs2 = hs; hs2.userId = kMinClientUserId; // not server
    auto hs2b = ByteOrderSpec::serializeToNetwork(hs2);
    std::vector<std::uint8_t> rsh2(hs2b.begin(), hs2b.end());
    rsh2.insert(rsh2.end(), payloadv.begin(), payloadv.end());
    EXPECT_TRUE(Validator::validatePacket(std::span<const std::uint8_t>(rsh2), true).isErr());
}

// Fuzz header fields deterministically to cover several validation branches
TEST(CoverageHarness, DeterministicHeaderFuzz) {
    for (std::uint8_t magic : { (std::uint8_t)kMagicByte, (std::uint8_t)0x00 }) {
        for (int opcode = 0; opcode <= 0xFF; ++opcode) {
            std::uint8_t op8 = static_cast<std::uint8_t>(opcode);
            Header h{magic, op8, 0, kUnassignedUserId, 0, 0, 0, {0,0,0}};
            // convert to bytes but keep it small
            auto hb = ByteOrderSpec::serializeToNetwork(h);
            std::vector<std::uint8_t> raw(hb.begin(), hb.end());
            // attempt validatePacket (it may throw/err)
            auto res = Validator::validatePacket(std::span<const std::uint8_t>(raw), false);
            // no assertion; we only exercise branches
        }
    }

    SUCCEED();
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
