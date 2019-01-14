/**
 *  \file   FdSet.h
 *  \author Jason Fernandez
 *  \date   4/1/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#ifndef __FDSET_H__
#define __FDSET_H__

#include "SharedFd.h"
#include "Signal.h"

#include <sys/epoll.h>
#include <vector>

/**
 **********************************************************************
 *
 * @class FdSet
 *
 * Maintains a set of file descriptors that can be polled for events,
 * using the POSIX epoll(4) API. Because of this, file descriptor
 * events can be either edge or level triggered; see the epoll(4) man
 * page for details
 *
 **********************************************************************
 */
class FdSet
{
	/**
	 * File descriptor events considered "exceptional"
	 */
	static const int except_cond =
		 EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP;

	/**
	 * Maintains the information of a single registered
	 * file descriptor
	 */
	struct FdInfo
	{
		FdInfo();

		FdInfo(SharedFd _fd, short _events );
		
		FdInfo(FdInfo&& other);

		~FdInfo();
		
		FdInfo& operator=(FdInfo&& rhs);

		/** The file descriptor to monitor for events */
		SharedFd fd;

		/** The events to monitor for */
		short events;
	};

public:

	FdSet();

	FdSet(FdSet&& other);

	~FdSet();

	FdSet& operator=(FdSet&& rhs);

	bool poll(int timeout = -1);

	bool poll_read( int timeout = -1);

	bool poll_write(int timeout = -1);

	bool push_back(SharedFd fd,
		short events);

	/**
	 * A signal that gets raised whenever some event occurs on
	 * a file descriptor. The signal handler is provided the file
	 * descriptor on which the event occured
	 */
	typedef Signal::Signal<bool,const SharedFd&> sig_t;

	/**
	 * The signal that is raised whenever data is available for
	 * reading on a file descriptor
	 */
	sig_t read_sig;

	/**
	 * The signal that is raised whenever space is available for
	 * writing on a file descriptor
	 */
	sig_t write_sig;

	/**
	 * The signal that is raised whenever an exception occurs on
	 * a file descriptor. Currently this is anything other than
	 * read or write events
	 */
	sig_t except_sig;

private:

	bool _poll(int timeout, short ev);

	/**
	 * The value returned by ::epoll_create()
	 */
	int _epfd;

	/**
	 * The returned file descriptor events
	 */
	struct epoll_event events[FD_SETSIZE];

	/**
	 * The file descriptors being monitored
	 */
	std::vector<FdInfo>
		_fds;
};

#endif
