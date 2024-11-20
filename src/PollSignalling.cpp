#include "PollSignalling.h"

#include "log.h"

#include <sys/eventfd.h>

#if __APPLE__
EventPipe::EventPipe()
{
	int pipe[2] = { FD_INVALID, FD_INVALID };
	//Create pipe
	if (::pipe (pipe) == -1)
	{
		//Error
		throw std::runtime_error("Failed to create pipe");
	}

	//Set non blocking
	fcntl(pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(pipe[1], F_SETFL, O_NONBLOCK);

	pipeFds[0] = FileDescriptor(pipe[0]);
	pipeFds[1] = FileDescriptor(pipe[1]);
}
#else
EventPipe::EventPipe()
	: eventFd(eventfd(0, EFD_NONBLOCK))
{
	if (!eventFd.isValid())
		throw std::runtime_error("Failed to event fd");
}
#endif

void PollSignalling::Signal()
{	
	uint64_t one = 1;

	if (signaled.test_and_set() || !eventPipe.GetWriteFd().isValid())
		//No need to do anything
		return;
			
	//We have signaled it above
	//worst case scenario is that race happens between this to points
	//and that we signal it twice
	
	//Write to tbe pipe, and assign to one to avoid warning in compile time
	one = write(eventPipe.GetWriteFd(), (uint8_t*)&one, sizeof(one));
}

void PollSignalling::ClearSignal()
{
	if (eventPipe.GetReadFd().isValid())
	{
		uint64_t data = 0;
		//Remove pending data on signal pipe
		while (read(eventPipe.GetReadFd(), &data, sizeof(uint64_t)) > 0)
		{
			//DO nothing
		}
	}
	
	//We are not signaled anymore
	signaled.clear();
}
	
