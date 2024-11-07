#include "TestCommon.h"
#include "h264/H264Packetizer.h"
#include "h265/H265Packetizer.h"

class H264PacketizerForTest : public H264Packetizer
{
    public:
        void TestRTPFUA(VideoFrame& frame, BufferReader nal)
        {
            H264Packetizer::EmitNal(frame, nal);
        }
};

class H265PacketizerForTest : public H265Packetizer
{
    public:
        void TestRTPFUA(VideoFrame& frame, BufferReader nal)
        {
            H265Packetizer::EmitNal(frame, nal);
        }
};

TEST(TestFUA, h264Fragmentation)
{
    static constexpr int H264NalHeaderSize = 1;
    static constexpr int FUPayloadHdrSize = 1;
    static constexpr int FUHeaderSize = 1;
    H264PacketizerForTest h264Packetizer;
    {
        // mod == 1
        static constexpr int nalSize = 1198*2+2+H264NalHeaderSize;
        static constexpr int nalPayloadSize = nalSize - H264NalHeaderSize;
        uint8_t nalData[nalSize] = {0};
        nalData[0] = 0x65;

        VideoFrame frame(VideoCodec::H264);
        BufferReader nal(nalData, nalSize);

        h264Packetizer.TestRTPFUA(frame, nal);

        int expectedPos = 4 + H264NalHeaderSize;
        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==1);

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalData[0] & 0b0'11'00000) | 28, 0x05};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, 2), 0);
        
            expectedPos += len;
            mod--;
        }
    }
    {
        // integral number of FU-A packets
        static constexpr int nalSize = (RTPPAYLOADSIZE - FUHeaderSize - FUPayloadHdrSize)*10+H264NalHeaderSize;
        static constexpr int nalPayloadSize = nalSize - H264NalHeaderSize;
        uint8_t nalData[H264NalHeaderSize] = {0};
        nalData[0] = 0x65;

        VideoFrame frame(VideoCodec::H264);
        BufferReader nal(nalData, nalSize);

        h264Packetizer.TestRTPFUA(frame, nal);

        int expectedPos = 4 + H264NalHeaderSize;
        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==0);

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalData[0] & 0b0'11'00000) | 28, 0x05};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, 2), 0);
        
            expectedPos += len;
            mod--;
        }
    }
    {
        static constexpr int nalSize = 1198*10+1197+H264NalHeaderSize;
        static constexpr int nalPayloadSize = nalSize - H264NalHeaderSize;
        uint8_t nalData[nalSize] = {0};
        nalData[0] = 0x65;

        VideoFrame frame(VideoCodec::H264);
        BufferReader nal(nalData, nalSize);

        h264Packetizer.TestRTPFUA(frame, nal);

        int expectedPos = 4 + H264NalHeaderSize;
        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==expectedRTPPacekts-1);

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalData[0] & 0b0'11'00000) | 28, 0x05};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, 2), 0);
        
            expectedPos += len;
            mod--;
        }
    }
}

TEST(TestFUA, h265Fragmentation)
{
    static constexpr int FUPayloadHdrSize = 2;
    static constexpr int FUHeaderSize = 1;
    H265PacketizerForTest h265Packetizer;
    {
        // mod == 1
        static constexpr int nalSize = 1197*2+1+HEVCParams::RTP_NAL_HEADER_SIZE;
        static constexpr int nalPayloadSize = nalSize - HEVCParams::RTP_NAL_HEADER_SIZE;

        uint8_t nalData[nalSize] = {0};
        nalData[0] = 0x26;
        nalData[1] = 0x01;
        uint16_t naluHeader = nalData[0] << 8 | nalData[1];
        BYTE nalUnitType = (naluHeader >> 9) & 0b111111;
        BYTE nalLID =  (naluHeader >> 3) & 0b111111;
        BYTE nalTID = naluHeader & 0b111;

        BufferReader nal(nalData, nalSize);
        VideoFrame frame(VideoCodec::H265);
        h265Packetizer.TestRTPFUA(frame, nal);

        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==1);
        int expectedPos = 4 + HEVCParams::RTP_NAL_HEADER_SIZE;

        const uint16_t nalHeaderFU = ((uint16_t)(HEVC_RTP_NALU_Type::UNSPEC49_FU) << 9)
            | ((uint16_t)(nalLID) << 3)
            | ((uint16_t)(nalTID));

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalHeaderFU & 0xff00) >> 8, nalHeaderFU & 0xff,  0x13};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, FUHeaderSize+FUPayloadHdrSize), 0);
            expectedPos += len;
            mod--;
        }
    }
    {
        // integral number of FU-A packets
        static constexpr int nalSize = (RTPPAYLOADSIZE - FUHeaderSize - FUPayloadHdrSize)*10+HEVCParams::RTP_NAL_HEADER_SIZE;
        static constexpr int nalPayloadSize = nalSize - HEVCParams::RTP_NAL_HEADER_SIZE;

        uint8_t nalData[nalSize] = {0};
        nalData[0] = 0x26;
        nalData[1] = 0x01;
        uint16_t naluHeader = nalData[0] << 8 | nalData[1];
        BYTE nalUnitType = (naluHeader >> 9) & 0b111111;
        BYTE nalLID =  (naluHeader >> 3) & 0b111111;
        BYTE nalTID = naluHeader & 0b111;

        BufferReader nal(nalData, nalSize);
        VideoFrame frame(VideoCodec::H265);
        h265Packetizer.TestRTPFUA(frame, nal);

        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==0);
        int expectedPos = 4 + HEVCParams::RTP_NAL_HEADER_SIZE;

        const uint16_t nalHeaderFU = ((uint16_t)(HEVC_RTP_NALU_Type::UNSPEC49_FU) << 9)
            | ((uint16_t)(nalLID) << 3)
            | ((uint16_t)(nalTID));

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalHeaderFU & 0xff00) >> 8, nalHeaderFU & 0xff,  0x13};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, FUHeaderSize+FUPayloadHdrSize), 0);
            expectedPos += len;
            mod--;
        }
    }
    {
        static constexpr int nalSize = 1197*10+1196+HEVCParams::RTP_NAL_HEADER_SIZE;
        static constexpr int nalPayloadSize = nalSize - HEVCParams::RTP_NAL_HEADER_SIZE;
        uint8_t nalData[nalSize] = {0};
        nalData[0] = 0x26;
        nalData[1] = 0x01;
        uint16_t naluHeader = nalData[0] << 8 | nalData[1];
        BYTE nalUnitType = (naluHeader >> 9) & 0b111111;
        BYTE nalLID =  (naluHeader >> 3) & 0b111111;
        BYTE nalTID = naluHeader & 0b111;

        BufferReader nal(nalData, nalSize);
        VideoFrame frame(VideoCodec::H265);
        h265Packetizer.TestRTPFUA(frame, nal);

        int expectedRTPPacekts = std::ceil((double)nalPayloadSize / (RTPPAYLOADSIZE - FUPayloadHdrSize - FUHeaderSize));
        auto packetLen = nalPayloadSize / expectedRTPPacekts;
        int mod = nalPayloadSize % expectedRTPPacekts;
        ASSERT_TRUE(mod==expectedRTPPacekts-1);
        int expectedPos = 4 + HEVCParams::RTP_NAL_HEADER_SIZE;

        const uint16_t nalHeaderFU = ((uint16_t)(HEVC_RTP_NALU_Type::UNSPEC49_FU) << 9)
            | ((uint16_t)(nalLID) << 3)
            | ((uint16_t)(nalTID));

        auto& rtpInfo = frame.GetRtpPacketizationInfo();
        EXPECT_EQ(expectedRTPPacekts, rtpInfo.size());
        for (int i=0; i < expectedRTPPacekts; i++)
        {
            auto len = packetLen + (mod>0 ? 1:0);
            uint8_t expectedFU[FUHeaderSize+FUPayloadHdrSize] = {(nalHeaderFU & 0xff00) >> 8, nalHeaderFU & 0xff,  0x13};

            if (i==0)
                // set Start bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b10'000000;
            if (i==expectedRTPPacekts-1)
                //  set End bit
                expectedFU[FUHeaderSize+FUPayloadHdrSize-1] |= 0b01'000000;
            EXPECT_EQ(rtpInfo[i].GetPos(), expectedPos);
            EXPECT_EQ(rtpInfo[i].GetSize(), len);
            EXPECT_EQ(rtpInfo[i].GetPrefixLen(), FUHeaderSize+FUPayloadHdrSize);
            EXPECT_EQ(memcmp(rtpInfo[i].GetPrefixData(),  expectedFU, FUHeaderSize+FUPayloadHdrSize), 0);
            expectedPos += len;
            mod--;
        }
    }
}