/**
 *  \file   ConstDataBuffer.h
 *  \author Jason Fernandez
 *  \date   4/19/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#ifndef __CONST_DATA_BUFFER_H__
#define __CONST_DATA_BUFFER_H__

#include <cstddef>

namespace net
{
	/**
	 * A convenience class for handling a fixed-size buffer by
	 * wrapping a pointer, size pair into a single class.
	 * Note that this class does not manage the buffer at all,
	 * e.g. no dynamic memory operations are performed
	 */
	class ConstDataBuffer
	{

	public:

		ConstDataBuffer();

		ConstDataBuffer(const char* buf,size_t size);

		~ConstDataBuffer();

		explicit operator bool() const;

		void clear();

		const char* get() const;

		const char& operator[]( size_t index ) const;

		void reset(const char* buf, size_t size);

		size_t size() const;

	private:

		/**
		 * The internal data buffer
		 */
		const char* _buf;

		/**
		 * The buffer size (bytes)
		 */
		size_t _size;
	};
}

#endif
