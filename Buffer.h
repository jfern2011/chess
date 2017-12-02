#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "abort.h"
#include "types.h"

/**
 **********************************************************************
 *
 * A wrapper for a C++ multi-dimensional array. This implements bounds
 * checking to make it easier to catch buffer overflows at runtime.
 * This class is kept as simple as possible since it's used many times
 * during searches
 *
 * @tparam T  The type of each element
 * @tparam N1 Number of elements along the 1st dimension
 * @tparam N2 Number of elements along higher dimensions
 *
 **********************************************************************
 */
template <typename T, int N1, int... N2>
class Buffer
{
	static_assert(N1 > 0, "Dimensions must be greater than zero.");

public:

	Buffer()
	{
	}

	~Buffer()
	{
	}

	/**
	 * Indexing operator. This call produces a Buffer whose number of
	 * dimensions is reduced by 1
	 *
	 * For example, if we have a Buffer<int,2,3>, then we'll get
	 * back a Buffer<int,3>
	 *
	 * @param[in] index The index to look up
	 *
	 * @return The element at \a index, or the first element on error
	 */
	inline Buffer<T,N2...>& operator[](uint32 index)
	{
		AbortIf(N1 <= index, data[0]);

		return data[index];
	}

	/**
	 * Indexing operator. This call produces a Buffer whose number of
	 * dimensions is reduced by 1
	 *
	 * For example, if we have a Buffer<int,2,3>, then we'll get
	 * back a Buffer<int,3>
	 *
	 * @param[in] index The index to look up
	 *
	 * @return A *const* reference to the element at \a index, or the
	 *         first element on error
	 */
	inline const Buffer<T,N2...>& operator[](uint32 index) const
	{
		AbortIf(N1 <= index, data[0]);

		return data[index];
	}

private:

	Buffer<T,N2...> data[N1];
};

/**
 **********************************************************************
 *
 * A wrapper for a simple C++ array. Aside from behaving like a normal
 * array, this performs bounds checking to make it easier to catch
 * buffer overflows at runtime. Because this is used many times during
 * searches, it is kept as simple as possible
 *
 * @tparam T The type of each buffer element
 * @tparam N The number of elements
 *
 **********************************************************************
 */
template <typename T, int N>
class Buffer<T,N>
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
	 * Indexing operator. This will return a *const* reference to the
	 * element at the specified index
	 *
	 * @param[in] index The index to look up
	 *
	 * @return The element at \a index, or the first element on error
	 */
	inline const T& operator[](uint32 index) const
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
	 * Type conversion to a pointer. Allows passing a Buffer to stuff
	 * like std::memcpy()
	 *
	 * @return A pointer to the data
	 */
	inline operator T*()
	{
		return data;
	}

	/**
	 * Type conversion to a pointer. Allows passing a Buffer to stuff
	 * like std::memcpy()
	 *
	 * @return A const pointer to the data
	 */
	inline operator const T*() const
	{
		return data;
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
