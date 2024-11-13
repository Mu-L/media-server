#ifndef _RTMPCONNECTION_H_
#define _RTMPCONNECTION_H_
#include "config.h"
#include "rtmp.h"
#include "rtmpchunk.h"
#include "rtmpmessage.h"
#include "rtmpstream.h"
#include "rtmpapplication.h"
#include "EventLoop.h"

#include <sys/socket.h>

#include <map>
#include <memory>
#include <atomic>

class RTMPConnection :
	public std::enable_shared_from_this<RTMPConnection>,
	public RTMPNetConnection::Listener,
	public RTMPMediaStream::Listener,
	public RTMPNetStream::Listener,
	public EventLoop
{
public:
	using shared = std::shared_ptr<RTMPConnection>;
	
	enum class ExitCode
	{
		ReadError = 1,
		ParseError = 2,
		PollError = 3,
		PollTimeout = 4,
		Disconnected = 5
	};
	
	class Listener
	{
	public:
		//Virtual desctructor
		virtual ~Listener(){};
	public:
		//Interface
		virtual RTMPNetConnection::shared OnConnect(
			const struct sockaddr_in& peername,
			const std::wstring& appName,
			RTMPNetConnection::Listener *listener,
			std::function<void(bool)> accept
		) = 0;
		virtual void onDisconnect(RTMPConnection* con) = 0;
	};
public:
	RTMPConnection();
	~RTMPConnection();
	
	void SetListener(Listener* listener);

	int Init(int fd);
	int End();
	
	int GetSocket() { return socket; }

	//Listener from NetConnection
	virtual void onNetConnectionStatus(QWORD transId,const RTMPNetStatusEventInfo &info,const wchar_t *message) override;
	virtual void onNetConnectionDisconnected() override;
	//Listener from NetStream
	virtual void onNetStreamDestroyed(DWORD id) override;
	virtual void onNetStreamStatus(DWORD streamId,QWORD transId,const RTMPNetStatusEventInfo &info,const wchar_t *message) override;
	//Listener for the media data
	virtual void onAttached(RTMPMediaStream *stream) override;
	virtual void onMediaFrame(DWORD id,RTMPMediaFrame *frame) override;
	virtual void onMetaData(DWORD id,RTMPMetaData *meta) override;
	virtual void onCommand(DWORD id,const wchar_t *name,AMFData* obj) override;
	virtual void onStreamBegin(DWORD id) override;
	virtual void onStreamEnd(DWORD id) override;
	//virtual void onStreamIsRecorded(DWORD id);
	virtual void onStreamReset(DWORD id) override;
	virtual void onDetached(RTMPMediaStream *stream) override;
	
	void OnLoopEnter() override;
	std::optional<uint16_t> GetPollEventMask(int fd) const override;
	void OnPollIn(int fd) override;
	void OnPollOut(int fd) override;
	void OnPollTimeout() override;
	void OnPollError(int fd, int errorCode) override;
	void OnLoopExit(int exitCode) override;
	
	DWORD GetRTT()	{ return rtt; }
protected:
	
	void PingRequest();
private:
	void ParseData(BYTE *data,const DWORD size);
	DWORD SerializeChunkData(BYTE *data,const DWORD size);
	int WriteData(BYTE *data,const DWORD size);

	void ProcessControlMessage(DWORD messageStremId,BYTE type,RTMPObject* msg);
	void ProcessCommandMessage(DWORD messageStremId,RTMPCommandMessage* cmd);
	void ProcessMediaData(DWORD messageStremId,RTMPMediaFrame* frame);
	void ProcessMetaData(DWORD messageStremId,RTMPMetaData* frame);

	void SendCommandError(DWORD messageStreamId,QWORD transId,AMFData* params = NULL,AMFData *extra = NULL);
	void SendCommandResult(DWORD messageStreamId,QWORD transId,AMFData* params,AMFData *extra);
	void SendCommandResponse(DWORD messageStreamId,const wchar_t* name,QWORD transId,AMFData* params,AMFData *extra);
	
	void SendCommand(DWORD messageStreamId,const wchar_t* name,AMFData* params,AMFData *extra);
	void SendControlMessage(RTMPMessage::Type type,RTMPObject* msg);
	
	void SignalWriteNeeded();
private:
	enum State {HEADER_C0_WAIT=0,HEADER_C1_WAIT=1,HEADER_C2_WAIT=2,CHUNK_HEADER_WAIT=3,CHUNK_TYPE_WAIT=4,CHUNK_EXT_TIMESTAMP_WAIT=5,CHUNK_DATA_WAIT=6};
	typedef std::map<DWORD,RTMPChunkInputStream*>  RTMPChunkInputStreams;
	typedef std::map<DWORD,RTMPChunkOutputStream*> RTMPChunkOutputStreams;
	typedef std::map<DWORD,RTMPNetStream::shared> RTMPNetStreams;
private:
	int socket;
	volatile bool inited;
	State state;

	RTMPHandshake01 s01;
	RTMPHandshake0 c0;
	RTMPHandshake1 c1;
	RTMPHandshake2 s2;
	RTMPHandshake2 c2;
	
	bool digest;

	DWORD videoCodecs = 0;
	DWORD audioCodecs = 0;

	RTMPChunkBasicHeader header;
	RTMPChunkType0	type0;
	RTMPChunkType1	type1;
	RTMPChunkType2	type2;
	RTMPExtendedTimestamp extts;

	RTMPChunkInputStreams  	chunkInputStreams;
	RTMPChunkOutputStreams  chunkOutputStreams;
	RTMPChunkInputStream* 	chunkInputStream = nullptr;

	DWORD chunkStreamId = 0;
	DWORD chunkLen = 0;
	DWORD maxChunkSize;
	DWORD maxOutChunkSize;

	RTMPNetConnection::shared app;
	std::wstring	 appName;
	RTMPNetStreams	 streams;
	DWORD maxStreamId;
	DWORD maxTransId;

	std::mutex mutex;
	Listener* listener = nullptr;

	timeval startTime;
	DWORD windowSize;
	DWORD curWindowSize;
	DWORD recvSize;
	DWORD inBytes;
	DWORD outBytes;

	QWORD bandIni = 0;
	DWORD bandSize = 0;
	DWORD bandCalc = 0;
	
	DWORD rtt = 0;
	
	uint16_t eventMask = 0;
	
	static constexpr unsigned int BufferSize = 1400;
	mutable BYTE buffer[BufferSize];
};

#endif
