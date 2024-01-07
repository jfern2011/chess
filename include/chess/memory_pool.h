/**
 *  \file   memory_pool.h
 *  \author Jason Fernandez
 *  \date   04/16/2023
 */

#ifndef CHESS_MEMORY_POOL_H_
#define CHESS_MEMORY_POOL_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "chess/logger.h"

namespace chess {
/**
 * @brief A simple memory pool from which individual objects of a particular
 *        type are allocated
 *
 * @tparam T The type allocated on each call to Allocate()
 */
template <typename T>
class MemoryPool final {
public:
    static_assert(sizeof(std::uint8_t*) <= sizeof(T));

    MemoryPool(std::size_t size, std::shared_ptr<Logger> logger);

    MemoryPool(const MemoryPool& pool) = delete;
    MemoryPool(MemoryPool&& pool) = delete;
    MemoryPool& operator=(const MemoryPool& pool) = delete;
    MemoryPool& operator=(MemoryPool& pool) = delete;

    ~MemoryPool();

    T* Allocate();

    void Free();

    bool Free(T* address);

    bool Full() const;

    std::size_t InUse() const;

    std::size_t Size() const;

private:
    void Initialize();

    /**
     * Underlying storage for the memory pool
     */
    std::uint8_t* data_;

    /**
     * The head of the memory pool
     */
    std::uint8_t* head_;

    /**
     * The number of bytes currently in use
     */
    std::size_t in_use_;

    /**
     * The total pool size, in bytes
     */
    std::size_t size_;
};

/**
 * @brief Constructor
 *
 * @param size   The total size of the memory pool in bytes
 * @param logger For logging diagnostics
 */
template <typename T>
MemoryPool<T>::MemoryPool(std::size_t size,
                          std::shared_ptr<Logger> logger)
    : data_(nullptr), head_(nullptr), in_use_(0u), size_(0u) {
    const std::size_t n_elements = size / sizeof(T);

    if (n_elements > 0) {
        size_ = n_elements * sizeof(T);

        data_ = new std::uint8_t[size_];
        head_ = data_;

        Initialize();
    }

    logger->Write("Allocated %zu elements in %zu bytes (%zu requested)\n",
                  n_elements, size_, size);
}

/**
 * @brief Destructor
 */
template <typename T>
MemoryPool<T>::~MemoryPool() {
    if (data_) delete[] data_;
}

/**
 * @brief Allocate an element
 *
 * @tparam T The data type of allocated/deallocated elements
 *
 * @return Address of the new entry, or nullptr if out of memory
 */
template <typename T>
T* MemoryPool<T>::Allocate() {
    if (Full()) return nullptr;

    T* entry = reinterpret_cast<T*>(head_);

    auto next = reinterpret_cast<std::uint8_t**>(head_);

    head_ = *next;

    in_use_ += sizeof(T);

    return entry;
}

/**
 * @brief Free all memory
 *
 * @tparam T The data type of allocated/deallocated elements
 */
template <typename T>
void MemoryPool<T>::Free() {
    head_ = data_;
    in_use_ = 0;

    Initialize();
}

/**
 * @brief Free an element
 *
 * @note Double-free causes undefined behavior
 *
 * @tparam T The data type of allocated/deallocated elements
 *
 * @param address Address of the element to free
 *
 * @return True on success
 */
template <typename T>
bool MemoryPool<T>::Free(T* address) {
    const std::uint8_t* end = data_ + size_;

    const auto freed = reinterpret_cast<std::uint8_t*>(address);

    if (freed < data_ || freed >= end) {
        return false;
    }

    std::uint8_t* prev_head = head_;

    head_ = reinterpret_cast<std::uint8_t*>(address);

    *reinterpret_cast<std::uint8_t**>(head_) = prev_head;

    in_use_ -= sizeof(T);

    return true;
}

/**
 * @brief Check if the memory pool is used up
 *
 * @tparam T The data type of allocated/deallocated elements
 *
 * @return True if no more memory may be allocated
 */
template <typename T>
bool MemoryPool<T>::Full() const {
    return (in_use_ + sizeof(T)) > size_;
}

/**
 * @brief Check the number of bytes allocated so far
 *
 * @tparam T The data type of allocated/deallocated elements
 *
 * @return The amount of memory in use
 */
template <typename T>
std::size_t MemoryPool<T>::InUse() const {
    return in_use_;
}

/**
 * @brief Get the total size of the pool, in bytes
 *
 * @tparam T The data type of allocated/deallocated elements
 *
 * @return The size of the memory pool in bytes
 */
template <typename T>
std::size_t MemoryPool<T>::Size() const {
    return size_;
}

/**
 * @brief Initialize internal data structures
 */
template <typename T>
void MemoryPool<T>::Initialize() {
    const std::size_t n_elements = size_ / sizeof(T);

    for (std::size_t i = 0; i < n_elements; i++) {
        std::uint8_t* entry = data_ + i * sizeof(T);

        auto current = reinterpret_cast<std::uint8_t**>(entry);

        std::uint8_t* next =
            (i == n_elements-1) ? nullptr : (entry +sizeof(T));

        *current = next;
    }
}

}  // namespace chess

#endif  // CHESS_MEMORY_POOL_H_
