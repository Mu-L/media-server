#include "TestCommon.h"
#include "bitstream/BitReader.h"
#include "h264/H26xNal.h"

TEST(TestRbspReader, GetWithZeroBits)
{
    uint8_t buffer[] = {0x00, 0x01, 0x02, 0x03};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);
    EXPECT_EQ(r.Get(0), 0);
}

TEST(TestRbspReader, GetWithMoreThan32Bits)
{
    uint8_t buffer[] = {0x00, 0x01, 0x02, 0x03};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);
    EXPECT_THROW(r.Get(33), std::invalid_argument);
}

TEST(TestRbspReader, GetWithCacheLoad)
{
    uint8_t buffer[] = {0xab, 0xcd, 0x12, 0x34};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    // Request 16 bits, which will trigger a cache load and GetCached call
    DWORD res = r.Get(16);
    EXPECT_EQ(res, 0xabcd);
    // Second Get uses remaining cached bits (verified using debugs since Cache methods are private)
    EXPECT_EQ(r.Get(16), 0x1234);
}

TEST(TestRbspReader, GetWithPartialCacheFill)
{
    uint8_t buffer[] = {0x77, 0x88, 0x99};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    // Request 16 bits
    DWORD res = r.Get(16);
    EXPECT_EQ(res, 0x7788);
    // Attempt to Get more bits than available should trigger a range error
    EXPECT_THROW(r.Get(32), std::range_error);
}

TEST(TestRbspReader, GetWithEmulationPreventionBytes1)
{
    uint8_t buffer[] = {0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x12, 0x34};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    DWORD res;

    // Request 32 bits
    res = r.Get(32); // Should skip the first 0x03 sequences
    EXPECT_EQ(res, 0x00000000);

    // Request 16 bits
    res = r.Get(16); // Should skip the next 0x03 sequences
    EXPECT_EQ(res, 0x1234);
}

TEST(TestRbspReader, GetWithEmulationPreventionBytes2)
{
    uint8_t buffer[] = {0x00, 0x00, 0x03, 0x56};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    DWORD res;

    // Request 16 bits
    res = r.Get(16); // Should skip the first 0x03 sequence
    EXPECT_EQ(res, 0x0000);

    // Request 8 bits
    res = r.Get(8); // Should fetch the rest from cache
    EXPECT_EQ(res, 0x56);
}

TEST(TestRbspReader, GetWithEmulationPreventionBytes3)
{
    uint8_t buffer[] = {0x00, 0x00, 0x03, 0x78};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    DWORD res;

    // Request 24 bits
    res = r.Get(24); // Should skip the 0x03 sequence
    EXPECT_EQ(res, 0x000078);
}

TEST(TestRbspReader, GetWithEmulationPreventionBytes4)
{
    // The emulation sequence trespasses the 4 bytes cache boundary
    uint8_t buffer[] = {0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0xaa, 0xbb};
    RbspReader reader(buffer, sizeof(buffer));
	RbspBitReader r(reader);

    DWORD res;

    // Request 32 bits
    res = r.Get(32); // Cache will include first 0x00 of the seq [0x00, 0x00, 0x03]
    EXPECT_EQ(res, 0x01016000);
    
    // Request 16 bytes
    res = r.Get(16);
    EXPECT_EQ(res, 0x00aa); // Should skip 0x03 by tracking zeros across cache reloads

    // Request remaining bytes in cache
    res = r.Get(8);
    EXPECT_EQ(res, 0xbb);
}


