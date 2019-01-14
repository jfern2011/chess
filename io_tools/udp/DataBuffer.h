/**
 *  \file   DataBuffer.h
 *  \author Jason Fernandez
 *  \date   4/19/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#ifndef __DATA_BUFFER_H__
#define __DATA_BUFFER_H__

#include <cstddef>

namespace net
{
	/**
	 * A convenience class for handling a fixed-size buffer by
	 * wrapping a pointer, size pair into a single class.
	 * Note that this class does not manage the buffer at all,
	 * e.g. no dynamic memory operations are performed
	 */
	class DataBuffer
	{

	public:

		DataBuffer();

		DataBuffer(char* buf, size_t size);

		~DataBuffer();

		explicit operator bool() const;

		bool copy(const char* buf, size_t size, size_t offset = 0);

		void clear();

		char* get();

		const char* get() const;

		char operator[](size_t index) const;

		char& operator[](size_t index);

		void reset( char* buf, size_t size);

		size_t size() const;

	private:

		/**
		 * The internal data buffer
		 */
		char* _buf;

		/**
		 * The buffer size (bytes)
		 */
		size_t _size;
	};
}

#endif
