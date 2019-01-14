/**
 *  \file   DataBuffer.cpp
 *  \author Jason Fernandez
 *  \date   4/19/2018
 *
 *  https://github.com/jfern2011/io_tools
 */

#include <cstring>

#include "abort.h"
#include "DataBuffer.h"

namespace net
{
	/**
	 * Default constructor
	 */
	DataBuffer::DataBuffer() : _buf(nullptr), _size(0)
	{
	}

	/**
	 * Constructor
	 *
	 * @param [in] buf    The raw buffer
	 * @param [in] size   The size of the given buffer, in bytes
	 */
	DataBuffer::DataBuffer(char* buf, size_t size)
		: _buf(buf), _size(size)
	{
	}

	/**
	 * Destructor
	 */
	DataBuffer::~DataBuffer()
	{
	}

	/**
	 * C++ boolean type conversion. Returns true if the internal
	 * buffer is valid (non-null)
	 *
	 * @return True if we're holding a valid buffer
	 */
	DataBuffer::operator bool() const
	{
		return _buf != nullptr;
	}

	/**
	 * Deep copy a given buffer to the internal (stored) buffer
	 *
	 * @param[in] buf    The source data
	 * @param[in] size   The number of bytes to copy
	 * @param[in] offset The offset from the internal buffer at
	 *                   which to start copying
	 *
	 * @return True on success, or false if copying would
	 *         overrun the buffer
	 */
	bool DataBuffer::copy(const char* buf, size_t size,
		size_t offset)
	{
		AbortIf(offset + size > _size, false);

		std::memcpy(_buf + offset, buf, size);
		return true;
	}

	/**
	 * Clear the buffer. Nulls the internal pointer and sets the
	 * size to zero; equivalent to \ref reset(0,0)
	 */
	void DataBuffer::clear()
	{
		_buf = nullptr; _size = 0;
	}

	/**
	 * Get the buffer currently being handled
	 *
	 * @return The internal buffer
	 */
	char* DataBuffer::get()
	{
		return _buf;
	}

	/**
	 * Get the buffer currently being handled
	 *
	 * @return The internal buffer
	 */
	const char* DataBuffer::get() const
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
	char& DataBuffer::operator[](size_t index)
	{
		AbortIfNot(index < _size, _buf[index]);
		return _buf[index];
	}

	/**
	 * Indexing operator
	 *
	 * @param[in] index Get the byte at this index
	 *
	 * @return The byte at index \a index
	 */
	char DataBuffer::operator[](size_t index) const
	{
		AbortIfNot(index < _size, _buf[index]);
		return _buf[index];
	}

	/**
	 * Handle a new buffer
	 *
	 * @param [in] buf    The raw buffer
	 * @param [in] size   The size of the given buffer, in bytes
	 */
	void DataBuffer::reset(char* buf, size_t size)
	{
		_buf = buf; _size = size;
	}

	/**
	 * Get the size of the buffer, in bytes
	 *
	 * @return The buffer size
	 */
	size_t DataBuffer::size() const
	{
		return _size;
	}
}
