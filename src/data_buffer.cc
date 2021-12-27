/**
 *  \file   data_buffer.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include "chess/data_buffer.h"

namespace chess {
/**
 * @brief Default constructor
 */
ConstDataBuffer::ConstDataBuffer() : ConstDataBuffer(nullptr, 0) {
}

/**
 * @brief Constructor
 * 
 * @param buf  The buffer to hold onto
 * @param size The size of \a buf
 */
ConstDataBuffer::ConstDataBuffer(const char* buf, std::size_t size)
    : buf_(buf), size_(size) {
}

/**
 * @brief Get the working data buffer
 *
 * @return The data buffer
 */
const char* ConstDataBuffer::data() const noexcept {
    return buf_;
}

/**
 * @brief Get the size of the working buffer
 * 
 * @return The buffer size, in bytes
 */
std::size_t ConstDataBuffer::size() const noexcept {
    return size_;
}

}  // namespace chess
