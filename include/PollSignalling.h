#ifndef POLLSIGNALLING_H
#define POLLSIGNALLING_H

#include "FileDescriptor.h"
#include <atomic>
#include <memory>

/**
 * A class to wrap a pipe for poll signalling event.
*/
class PollSignalling
{
public:
	PollSignalling();
	
	void Signal();
	void ClearSignal();
	
	inline int GetFd() const
	{
		return *pipeFds[0];
	}
	
private:
	std::shared_ptr<FileDescriptor> pipeFds[2];
	std::atomic_flag signaled = ATOMIC_FLAG_INIT;
};

#endif
