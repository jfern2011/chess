/**
 *  \file   data_buffer.h
 *  \author Jason Fernandez
 *  \date   12/22/2021
 */

#ifndef CHESS_DATA_BUFFER_H_
#define CHESS_DATA_BUFFER_H_

#include <cstddef>
#include <cstdint>

namespace chess {
/**
 * @brief Convenience wrapper to an array/size pair
 */
class DataBuffer final {
public:
    DataBuffer();
    DataBuffer(std::uint8_t* buf, std::size_t size);

    DataBuffer(const DataBuffer& buffer)            = default;
    DataBuffer(DataBuffer&& buffer)                 = default;
    DataBuffer& operator=(const DataBuffer& buffer) = default;
    DataBuffer& operator=(DataBuffer&& buffer)      = default;

    ~DataBuffer() = default;

    const std::uint8_t* data() const noexcept;

    template <typename T> bool push_back(T&& element) noexcept;

    std::size_t size() const noexcept;

private:
    /** Pointer to the buffer itself */
    std::uint8_t* m_buf;

    /** The size of this data buffer */
    std::size_t m_size;
};

/**
 * @brief Convenience wrapper to an array/size pair
 */
class ConstDataBuffer final {
public:
    ConstDataBuffer();
    ConstDataBuffer(const char* buf, std::size_t size);

    ConstDataBuffer(const ConstDataBuffer& buffer)            = default;
    ConstDataBuffer(ConstDataBuffer&& buffer)                 = default;
    ConstDataBuffer& operator=(const ConstDataBuffer& buffer) = default;
    ConstDataBuffer& operator=(ConstDataBuffer&& buffer)      = default;

    ~ConstDataBuffer() = default;

    const char* data() const noexcept;
    std::size_t size() const noexcept;

private:
    /** Pointer to the buffer itself */
    const char* m_buf;

    /** The size of this data buffer */
    std::size_t m_size;
};

}  // namespace chess

#endif  // CHESS_DATA_BUFFER_H_
