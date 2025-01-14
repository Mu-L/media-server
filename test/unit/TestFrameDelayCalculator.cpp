#include <gtest/gtest.h>

#include "FrameDelayCalculator.h"
#include "video.h"
#include "audio.h"

#include "data/FramesArrivalInfo.h"

namespace
{
	
constexpr uint64_t MediaTypeToSsrc(MediaFrame::Type type)
{
	return type == MediaFrame::Audio ? 1 : 2;
}

}

class TestFrameDelayCalculator : public testing::Test
{
public:
	/**
	 * Test frame delay calculator.
	 * 
	 * @param framesInfo	The frame arrival info
	 * @param references 	The expected latencies of the reference time and timestamps againt the first arrived frame
	 * @param delays	The expected delay to be set for each stream. The first element of the pair is the ssrc of the stream.
	 */
	void TestDelayCalculator(const std::vector<std::tuple<MediaFrame::Type, uint64_t, uint64_t, uint32_t>>& framesInfo, 
			const std::vector<int64_t>& expectedLatencies, 
			const std::vector<std::pair<uint64_t, int64_t>>& delays);
	
protected:
	
	/**
	 * Calculate the latencies again the first reference time and timestamps. The negative value
	 * means the references used has a less latency than the first reference (which is basing on 
	 * first frame).
	 */
	std::vector<int64_t> calcLatency(const std::vector<std::pair<uint64_t, uint64_t>>& references);

	std::unique_ptr<FrameDelayCalculator> calculator;
};

void TestFrameDelayCalculator::TestDelayCalculator(const std::vector<std::tuple<MediaFrame::Type, uint64_t, uint64_t, uint32_t>>& framesInfo,
						const std::vector<int64_t>& expectedLatencies,
						const std::vector<std::pair<uint64_t, int64_t>>& expectedDelays)
{
	std::queue<std::pair<RTPPacket::shared, std::chrono::milliseconds>> audioPackets;
	std::queue<std::pair<RTPPacket::shared, std::chrono::milliseconds>> videoPackets;
	
	std::vector<std::pair<uint64_t, uint64_t>> references;
	std::vector<std::pair<uint64_t, int64_t>> delays;

	// Loop through all frames
	for (auto& f : framesInfo)
	{	
		auto now = std::chrono::milliseconds(std::get<1>(f));
				
		auto mediaType = std::get<0>(f);
		auto ssrc = MediaTypeToSsrc(mediaType);
		
		auto delayMs = calculator->OnFrame(ssrc, now, std::get<2>(f), std::get<3>(f));		
		delays.emplace_back(ssrc, delayMs.count());
		
		if (references.empty())
		{
			references.emplace_back(calculator->refTime.count(), calculator->refTimestamp);
		}
		else
		{
			auto lastRef = references.back();
			if (lastRef.first != calculator->refTime.count() || lastRef.second != calculator->refTimestamp)
			{
				references.emplace_back(calculator->refTime.count(), calculator->refTimestamp);
			}
		}
		
	}

	auto latencies = calcLatency(references);
	
	ASSERT_EQ(expectedLatencies, latencies);
	ASSERT_EQ(expectedDelays, delays);
}


std::vector<int64_t> TestFrameDelayCalculator::calcLatency(const std::vector<std::pair<uint64_t, uint64_t>>& references)
{
	std::vector<int64_t> latencies;
	
	if (references.empty()) return {};
	
	auto& first = references.front();

	for (auto& r : references)
	{
		auto calculatedTimeDiffMs = (r.second - first.second) * 1000 / 90000;
		auto timeDiffMs = r.first - first.first;
		
		latencies.push_back(int64_t(timeDiffMs) - int64_t(calculatedTimeDiffMs));
	}
	
	return latencies;
}

TEST_F(TestFrameDelayCalculator, TestNormal)
{
	calculator = std::make_unique<FrameDelayCalculator>(-200, 0, std::chrono::milliseconds(20));

	std::vector<int64_t> expectedLatencies = {0, 18, 18, 24, 27, 28};
	
	std::vector<std::pair<uint64_t, int64_t>> expectedDelays = {
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 96 }, { 1, 118 }, { 1, 139 },
		{ 1, 7 }, { 1, 29 }, { 1, 50 }, { 1, 71 }, { 1, 93 }, { 1, 101 }, { 1, 122 }, { 1, 144 },
		{ 2, 153 }, { 2, 135 }, { 2, 149 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 106 }, { 1, 128 }, { 1, 139 }, { 2, 159 }, { 2, 165 }, { 2, 168 }, { 2, 151 }, { 2, 163 },
		{ 1, 1 }, { 1, 23 }, { 1, 44 }, { 1, 65 }, { 1, 87 }, { 1, 108 }, { 1, 129 }, { 1, 151 },
		{ 2, 156 }, { 2, 169 }, { 2, 163 }, { 2, 166 }, { 2, 158 }, { 1, 0 }, { 1, 21 }, { 1, 43 },
		{ 1, 64 }, { 1, 85 }, { 1, 107 }, { 1, 118 }, { 1, 138 }, { 2, 150 }, { 2, 164 }, { 2, 156 },
		{ 2, 159 }, { 2, 156 }, { 1, 12 }, { 1, 33 }, { 1, 54 }, { 1, 76 }, { 1, 97 }, { 1, 118 },
		{ 1, 140 }, { 1, 161 }, { 2, 169 }, { 2, 163 }, { 2, 146 }, { 2, 158 }, { 2, 152 }, { 1, 10 },
		{ 1, 32 }, { 1, 53 }, { 1, 74 }, { 1, 96 }, { 1, 117 }, { 1, 138 }, { 1, 160 }, { 2, 164 },
		{ 2, 156 }, { 2, 160 }, { 2, 152 }, { 2, 165 }, { 1, 10 }, { 1, 30 }, { 1, 52 }, { 1, 73 },
		{ 1, 94 }, { 1, 116 }, { 1, 137 }, { 1, 158 }, { 2, 159 }, { 2, 152 }, { 2, 158 }, { 2, 151 },
		{ 2, 164 }, { 1, 12 }, { 1, 33 }, { 1, 54 }, { 1, 76 }, { 1, 97 }, { 1, 118 }, { 1, 140 },
		{ 1, 161 }, { 2, 158 }, { 2, 163 }, { 2, 156 }, { 2, 150 }, { 2, 162 }, { 1, 23 }, { 1, 45 },
		{ 1, 66 }, { 1, 87 }, { 1, 109 }, { 1, 130 }, { 1, 141 }, { 2, 155 }, { 2, 159 }, { 2, 152 },
		{ 2, 166 }, { 2, 159 }, { 1, 2 }, { 1, 23 }, { 1, 44 }, { 1, 66 }, { 1, 87 }, { 1, 108 },
		{ 1, 130 }, { 1, 151 }, { 2, 161 }, { 2, 164 }, { 2, 147 }, { 2, 159 }, { 2, 152 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 106 }, { 1, 128 }, { 1, 149 }, { 2, 155 },
		{ 2, 148 }, { 2, 161 }, { 2, 164 }, { 2, 153 }, { 1, 14 }, { 1, 36 }, { 1, 56 }, { 1, 77 },
		{ 1, 99 }, { 1, 120 }, { 1, 141 }, { 1, 163 }, { 2, 165 }, { 2, 149 }, { 2, 161 }, { 2, 154 },
		{ 2, 157 }, { 1, 11 }, { 1, 32 }, { 1, 54 }, { 1, 75 }, { 1, 96 }, { 1, 118 }, { 1, 139 },
		{ 2, 159 }, { 2, 147 }, { 2, 160 }, { 2, 153 }, { 2, 167 }, { 1, 0 }, { 1, 21 }, { 1, 41 },
		{ 1, 63 }, { 1, 84 }, { 1, 105 }, { 1, 127 }, { 1, 148 }, { 2, 164 }, { 2, 168 }, { 2, 160 },
		{ 2, 163 }, { 2, 166 }, { 1, 11 }, { 1, 33 }, { 1, 54 }, { 1, 75 }, { 1, 97 }, { 1, 118 },
		{ 1, 119 }, { 1, 141 }, { 2, 153 }, { 2, 167 }, { 2, 159 }, { 2, 172 }, { 2, 165 }, { 1, 9 },
		{ 1, 30 }, { 1, 52 }, { 1, 73 }, { 1, 94 }, { 1, 116 }, { 1, 137 }, { 1, 158 }, { 2, 167 },
		{ 2, 160 }, { 2, 154 }, { 2, 167 }, { 2, 160 }, { 1, 12 }, { 1, 33 }, { 1, 53 }, { 1, 75 },
		{ 1, 96 }, { 1, 117 }, { 1, 139 }, { 1, 160 }, { 2, 165 }, { 2, 158 }, { 2, 172 }, { 2, 164 },
		{ 2, 167 }, { 1, 19 }, { 1, 41 }, { 1, 62 }, { 1, 83 }, { 1, 105 }, { 1, 126 }, { 1, 147 },
		{ 1, 169 }, { 2, 161 }, { 2, 154 }, { 2, 167 }, { 2, 165 }, { 2, 167 }, { 1, 23 }, { 1, 44 },
		{ 1, 66 }, { 1, 87 }, { 1, 108 }, { 1, 130 }, { 1, 141 }, { 1, 162 }, { 2, 159 }, { 2, 173 },
		{ 2, 165 }, { 2, 159 }, { 2, 172 }, { 1, 27 }, { 1, 48 }, { 1, 69 }, { 1, 91 }, { 1, 112 },
		{ 1, 133 }, { 1, 135 }, { 1, 156 }, { 2, 149 }, { 2, 173 }, { 2, 167 }, { 2, 170 }, { 2, 159 },
		{ 1, 33 }, { 1, 55 }, { 1, 75 }, { 1, 96 }, { 1, 118 }, { 1, 139 }, { 1, 160 }, { 1, 163 },
		{ 2, 152 }, { 2, 166 }, { 2, 159 }, { 2, 172 }, { 2, 158 }, { 2, 161 }, { 1, 5 }, { 1, 26 },
		{ 1, 48 }, { 1, 69 }, { 1, 90 }, { 1, 103 }, { 1, 124 }, { 1, 145 }, { 2, 164 }, { 2, 168 },
		{ 2, 171 }, { 2, 154 }, { 2, 167 }, { 1, 6 }, { 1, 27 }, { 1, 48 }, { 1, 70 }, { 1, 91 },
		{ 1, 112 }, { 1, 134 }, { 1, 144 }, { 2, 159 }, { 2, 176 }, { 2, 168 }, { 2, 170 }, { 2, 164 },
		{ 1, 7 }, { 1, 29 }, { 1, 50 }, { 1, 71 }, { 1, 93 }, { 1, 114 }, { 1, 135 }, { 1, 157 },
		{ 2, 158 }, { 2, 174 }, { 2, 157 }, { 2, 170 }, { 2, 163 }, { 1, 20 }, { 1, 41 }, { 1, 63 },
		{ 1, 84 }, { 1, 105 }, { 1, 127 }, { 1, 148 }, { 1, 169 }, { 2, 177 }, { 2, 169 }, { 2, 172 },
		{ 2, 165 }, { 2, 157 }, { 1, 18 }, { 1, 39 }, { 1, 60 }, { 1, 82 }, { 1, 102 }, { 1, 123 },
		{ 1, 145 }, { 1, 166 }, { 2, 170 }, { 2, 163 }, { 2, 165 }, { 2, 159 }, { 2, 171 }, { 1, 15 },
		{ 1, 37 }, { 1, 58 }, { 1, 79 }, { 1, 101 }, { 1, 121 }, { 1, 142 }, { 1, 164 }, { 2, 164 },
		{ 2, 160 }, { 2, 163 }, { 2, 156 }, { 2, 169 }, { 1, 26 }, { 1, 47 }, { 1, 69 }, { 1, 90 },
		{ 1, 111 }, { 1, 133 }, { 1, 154 }, { 1, 166 }, { 2, 161 }, { 2, 167 }, { 2, 161 }, { 2, 153 },
		{ 2, 167 }, { 1, 29 }, { 1, 50 }, { 1, 71 }, { 1, 93 }, { 1, 114 }, { 1, 126 }, { 1, 147 },
		{ 1, 168 }, { 2, 160 }, { 2, 174 }, { 2, 156 }, { 2, 169 }, { 2, 162 }, { 2, 164 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 106 }, { 1, 128 }, { 1, 139 }, { 2, 161 },
		{ 2, 157 }, { 2, 170 }, { 2, 164 }, { 2, 157 }, { 1, 3 }, { 1, 25 }, { 1, 46 }, { 1, 67 },
		{ 1, 89 }, { 1, 110 }, { 1, 131 }, { 1, 143 }, { 2, 161 }, { 2, 173 }, { 2, 166 }, { 2, 159 },
		{ 2, 171 }, { 1, 1 }, { 1, 22 }, { 1, 44 }, { 1, 65 }, { 1, 86 }, { 1, 108 }, { 1, 129 },
		{ 1, 140 }, { 2, 154 }, { 2, 171 }, { 2, 164 }, { 2, 169 }, { 2, 173 }, { 1, 16 }, { 1, 37 },
		{ 1, 58 }, { 1, 80 }, { 1, 101 }, { 1, 122 }, { 1, 144 }, { 1, 165 }, { 2, 175 }, { 2, 168 },
		{ 2, 163 }, { 2, 176 }, { 2, 160 }, { 1, 16 }, { 1, 38 }, { 1, 59 }, { 1, 80 }, { 1, 102 },
		{ 1, 123 }, { 1, 144 }, { 1, 166 }, { 2, 172 }, { 2, 165 }, { 2, 168 }, { 2, 170 }, { 2, 153 },
		{ 1, 14 }, { 1, 35 }, { 1, 57 }, { 1, 78 }, { 1, 99 }, { 1, 121 }, { 1, 142 }, { 1, 163 },
		{ 2, 166 }, { 2, 163 }, { 2, 177 }, { 2, 170 }, { 2, 173 }, { 1, 28 }, { 1, 49 }, { 1, 70 },
		{ 1, 92 }, { 1, 113 }, { 1, 134 }, { 1, 156 }, { 1, 167 }, { 2, 166 }, { 2, 159 }, { 2, 171 },
		{ 2, 164 }, { 2, 170 }, { 1, 31 }, { 1, 53 }, { 1, 73 }, { 1, 94 }, { 1, 116 }, { 1, 137 },
		{ 1, 148 }, { 1, 170 }, { 2, 165 }, { 2, 157 }, { 2, 173 }, { 2, 177 }, { 2, 170 }, { 1, 33 },
		{ 1, 54 }, { 1, 76 }, { 1, 97 }, { 1, 118 }, { 1, 140 }, { 1, 161 }, { 1, 162 }, { 2, 153 },
		{ 2, 177 }, { 2, 170 }, { 2, 168 }, { 2, 164 }, { 2, 178 }, { 1, 2 }, { 1, 22 }, { 1, 43 },
		{ 1, 65 }, { 1, 86 }, { 1, 107 }, { 1, 129 }, { 1, 150 }, { 2, 171 }, { 2, 164 }, { 2, 167 },
		{ 2, 160 }, { 2, 173 }, { 1, 9 }, { 1, 31 }, { 1, 52 }, { 1, 73 }, { 1, 95 }, { 1, 107 },
		{ 1, 128 }, { 1, 150 }, { 2, 166 }, { 2, 164 }, { 2, 178 }, { 2, 170 }, { 1, 10 }, { 1, 31 },
		{ 1, 53 }, { 1, 74 }, { 1, 95 }, { 1, 117 }, { 1, 130 }, { 1, 151 }, { 2, 164 }, { 2, 180 },
		{ 2, 163 }, { 2, 175 }, { 1, 54 }, { 1, 75 }, { 1, 96 }, { 1, 118 }, { 1, 129 }, { 1, 149 },
		{ 1, 171 }, { 2, 168 }, { 2, 173 }, { 2, 176 }, { 2, 159 }, { 2, 174 }, { 1, 35 }, { 1, 56 },
		{ 1, 78 }, { 1, 99 }, { 1, 120 }, { 1, 141 }, { 1, 152 }, { 1, 173 }, { 2, 167 }, { 2, 180 },
		{ 2, 162 }, { 2, 175 }, { 2, 168 }, { 1, 32 }, { 1, 53 }, { 1, 74 }, { 1, 96 }, { 1, 117 },
		{ 1, 138 }, { 1, 160 }, { 1, 171 }, { 2, 160 }, { 2, 174 }, { 2, 156 }, { 2, 168 }, { 2, 162 },
		{ 2, 174 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 106 }, { 1, 128 },
		{ 1, 149 }, { 2, 168 }, { 2, 171 }, { 2, 164 }, { 2, 161 }, { 2, 174 }, { 1, 13 }, { 1, 35 },
		{ 1, 55 }, { 1, 76 }, { 1, 98 }, { 1, 119 }, { 1, 140 }, { 1, 158 }, { 2, 173 }, { 2, 176 },
		{ 2, 166 }, { 2, 180 }, { 2, 172 }, { 1, 15 }, { 1, 36 }, { 1, 57 }, { 1, 78 }, { 1, 99 },
		{ 1, 121 }, { 1, 142 }, { 1, 163 }, { 2, 155 }, { 2, 168 }, { 2, 160 }, { 2, 175 }, { 2, 169 },
		{ 1, 15 }, { 1, 36 }, { 1, 57 }, { 1, 79 }, { 1, 100 }, { 1, 121 }, { 1, 143 }, { 1, 164 },
		{ 2, 171 }, { 2, 164 }, { 2, 177 }, { 2, 160 }, { 2, 162 }, { 1, 22 }, { 1, 44 }, { 1, 65 },
		{ 1, 86 }, { 1, 108 }, { 1, 123 }, { 1, 143 }, { 1, 165 }, { 2, 168 }, { 2, 162 }, { 2, 175 },
		{ 2, 168 }, { 2, 172 }, { 1, 26 }, { 1, 47 }, { 1, 69 }, { 1, 90 }, { 1, 111 }, { 1, 133 },
		{ 1, 144 }, { 1, 165 }, { 2, 165 }, { 2, 158 }, { 2, 171 }, { 2, 153 }, { 2, 170 }, { 1, 27 },
		{ 1, 48 }, { 1, 69 }, { 1, 91 }, { 1, 112 }, { 1, 124 }, { 1, 145 }, { 2, 162 }, { 2, 176 },
		{ 2, 170 }, { 2, 172 }, { 1, 5 }, { 1, 26 }, { 1, 48 }, { 1, 69 }, { 1, 90 }, { 1, 112 },
		{ 1, 133 }, { 1, 148 }, { 2, 162 }, { 2, 175 }, { 2, 164 }, { 2, 170 }, { 2, 163 }, { 1, 17 },
		{ 1, 38 }, { 1, 59 }, { 1, 81 }, { 1, 102 }, { 1, 123 }, { 1, 145 }, { 1, 166 }, { 2, 176 },
		{ 2, 168 }, { 2, 161 }, { 2, 168 }, { 2, 160 }, { 1, 18 }, { 1, 40 }, { 1, 61 }, { 1, 82 },
		{ 1, 104 }, { 1, 125 }, { 1, 146 }, { 1, 168 }, { 2, 173 }, { 2, 166 }, { 2, 170 }, { 2, 163 },
		{ 2, 156 }, { 1, 18 }, { 1, 39 }, { 1, 61 }, { 1, 82 }, { 1, 103 }, { 1, 125 }, { 1, 146 },
		{ 1, 167 }, { 2, 169 }, { 2, 163 }, { 2, 177 }, { 2, 169 }, { 2, 172 }, { 1, 28 }, { 1, 49 },
		{ 1, 70 }, { 1, 92 }, { 1, 113 }, { 1, 134 }, { 1, 146 }, { 1, 166 }, { 2, 164 }, { 2, 167 },
		{ 2, 170 }, { 2, 153 }, { 2, 171 }, { 1, 29 }, { 1, 51 }, { 1, 72 }, { 1, 93 }, { 1, 115 },
		{ 1, 126 }, { 1, 147 }, { 1, 169 }, { 2, 163 }, { 2, 175 }, { 2, 169 }, { 2, 171 }, { 2, 163 },
		{ 1, 27 }, { 1, 48 }, { 1, 70 }, { 1, 91 }, { 1, 112 }, { 1, 134 }, { 1, 155 }, { 1, 176 },
		{ 2, 156 }, { 2, 169 }, { 2, 161 }, { 2, 165 }
	};
	
	ASSERT_NO_FATAL_FAILURE(TestDelayCalculator(TestData::FramesArrivalInfo, expectedLatencies, expectedDelays));
}

TEST_F(TestFrameDelayCalculator, testLatencyReduction)
{	
	calculator = std::make_unique<FrameDelayCalculator>(-100, 0, std::chrono::milliseconds(20));
	
	std::vector<int64_t> expectedLatencies = {
		0, -20, -40, -7, -27, -47, -67, -87, -107, 18, -2, -22, -42, -62, -82, -102, 
		-122, -142, 16, -4, -24, -44, -64, -84, -104, -124, 18, -2, -22, -42, -62, -82, 
		-102, -122, 6, -14, -34, -54, -74, -94, -114, -134, 7, -13, -33, -53, -73, -93, 
		-113, -133, 8, -12, -32, -52, -72, -92, -112, -132, 6, -14, -34, -54, -74, -94, 
		-114, -6, -26, -46, -66, -86, -106, -126, 16, -4, -24, -44, -64, -84, -104, -124, 
		18, -2, -22, -42, -62, -82, -102, -122, 4, -16, -36, -56, -76, -96, -116, -136, 
		7, -13, -33, -53, -73, -93, -113, -133, 24, 4, -16, -36, -56, -76, -96, -116, 
		-136, 12, -8, -28, -48, -68, -88, -108, -128, 14, -6, -26, -46, -66, -86, -106, 
		-126, 12, -8, -28, -48, -68, -88, -108, -128, 4, -16, -36, -56, -76, -96, -116, 
		-136, 0, -20, -40, -60, -80, -100, -120, -3, -23, -43, -63, -83, -103, -123, -143, 
		-135, -10, -30, -50, -70, -90, -110, -130, 18, -2, -22, -42, -62, -82, -102, -122, 
		18, -2, -22, -42, -62, -82, -102, -122, 16, -4, -24, -44, -64, -84, -104, -124, 
		3, -17, -37, -57, -77, -97, -117, -137, -134, 6, -14, -34, -54, -74, -94, -114, 
		-134, 8, -12, -32, -52, -72, -92, -112, -132, -3, -23, -43, -63, -83, -103, -123, 
		-5, -25, -45, -65, -85, -105, -125, 27, 7, -13, -33, -53, -73, -93, -113, 23, 
		3, -17, -37, -57, -77, -97, -117, -137, 26, 6, -14, -34, -54, -74, -94, -114, 
		-134, 11, -9, -29, -49, -69, -89, -109, -129, 10, -10, -30, -50, -70, -90, -110, 
		-130, -126, 13, -7, -27, -47, -67, -87, -107, -127, -1, -21, -41, -61, -81, -101, 
		-121, -5, -25, -45, -65, -85, -105, -125, -145, -143, -6, -26, -46, -66, -86, -106, 
		-126, 25, 5, -15, -35, -55, -75, -95, -115, -135, 17, -3, -23, -43, -63, -83, 
		-103, 17, -3, -23, -43, -63, -83, -103, -123, -27, -47, -67, -87, -107, -127, -147, 
		-8, -28, -48, -68, -88, -108, -128, -5, -25, -45, -65, -85, -105, -125, 28, 8, 
		-12, -32, -52, -72, -92, -112, -132, 15, -5, -25, -45, -65, -85, -105, -125, 13, 
		-7, -27, -47, -67, -87, -107, -127, 13, -7, -27, -47, -67, -87, -107, -127, 6, 
		-14, -34, -54, -74, -94, -114, -134, 2, -18, -38, -58, -78, -98, -118, 1, -19, 
		-39, -59, -79, -99, 23, 3, -17, -37, -57, -77, -97, -117, 11, -9, -29, -49, 
		-69, -89, -109, -129, 10, -10, -30, -50, -70, -90, -110, -130, -128, 10, -10, -30, 
		-50, -70, -90, -110, -130, 0, -20, -40, -60, -80, -100, -120, -1, -21, -41, -61, 
		-81, -101, -121, 1, -19, -39, -59, -79, -99, -119
	};

	std::vector<std::pair<uint64_t, int64_t>> expectedDelays = {
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 96 }, { 1, 98 }, { 1, 99 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 93 }, { 1, 95 }, { 1, 96 },
		{ 2, 85 }, { 2, 48 }, { 2, 41 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 86 }, { 1, 88 }, { 1, 79 }, { 2, 79 }, { 2, 65 }, { 2, 48 }, { 2, 11 }, { 2, 3 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 },
		{ 2, 75 }, { 2, 67 }, { 2, 42 }, { 2, 24 }, { 2, 16 }, { 1, 0 }, { 1, 21 }, { 1, 42 },
		{ 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 78 }, { 1, 78 }, { 2, 70 }, { 2, 63 }, { 2, 36 },
		{ 2, 19 }, { 2, 15 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 },
		{ 1, 88 }, { 1, 89 }, { 2, 77 }, { 2, 51 }, { 2, 14 }, { 2, 6 }, { 2, 0 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 73 },
		{ 2, 46 }, { 2, 29 }, { 2, 1 }, { 2, 15 }, { 1, 0 }, { 1, 20 }, { 1, 41 }, { 1, 63 },
		{ 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 68 }, { 2, 42 }, { 2, 27 }, { 2, 1 },
		{ 2, 14 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 1, 89 }, { 2, 66 }, { 2, 51 }, { 2, 24 }, { 2, 18 }, { 2, 30 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 78 }, { 2, 72 }, { 2, 55 }, { 2, 29 },
		{ 2, 22 }, { 2, 15 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 },
		{ 1, 88 }, { 1, 89 }, { 2, 79 }, { 2, 62 }, { 2, 25 }, { 2, 17 }, { 2, 10 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 75 },
		{ 2, 48 }, { 2, 41 }, { 2, 24 }, { 2, 13 }, { 1, 0 }, { 1, 21 }, { 1, 41 }, { 1, 63 },
		{ 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 71 }, { 2, 34 }, { 2, 26 }, { 2, 0 },
		{ 2, 2 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 2, 88 }, { 2, 55 }, { 2, 49 }, { 2, 22 }, { 2, 15 }, { 1, 0 }, { 1, 21 }, { 1, 41 },
		{ 1, 63 }, { 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 84 }, { 2, 68 }, { 2, 40 },
		{ 2, 23 }, { 2, 6 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 },
		{ 1, 68 }, { 1, 69 }, { 2, 61 }, { 2, 55 }, { 2, 27 }, { 2, 21 }, { 2, 13 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 78 },
		{ 2, 51 }, { 2, 24 }, { 2, 18 }, { 2, 11 }, { 1, 0 }, { 1, 21 }, { 1, 41 }, { 1, 63 },
		{ 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 73 }, { 2, 46 }, { 2, 40 }, { 2, 12 },
		{ 2, 15 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 1, 89 }, { 2, 61 }, { 2, 34 }, { 2, 28 }, { 2, 5 }, { 2, 7 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 78 }, { 1, 79 }, { 2, 56 }, { 2, 50 },
		{ 2, 22 }, { 2, 15 }, { 2, 29 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 86 }, { 1, 68 }, { 1, 69 }, { 2, 42 }, { 2, 46 }, { 2, 20 }, { 2, 3 }, { 2, 0 },
		{ 1, 0 }, { 1, 21 }, { 1, 41 }, { 1, 63 }, { 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 69 },
		{ 2, 38 }, { 2, 32 }, { 2, 5 }, { 2, 19 }, { 2, 4 }, { 2, 7 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 97 }, { 1, 99 }, { 1, 100 }, { 2, 99 }, { 2, 82 },
		{ 2, 66 }, { 2, 29 }, { 2, 21 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 86 }, { 1, 88 }, { 1, 78 }, { 2, 73 }, { 2, 70 }, { 2, 42 }, { 2, 24 }, { 2, 18 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 },
		{ 2, 70 }, { 2, 67 }, { 2, 29 }, { 2, 22 }, { 2, 16 }, { 1, 0 }, { 1, 21 }, { 1, 42 },
		{ 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 77 }, { 2, 49 }, { 2, 31 },
		{ 2, 5 }, { 2, 0 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 84 }, { 1, 85 },
		{ 1, 87 }, { 1, 88 }, { 2, 72 }, { 2, 45 }, { 2, 27 }, { 2, 1 }, { 2, 13 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 68 },
		{ 2, 44 }, { 2, 28 }, { 2, 0 }, { 2, 13 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 },
		{ 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 80 }, { 2, 55 }, { 2, 40 }, { 2, 15 }, { 2, 7 },
		{ 2, 20 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 97 }, { 1, 98 },
		{ 1, 99 }, { 2, 71 }, { 2, 65 }, { 2, 27 }, { 2, 20 }, { 2, 13 }, { 2, 15 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 79 }, { 2, 81 },
		{ 2, 57 }, { 2, 50 }, { 2, 24 }, { 2, 17 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 },
		{ 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 79 }, { 2, 77 }, { 2, 70 }, { 2, 43 }, { 2, 15 },
		{ 2, 7 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 1, 79 }, { 2, 73 }, { 2, 69 }, { 2, 43 }, { 2, 28 }, { 2, 12 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 79 }, { 2, 52 },
		{ 2, 27 }, { 2, 20 }, { 2, 4 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 75 }, { 2, 49 }, { 2, 31 }, { 2, 13 }, { 2, 0 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 },
		{ 2, 72 }, { 2, 49 }, { 2, 42 }, { 2, 16 }, { 2, 19 }, { 1, 0 }, { 1, 21 }, { 1, 42 },
		{ 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 79 }, { 2, 58 }, { 2, 31 }, { 2, 23 },
		{ 2, 16 }, { 2, 22 }, { 1, 0 }, { 1, 21 }, { 1, 41 }, { 1, 63 }, { 1, 84 }, { 1, 85 },
		{ 1, 77 }, { 1, 78 }, { 2, 53 }, { 2, 25 }, { 2, 22 }, { 2, 5 }, { 2, 0 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 69 }, { 2, 40 },
		{ 2, 43 }, { 2, 17 }, { 2, 15 }, { 2, 11 }, { 2, 24 }, { 1, 0 }, { 1, 20 }, { 1, 41 },
		{ 1, 63 }, { 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 89 }, { 2, 62 }, { 2, 45 },
		{ 2, 18 }, { 2, 11 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 97 },
		{ 1, 99 }, { 1, 100 }, { 2, 97 }, { 2, 74 }, { 2, 68 }, { 2, 40 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 80 }, { 1, 81 }, { 2, 74 }, { 2, 69 },
		{ 2, 33 }, { 2, 25 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 75 }, { 1, 95 },
		{ 1, 97 }, { 2, 74 }, { 2, 59 }, { 2, 42 }, { 2, 5 }, { 2, 0 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 85 }, { 1, 77 }, { 1, 78 }, { 2, 51 }, { 2, 45 },
		{ 2, 7 }, { 2, 19 }, { 2, 13 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 86 }, { 1, 88 }, { 1, 79 }, { 2, 48 }, { 2, 42 }, { 2, 4 }, { 2, 16 }, { 2, 10 },
		{ 2, 22 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 1, 89 }, { 2, 88 }, { 2, 71 }, { 2, 44 }, { 2, 21 }, { 2, 14 }, { 1, 0 }, { 1, 21 },
		{ 1, 41 }, { 1, 63 }, { 1, 84 }, { 1, 85 }, { 1, 87 }, { 1, 84 }, { 2, 79 }, { 2, 62 },
		{ 2, 33 }, { 2, 26 }, { 2, 19 }, { 1, 0 }, { 1, 21 }, { 1, 41 }, { 1, 63 }, { 1, 84 },
		{ 1, 85 }, { 1, 87 }, { 1, 88 }, { 2, 59 }, { 2, 53 }, { 2, 25 }, { 2, 19 }, { 2, 14 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 },
		{ 2, 76 }, { 2, 49 }, { 2, 42 }, { 2, 5 }, { 2, 7 }, { 1, 0 }, { 1, 21 }, { 1, 42 },
		{ 1, 64 }, { 1, 85 }, { 1, 100 }, { 1, 101 }, { 1, 102 }, { 2, 86 }, { 2, 59 }, { 2, 52 },
		{ 2, 26 }, { 2, 9 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 },
		{ 1, 78 }, { 1, 79 }, { 2, 59 }, { 2, 32 }, { 2, 24 }, { 2, 7 }, { 2, 23 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 97 }, { 1, 98 }, { 2, 95 }, { 2, 89 },
		{ 2, 63 }, { 2, 45 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 },
		{ 1, 88 }, { 1, 83 }, { 2, 76 }, { 2, 70 }, { 2, 39 }, { 2, 25 }, { 2, 18 }, { 1, 0 },
		{ 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 79 },
		{ 2, 51 }, { 2, 24 }, { 2, 11 }, { 2, 3 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 },
		{ 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 }, { 2, 75 }, { 2, 47 }, { 2, 31 }, { 2, 4 },
		{ 2, 0 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 },
		{ 1, 89 }, { 2, 71 }, { 2, 44 }, { 2, 39 }, { 2, 11 }, { 2, 13 }, { 1, 0 }, { 1, 21 },
		{ 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 78 }, { 1, 78 }, { 2, 56 }, { 2, 39 },
		{ 2, 22 }, { 2, 5 }, { 2, 23 }, { 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 },
		{ 1, 96 }, { 1, 98 }, { 1, 99 }, { 2, 73 }, { 2, 66 }, { 2, 39 }, { 2, 21 }, { 2, 14 },
		{ 1, 0 }, { 1, 21 }, { 1, 42 }, { 1, 64 }, { 1, 85 }, { 1, 86 }, { 1, 88 }, { 1, 89 },
		{ 2, 48 }, { 2, 42 }, { 2, 14 }, { 2, 18 }
	};
	
	ASSERT_NO_FATAL_FAILURE(TestDelayCalculator(TestData::FramesArrivalInfo, expectedLatencies, expectedDelays));
}
