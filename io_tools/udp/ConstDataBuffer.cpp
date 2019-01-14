/**
 *  \file   ConstDataBuffer.cpp
 *  \author Jason Fernandez
 *  \date   4/19/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#include "abort.h"
#include "ConstDataBuffer.h"

namespace net
{
	/**
	 * Default constructor
	 */
	ConstDataBuffer::ConstDataBuffer() : _buf(nullptr), _size(0)
	{
	}

	/**
	 * Constructor
	 *
	 * @param[in] buf  The raw buffer
	 * @param[in] size The size of the given buffer, in bytes
	 */
	ConstDataBuffer::ConstDataBuffer(const char* buf, size_t size)
		: _buf(buf), _size(size)
	{
	}

	/**
	 * Destructor
	 */
	ConstDataBuffer::~ConstDataBuffer()
	{
	}

	/**
	 * C++ boolean type conversion. Returns true if the internal
	 * buffer is valid (non-null)
	 *
	 * @return True if we're holding a valid buffer
	 */
	ConstDataBuffer::operator bool() const
	{
		return _buf != nullptr;
	}

	/**
	 * Clear the buffer. Nulls the internal pointer and sets the
	 * size to zero; equivalent to \ref reset(0,0)
	 */
	void ConstDataBuffer::clear()
	{
		_buf = nullptr; _size = 0;
	}

	/**
	 * Get the buffer currently being handled
	 *
	 * @return The internal buffer
	 */
	const char* ConstDataBuffer::get() const
	{
		return _buf;
	}

	/**
	 * Indexing operator
	 *
	 * @param[in] index Get the byte at this index
	 *
	 * @return A reference to the byte at index \a index
	 */
	const char& ConstDataBuffer::operator[](size_t index) const
	{
		AbortIfNot(index < _size, _buf[index]);
		return _buf[index];
	}

	/**
	 * Handle a new buffer
	 *
	 * @param [in] buf   The raw buffer
	 * @param [in] size  The size of the given buffer, in bytes
	 */
	void ConstDataBuffer::reset(const char* buf, size_t size)
	{
		_buf = buf; _size = size;
	}

	/**
	 * Get the size of the buffer, in bytes
	 *
	 * @return The buffer size
	 */
	size_t ConstDataBuffer::size() const
	{
		return _size;
	}
}
