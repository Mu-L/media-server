#include "H26xPacketizer.h"

#include "video.h"
#include "h264/h264.h"

H26xPacketizer::H26xPacketizer(VideoCodec::Type codec) :
	RTPPacketizer(MediaFrame::Video, codec)
{}

void H26xPacketizer::EmitNal(VideoFrame& frame, BufferReader nal, std::string& fuPrefix, int naluHeaderSize)
{
	//UltraDebug("-H26xPacketizer::EmitNal()\n");
	//Empty prefix
	uint8_t prefix[4] = {};

	//Set NAL size on prefix
	set4(prefix, 0, nal.GetLeft());

	//Allocate enough space to prevent further reallocations
	frame.Alloc(frame.GetLength() + sizeof(prefix) + nal.GetLeft());

	//Add nal start
	frame.AppendMedia(prefix, sizeof(prefix));

	//Check nal start in frame
	auto pos = frame.AppendMedia(nal.PeekData(), nal.GetLeft());

	//If it is small
	if (nal.GetLeft() < RTPPAYLOADSIZE)
	{
		//Single packetization
		frame.AddRtpPacket(pos, nal.GetLeft());
		return;
	}

	//Otherwise use FU
	/*
	* The FU header (last byte of passed fuPrefix) has the following format:
	*
	*      +---------------+
	*      |0|1|2|3|4|5|6|7|
	*      +-+-+-+-+-+-+-+-+
	*      |S|E|R|  Type   |
	*      +---------------+
	*
	* For H.264, R must be 0. For H.265, R is part of the type.
	*/
	//Start with S = 1, E = 0
	size_t fuPrefixSize = fuPrefix.size();
	fuPrefix[fuPrefixSize - 1] &= 0b00'111111;
	fuPrefix[fuPrefixSize - 1] |= 0b10'000000;

	//Skip payload nal header
	nal.Skip(naluHeaderSize);
	pos += naluHeaderSize;
	uint32_t numPackets = std::ceil(static_cast<double>(nal.GetLeft()) / (RTPPAYLOADSIZE - fuPrefixSize));
	auto packetLen = nal.GetLeft() / numPackets;
	int mod = nal.GetLeft() % numPackets;

	for (uint32_t i=0; i<numPackets; i++)
	{
		auto len = packetLen + (mod>0 ? 1:0);
		if (i == numPackets-1)
			//Last fragment, set End bit
			fuPrefix[fuPrefixSize - 1] |= 0b01'000000;
		frame.AddRtpPacket(pos, len, (uint8_t*) fuPrefix.data(), fuPrefixSize);
		//Clear Start bit
		fuPrefix[fuPrefixSize - 1] &= 0b01'111111;
		pos += len;
		mod--;
		nal.Skip(len);
	}
}

std::unique_ptr<MediaFrame> H26xPacketizer::ProcessAU(BufferReader& reader)
{
	//UltraDebug("-H26xPacketizer::ProcessAU() | H26x AU [len:%d]\n", reader.GetLeft());
	
	if (!currentFrame)
		currentFrame = std::make_unique<VideoFrame>(static_cast<VideoCodec::Type>(codec), reader.GetLeft());

	std::optional<bool> frameEnd;
	NalSliceAnnexB(reader
		, [&](auto nalReader){OnNal(*currentFrame, nalReader, frameEnd);}
	);
	
	// If frame end is not got, regard it as the frame complete.
	if (frameEnd.has_value() && !frameEnd.value())
	{
		return nullptr;
	}

	//Check if we have new width and heigth
	if (currentFrame->GetWidth() && currentFrame->GetHeight())
	{
		//Update cache
		width = currentFrame->GetWidth();
		height = currentFrame->GetHeight();
	} else {
		//Update from cache
		currentFrame->SetWidth(width);
		currentFrame->SetHeight(height);
	}

	return std::move(currentFrame);
}

void H26xPacketizer::ResetFrame()
{
	currentFrame.reset();
}	