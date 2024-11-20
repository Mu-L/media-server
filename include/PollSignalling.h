#ifndef POLLSIGNALLING_H
#define POLLSIGNALLING_H

#include "FileDescriptor.h"
#include <atomic>
#include <memory>

class EventPipe
{
public:
	EventPipe();
	inline const FileDescriptor& GetReadFd () const
	{
#if __APPLE__
		return pipeFds[0];
#else
		return eventFd;
#endif
	}
	inline const FileDescriptor& GetWriteFd () const
	{
#if __APPLE__
		return pipeFds[1];
#else
		return eventFd;
#endif
	}
private:
#if __APPLE__
	FileDescriptor pipeFds[2];
#else
	FileDescriptor eventFd;
#endif
};

/**
 * A class to wrap a pipe for poll signalling event.
*/
class PollSignalling
{
public:
	void Signal();
	void ClearSignal();
	
	inline int GetFd() const
	{
		return eventPipe.GetReadFd();
	}
	
private:
	EventPipe eventPipe;
	std::atomic_flag signaled = ATOMIC_FLAG_INIT;
};

#endif
