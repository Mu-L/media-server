/*
 * File:   h265depacketizer.cpp
 * Author: Sergio
 *
 * Created on 26 de enero de 2012, 9:46
 */

#include "H265Depacketizer.h"
#include "h265.h"
#include "media.h"
#include "codecs.h"
#include "rtp.h"
#include "log.h"


H265Depacketizer::H265Depacketizer() : RTPDepacketizer(MediaFrame::Video, VideoCodec::H265), frame(VideoCodec::H265, 0)
{
	//Set clock rate
	frame.SetClockRate(90000);
}

H265Depacketizer::~H265Depacketizer()
{
}

void H265Depacketizer::ResetFrame()
{
	//Clear packetization info
	frame.Reset();
	//Clear config
	//No fragments
	iniFragNALU = 0;
	startedFrag = false;
}

MediaFrame* H265Depacketizer::AddPacket(const RTPPacket::shared& packet)
{
	//Get timestamp in ms
	auto ts = packet->GetExtTimestamp();
	//Check it is from same packet
	if (frame.GetTimeStamp() != ts)
		//Reset frame
		ResetFrame();
	//If not timestamp
	if (frame.GetTimeStamp() == (DWORD)-1)
	{
		//Set timestamp
		frame.SetTimestamp(ts);
		//Set clock rate
		frame.SetClockRate(packet->GetClockRate());
		//Set time
		frame.SetTime(packet->GetTime());
		//Set sender time
		frame.SetSenderTime(packet->GetSenderTime());
	}
	//Set SSRC
	frame.SetSSRC(packet->GetSSRC());
	//Add payload
	AddPayload(packet->GetMediaData(), packet->GetMediaLength());
	//If it is last return frame
	if (!packet->GetMark())
		return NULL;
	//Return frame
	return &frame;
}

MediaFrame* H265Depacketizer::AddPayload(const BYTE* payload, DWORD payloadLen)
{
	BYTE nalHeader[4];
	BYTE S, E;
	DWORD pos;
	//Check length
	if (payloadLen<2)
		//Exit
		return NULL;

	/* 
     *   +-------------+-----------------+
     *   |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
     *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *   |F|   Type    |  LayerId  | TID |
     *   +-------------+-----------------+
	 *
	 * F must be 0.
	 */

	BYTE nalUnitType = (payload[0] & 0x7e) >> 1;
	BYTE nuh_layer_id = ((payload[0] & 0x1) << 5) + ((payload[1] & 0xf8) >> 3);
	BYTE nuh_temporal_id = payload[1] & 0x7;

	//Get nal data
	const BYTE* nalData = payload + 2;

	//Get nalu size
	DWORD nalSize = payloadLen;

	UltraDebug("-H265 [NAL header:0x%02x%02x,type:%d,layer_id:%d, temporal_id:%d, size:%d]\n", payload[0], payload[1], nalUnitType, nuh_layer_id, nuh_temporal_id, nalSize);

	//Check type
	switch (nalUnitType)
	{
		case HEVC_RTP_NALU_Type::AUD:		//35
		case HEVC_RTP_NALU_Type::EOS:		//36
		case HEVC_RTP_NALU_Type::EOB:		//37
		case HEVC_RTP_NALU_Type::FD:		//38
			Warning("-H265 Un-defined/implemented NALU, skipping");
			/* undefined */
			return nullptr;
		case HEVC_RTP_NALU_Type::UNSPEC48_AP:	//48 
			/**
			   Figure 7 presents an example of an AP that contains two aggregation
			   units, labeled as 1 and 2 in the figure, without the DONL and DOND
			   fields being present.

			    0                   1                   2                   3
			    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   |                          RTP Header                           |
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   |   PayloadHdr (Type=48)        |         NALU 1 Size           |
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   |          NALU 1 HDR           |                               |
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+         NALU 1 Data           |
			   |                   . . .                                       |
			   |                                                               |
			   +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   |  . . .        | NALU 2 Size                   | NALU 2 HDR    |
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   | NALU 2 HDR    |                                               |
			   +-+-+-+-+-+-+-+-+              NALU 2 Data                      |
			   |                   . . .                                       |
			   |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			   |                               :...OPTIONAL RTP padding        |
			   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

			   Figure 7: An Example of an AP Packet Containing Two Aggregation
			   Units without the DONL and DOND Fields

			*/
			Debug("-H265 TODO: non implemented yet, need update to rfc7798, return nullptr \n", payload[0], nalUnitType, nalSize);
			return nullptr;

			///* Skip SPayloadHdr */
			//payload+=2;
			//payloadLen-=2;

			///* STAP-A Single-time aggregation packet 5.7.1 */
			//while (payloadLen > 2)
			//{
			//	/* Get NALU size */
			//	nalSize = get2(payload, 0);

			//	/* strip NALU size */
			//	payload += 2;
			//	payloadLen -= 2;

			//	//Check
			//	if (!nalSize || nalSize > payloadLen)
			//		//Error
			//		break;

			//	//Get nal type
			//	nalUnitType = payload[0] & 0x1f;
			//	//Get data
			//	nalData = payload + 1;

			//	//Check if IDR SPS or PPS
			//	switch (nalUnitType)
			//	{
			//		case 19: //IDR
			//		case 20: //IDR
			//		case 33: //SPS
			//		case 34: //PPS
			//			//It is intra
			//			frame.SetIntra(true);
			//			break;
			//	}

			//	//Set size
			//	set4(nalHeader, 0, nalSize);
			//	//Append data
			//	frame.AppendMedia(nalHeader, sizeof(nalHeader));

			//	//Append data and get current post
			//	pos = frame.AppendMedia(payload, nalSize);
			//	//Add RTP packet
			//	frame.AddRtpPacket(pos, nalSize, NULL, 0);

			//	payload += nalSize;
			//	payloadLen -= nalSize;
			//}
			//break;
		case HEVC_RTP_NALU_Type::UNSPEC49_FU: 
			/*
						0                   1                   2                   3
					    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
					   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					   |    PayloadHdr (Type=49)       |   FU header   | DONL (cond)   |
					   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
					   | DONL (cond)   |                                               |
					   |-+-+-+-+-+-+-+-+                                               |
					   |                         FU payload                            |
					   |                                                               |
					   |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					   |                               :...OPTIONAL RTP padding        |
					   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

					                Figure 9: The Structure of an FU

			*/
			Debug("-H265 TODO: non implemented yet, need update to rfc7798, return nullptr \n", payload[0], nalUnitType, nalSize);
			return nullptr;

			////Check length
			//if (payloadLen < 3)
			//	return NULL;

			///* +---------------+
			// * |0|1|2|3|4|5|6|7|
			// * +-+-+-+-+-+-+-+-+
			// * |S|E|R| Type	   |
			// * +---------------+
			// *
			// * R is reserved and always 0
			// */
			//S = (payload[2] & 0x80) == 0x80;
			//E = (payload[2] & 0x40) == 0x40;

			///* strip off FU indicator and FU header bytes */
			//nalSize = payloadLen - 3;

			////if it is the start fragment of the nal unit
			//if (S)
			//{
			//	/* NAL unit starts here */
			//	BYTE fragNalHeader = (payload[0] & 0xe0) | (payload[2] & 0x1f);

			//	//Get nal type
			//	nalUnitType = fragNalHeader & 0x1f;

			//	//Check it
			//	if (nalUnitType == 0x05)
			//		//It is intra
			//		frame.SetIntra(true);

			//	//Get init of the nal
			//	iniFragNALU = frame.GetLength();
			//	//Set empty header, will be set later
			//	set4(nalHeader, 0, 0);
			//	//Append data
			//	frame.AppendMedia(nalHeader, sizeof(nalHeader));
			//	//Append NAL header
			//	frame.AppendMedia(&fragNalHeader, 1);
			//	//We have a start frag
			//	startedFrag = true;
			//}

			////If we didn't receive a start frag
			//if (!startedFrag)
			//	//Ignore
			//	return NULL;

			////Append data and get current post
			//pos = frame.AppendMedia(payload + 3, nalSize);
			////Add rtp payload
			//frame.AddRtpPacket(pos, nalSize, payload, 3);

			////If it is the end fragment of the nal unit
			//if (E)
			//{
			//	//Ensure it is valid
			//	if (iniFragNALU + 4 > frame.GetLength())
			//		//Error
			//		return NULL;
			//	//Get NAL size
			//	DWORD nalSize = frame.GetLength() - iniFragNALU - 4;
			//	//Set it
			//	set4(frame.GetData(), iniFragNALU, nalSize);
			//	//Done with fragment
			//	iniFragNALU = 0;
			//	startedFrag = false;
			//}
			////Done
			//break;
		default:
			/* 4.4.1.  Single NAL Unit Packets */
			/* 
					0                   1                   2                   3
				    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
				   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				   |           PayloadHdr          |      DONL (conditional)       |
				   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				   |                                                               |
				   |                  NAL unit payload data                        |
				   |                                                               |
				   |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				   |                               :...OPTIONAL RTP padding        |
				   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			*/
			/* @Zita Liao: sprop-max-don-diff is considered to be absent right now (so no DONL). Need to extract from SDP */
			/* the entire payload is the output buffer */
			nalSize = payloadLen;
			//Check if IDR/SPS/PPS, set Intra
			if ((nalUnitType == HEVC_RTP_NALU_Type::IDR_W_RADL)
				|| (nalUnitType == HEVC_RTP_NALU_Type::IDR_W_RADL)
				|| (nalUnitType == HEVC_RTP_NALU_Type::SPS)
				|| (nalUnitType == HEVC_RTP_NALU_Type::PPS))
			{
				frame.SetIntra(true);
			}

			switch (nalUnitType)
			{
				case HEVC_RTP_NALU_Type::SPS:			// 33
				{
					//Add full nal to config
					config.AddSequenceParameterSet(payload,nalSize);
					
					//Parse sps
					H265SeqParameterSet sps;
					if (sps.Decode(nalData,nalSize-1,nuh_layer_id))
					{
						//Set dimensions
						frame.SetWidth(sps.GetWidth());
						frame.SetHeight(sps.GetHeight());
	
						UltraDebug("-H265 frame (with cropping) size [width: %d, frame height: %d]\n", sps.GetWidth(), sps.GetHeight());

						//@Zita TODO: after sps.Decode(), get needed info from sps
						////Set config
						//config.SetConfigurationVersion(1);
						//config.SetAVCProfileIndication(nalData[0]);
						//config.SetProfileCompatibility(nalData[1]);
						//config.SetAVCLevelIndication(nalData[2]);
						//config.SetNALUnitLength(sizeof(nalHeader)-1);
					}
					break;
				}
				case HEVC_RTP_NALU_Type::PPS:			// 34
					//Add full nal to config
					config.AddPictureParameterSet(payload,nalSize);
				break;
			}
			//Set size
			set4(nalHeader, 0, nalSize);
			//Append data
			frame.AppendMedia(nalHeader, sizeof(nalHeader));
			//Append data and get current post
			pos = frame.AppendMedia(payload, nalSize);
			//Add RTP packet
			frame.AddRtpPacket(pos, nalSize, NULL, 0);
			//Done
			break;
	}

	return &frame;
}

