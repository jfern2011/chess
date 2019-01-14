/**
 *  \file   FdSet.cpp
 *  \author Jason Fernandez
 *  \date   4/1/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#include "abort.h"
#include "FdSet.h"

#include <unistd.h>
#include <utility>

/**
 * Default constructor
 */
FdSet::FdInfo::FdInfo() : fd(), events(0)
{
}

/**
 * Constructor
 *
 * @param[in] _fd     The file descriptor to monitor
 * @param[in] _events Monitor for these events
 */
FdSet::FdInfo::FdInfo(SharedFd _fd, short _events)
	: fd(_fd), events(_events)
{
}

/**
 * Move constructor
 *
 * @param [in] other The FdInfo to take ownership of
 */
FdSet::FdInfo::FdInfo(FdInfo&& other)
{
	*this = std::move(other);
}

/**
 * Destructor
 */
FdSet::FdInfo::~FdInfo()
{
}

/**
 * Move assignment
 *
 * @param [in] rhs  The FdInfo to take ownership of
 *
 * @return *this
 */
auto FdSet::FdInfo::operator=(FdInfo&& rhs) -> decltype(*this)
{
	if (this != &rhs)
	{
		fd     = std::move(rhs.fd);
		events = rhs.events;
	}

	return *this;
}

/**
 * Default constructor
 */
FdSet::FdSet(): _epfd(::epoll_create1(0)), _fds()
{
}

/**
 * Move constructor
 *
 * @param[in] other An FdSet to take ownership of
 */
FdSet::FdSet(FdSet&& other)
{
	*this = std::move(other);
}

/**
 * Destructor
 */
FdSet::~FdSet()
{
	if (0 <= _epfd) ::close(_epfd);
}

/**
 * Move assignment
 *
 * @param [in] rhs  An FdSet to take ownership of
 *
 * @return *this
 */
FdSet& FdSet::operator=(FdSet&& rhs)
{
	if (this != &rhs)
	{
		_fds  = std::move(rhs._fds);
		_epfd = rhs._epfd;

		except_sig = std::move(rhs.except_sig);
		read_sig   = std::move(rhs.read_sig);
		write_sig  = std::move(rhs.write_sig );

		rhs._epfd = -1;
	}

	return *this;
}

/**
 *  Poll for the file descriptor set for events. This will dispatch
 *  callbacks to handle each event
 *
 * @param[in] timeout Block for at least this many milliseconds. If
 *                    negative, block indefinitely
 *
 * @return True on success
 */
bool FdSet::poll(int timeout)
{
	AbortIfNot(_poll(timeout, EPOLLIN | EPOLLOUT), false);

	return true;
}

/**
 * Poll for read events, dispatching the handler for each
 *
 * @param[in] timeout Block for at least this many milliseconds. If
 *                    negative, block indefinitely
 *
 * @return True on success
 */
bool FdSet::poll_read(int timeout)
{
	AbortIfNot(_poll(timeout, EPOLLIN), false);

	return true;
}

/**
 * Poll for write events, dispatching the handler for each
 *
 * @param[in] timeout Block for at least this many milliseconds. If
 *                    negative, block indefinitely
 *
 * @return True on success
 */
bool FdSet::poll_write(int timeout)
{
	AbortIfNot(_poll(timeout, EPOLLOUT), false);

	return true;
}

/**
 * Adds a new file descriptor to monitor for events
 *
 * @param[in] fd     The file descriptor
 * @param[in] events The events to monitor the file descriptor for
 *                   as described in the epoll_ctl(2) man page.
 *                   All event types described there are supported,
 *                   including EPOLLET
 * 
 * @return True on success
 */
bool FdSet::push_back(SharedFd fd, short events)
{
	AbortIfNot(fd, false);

	struct epoll_event ev;

	ev.data.fd  = fd.get();
	ev.events   = events | (EPOLLRDHUP|EPOLLPRI);
	ev.data.u32 = _fds.size();

	AbortIf(_epfd < 0, false);
	AbortIf(::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd.get(), &ev) < 0,
		false);

	AbortIf( _fds.size() == FD_SETSIZE,
		false);

	_fds.push_back(FdInfo(fd, events));
	return true;
}

/**
 * Poll all file descriptors for events
 *
 * @param[in] timeout Block for at least this many milliseconds. If
 *                    negative, block indefinitely
 * @param[in] ev      Which events to poll for. Note that we always
 *                    check for exceptions
 *
 * @return True on success
 */
bool FdSet::_poll(int timeout, short ev)
{
	AbortIf(_epfd < 0, false);

	int n_ready = ::epoll_wait(_epfd, events, _fds.size(), timeout);
	AbortIf(n_ready < 0, false);

	for (int i = 0; i < n_ready; i++)
	{
		const FdInfo& info = _fds[events[i].data.u32];

		if ((events[i].events & except_cond)
				&& except_sig.is_connected())
		{
			AbortIfNot(except_sig.raise(info.fd),
				false);
		}

		if ((events[i].events & EPOLLIN & ev)
				&& read_sig.is_connected())
		{
			AbortIfNot(read_sig.raise(info.fd),
				false);
		}

		if ((events[i].events & EPOLLOUT & ev)
				&& write_sig.is_connected())
		{
			AbortIfNot(write_sig.raise(info.fd),
				false);
		}
	}

	return true;
}
