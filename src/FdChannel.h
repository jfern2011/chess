#ifndef __FD_CHANNEL_H__
#define __FD_CHANNEL_H__

#include "io_tools/Fd/Fd.h"
#include "OutputChannel.h"

namespace Chess
{
	/**
	 * A channel that writes to a file descriptor
	 */
	class FdChannel : public OutputChannel
	{

	public:

		FdChannel(Fd&& fd);

		virtual ~FdChannel();

		bool set_blocking(bool value);

		bool write(const char* buf, size_t size )
			override;

	private:

		/**
		 * The underlying file descriptor
		 */
		Fd _fd;
	};
}

#endif
