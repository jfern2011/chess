#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "abort.h"
#include "types.h"

/**
 **********************************************************************
 *
 * A wrapper for a simple C++ array. Aside from behaving like a normal
 * array, this performs bounds checking to make it easier to catch
 * buffer overflows at runtime. This is used during many time-critical
 * computations and is therefore kept as simple as possible
 *
 * @tparam T The type of each buffer element
 * @tparam N The size of the buffer
 *
 **********************************************************************
 */
template <typename T, int N>
class Buffer
{
	static_assert(N > 0, "Buffer must contain at least 1 item.");

public:

	/**
	 * Constructor
	 */
	Buffer()
	{
	}

	/**
	 * Destructor
	 */
	~Buffer()
	{
	}

	/**
	 * Indexing operator. This will return a reference to the element
	 * at the specified index
	 *
	 * @param[in] index The index to look up
	 *
	 * @return The element at \a index, or the first element on error
	 */
	inline T& operator[](uint32 index)
	{
		AbortIf(N <= index, data[0]);

		return data[index];
	}

	/**
	 * Deference operator
	 *
	 * @return The first element in this Buffer
	 */
	inline T& operator*()
	{
		return data[0];
	}

	/**
	 * Pointer arithmetic (addition)
	 *
	 * @param[in] offset The offset to add to the start of the buffer
	 *
	 * @return  A pointer to the address at offset \a offset from the
	 *          buffer start, or nullptr on error
	 */
	inline T* operator+(uint32 offset)
	{
		AbortIf(N <= offset, nullptr);

		return data + offset;
	}

private:

	T data[N];
};

#endif
