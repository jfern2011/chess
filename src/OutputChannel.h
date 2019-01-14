#ifndef __OUTPUT_CHANNEL_H__
#define __OUTPUT_CHANNEL_H__

#include <cstddef>

#include "abort/abort.h"
#include "util/str_util.h"

namespace Chess
{
	/**
	 * An abstract interface for sending engine outputs any
	 * some user-defined destination
	 */
	class OutputChannel
	{

	public:

		OutputChannel();

		virtual ~OutputChannel();

		/**
		 * Output stream operator. Note that mutliple
		 * calls can be chained
		 *
		 * @tparam T The type of \a data
		 *
		 * @param [in] data The data element to write
		 *
		 * @return *this
		 */
		template <typename T>
		OutputChannel& operator<<(const T& data)
		{
			std::string data_s;
			AbortIfNot(Util::to_string(data, data_s), *this);
			AbortIfNot(write(
				data_s.c_str(), data_s.size()), *this);
			return *this;
		}

		/**
		 * Write to the output channel
		 *
		 * @param[in] buf  The buffer to write
		 * @param[in] size Number of bytes to write
		 *
		 * @return True on success
		 */
		virtual bool write(const char* buf,
			size_t size) = 0;

	protected:

	};
}

#endif
