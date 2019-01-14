#include "abort/abort.h"
#include "FdChannel.h"

#include <unistd.h>
#include <utility>

namespace Chess
{
	/**
	 * Constructor
	 *
	 * @param[in] fd The file descriptor to write to
	 */
	FdChannel::FdChannel(Fd&& fd)
		: _fd(std::move(fd))
	{
	}

	/**
	 * Destructor
	 */
	FdChannel::~FdChannel()
	{
	}

	/**
	 * Set the blocking behavior of the file
	 *
	 * @param value True for blocking, false otherwise
	 *
	 * @return True on success
	 */
	bool FdChannel::set_blocking(bool value)
	{
		AbortIfNot(_fd.set_blocking(value), false);
		return true;
	}

	/**
	 * See \ref OutputChannel::write()
	 */
	bool FdChannel::write(const char* buf, size_t size)
	{
		AbortIf(::write(_fd.get(), buf, size)
			!= (int)size, false);

		return true;
	}
}
