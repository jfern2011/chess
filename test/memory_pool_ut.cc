/**
 *  \file   memory_pool_ut.cc
 *  \author Jason Fernandez
 *  \date   04/22/2023
 */

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "chess/logger.h"
#include "chess/memory_pool.h"

namespace {
class NullStreamChannel final : public chess::OutputStreamChannel {
public:
    void Flush() noexcept override {}
    void Write(const chess::ConstDataBuffer& ) noexcept override {}
};

struct MemoryChunk {
    std::uint8_t buf[16];
};

/**
 * @brief Check for overlapping memory
 *
 * @tparam T The data type returned on each allocation
 *
 * @param chunks A (possibly non-contiguous) collection of allocated memory
 *               chunks expected NOT to overlap
 *
 * @return True if checks pass (no overlaps detected)
 */
template <typename T>
bool CheckMemory(const std::vector<T*>& chunks) {
    auto sorted(chunks);

    std::sort(sorted.begin(), sorted.end());

    for (std::size_t i = 1; i < sorted.size(); i++) {
        const auto prev = reinterpret_cast<std::uint8_t*>(sorted[i-1]);
        const auto curr = reinterpret_cast<std::uint8_t*>(sorted[i]);

        if (prev == curr || prev + sizeof(T) != curr) {
            return false;
        }
    }

    // No overlapping regions detected
    return true;
}

TEST(MemoryPool, zero_sized) {
    auto channel = std::make_shared<NullStreamChannel>();
    channel->Resize(1024);

    auto logger = std::make_shared<chess::Logger>("Test", channel);

    for (std::size_t size = 0; size < sizeof(MemoryChunk); size++) {
        chess::MemoryPool<MemoryChunk> pool(size, logger);

        ASSERT_EQ(pool.Allocate(), nullptr);
        ASSERT_TRUE(pool.Full());
        ASSERT_EQ(pool.InUse(), 0u);
        ASSERT_EQ(pool.Size(), 0u);
    }
}

TEST(MemoryPool, single_sized) {
    auto channel = std::make_shared<NullStreamChannel>();
    channel->Resize(1024);

    auto logger = std::make_shared<chess::Logger>("Test", channel);

    constexpr std::size_t element_size = sizeof(MemoryChunk);
    
    for (std::size_t size = element_size; size < 2 * element_size; size++) {
        chess::MemoryPool<MemoryChunk> pool(size, logger);

        ASSERT_FALSE(pool.Full());
        ASSERT_EQ(pool.InUse(), 0u);
        ASSERT_EQ(pool.Size(), element_size);

        MemoryChunk* chunk = pool.Allocate();

        ASSERT_EQ(pool.Allocate(), nullptr);
        ASSERT_TRUE(pool.Full());
        ASSERT_EQ(pool.InUse(), element_size);
        ASSERT_EQ(pool.Size(), element_size);

        ASSERT_TRUE(pool.Free(chunk));

        ASSERT_EQ(pool.Allocate(), chunk);
        ASSERT_TRUE(pool.Free(chunk));

        ASSERT_FALSE(pool.Full());
        ASSERT_EQ(pool.InUse(), 0u);
        ASSERT_EQ(pool.Size(), element_size);
    }
}

TEST(MemoryPool, stress_test) {
    auto channel = std::make_shared<NullStreamChannel>();
    channel->Resize(1024);

    auto logger = std::make_shared<chess::Logger>("Test", channel);

    constexpr std::size_t element_size = sizeof(MemoryChunk);
    constexpr std::size_t pool_size = 1000000;  // 1 MB
    constexpr std::size_t num_elements = pool_size / element_size;
    constexpr std::size_t expected_size = num_elements * element_size;

    chess::MemoryPool<MemoryChunk> pool(pool_size, logger);

    EXPECT_FALSE(pool.Full());
    EXPECT_EQ(pool.InUse(), 0u);
    EXPECT_EQ(pool.Size(), expected_size);

    // Repeatedly allocate until all memory is used up

    std::vector<MemoryChunk*> allocated(num_elements);

    for (std::size_t i = 0; i < allocated.size(); i++) {
        allocated[i] = pool.Allocate();
        ASSERT_NE(allocated[i], nullptr);

        if (i > 0) {
            const auto prev =
                reinterpret_cast<std::uint8_t*>(allocated[i-1]);
            const auto current =
                reinterpret_cast<std::uint8_t*>(allocated[i-0]);

            ASSERT_EQ(current-prev, element_size);
        }
    }

    EXPECT_TRUE(pool.Full());
    EXPECT_EQ(pool.InUse(), expected_size);
    EXPECT_EQ(pool.Allocate(), nullptr);

    // Repeatedly free and re-allocate each element

    for (std::size_t i = 0; i < allocated.size(); i++) {
        ASSERT_TRUE(pool.Free(allocated[i]));
        ASSERT_EQ(pool.Allocate(), allocated[i]);
    }

    EXPECT_TRUE(pool.Full());
    EXPECT_EQ(pool.InUse(), expected_size);
    EXPECT_EQ(pool.Allocate(), nullptr);

    // Repeatedly free and re-allocate each element, going in reverse

    for (std::size_t i = allocated.size(); i > 0; i--) {
        ASSERT_TRUE(pool.Free(allocated[i-1]));
        ASSERT_EQ(pool.Allocate(), allocated[i-1]);
    }

    EXPECT_TRUE(pool.Full());
    EXPECT_EQ(pool.InUse(), expected_size);
    EXPECT_EQ(pool.Allocate(), nullptr);

    // Free all elements

    for (std::size_t i = 0; i < allocated.size(); i++) {
        ASSERT_TRUE(pool.Free(allocated[i]));
    }

    EXPECT_FALSE(pool.Full());
    EXPECT_EQ(pool.InUse(), 0u);
    EXPECT_EQ(pool.Size(), expected_size);

    // Repeatedly allocate, then free each Nth element

    const auto max_n = std::min<std::size_t>(30u, num_elements);

    for (std::size_t n = 1; n <= max_n; n++) {
        std::vector<bool> freed(num_elements, false);

        for (std::size_t i = 0; i < allocated.size(); i++) {
            allocated[i] = pool.Allocate();
            ASSERT_NE(allocated[i], nullptr);
        }

        ASSERT_TRUE(CheckMemory(allocated));

        ASSERT_TRUE(pool.Full());
        ASSERT_EQ(pool.InUse(), expected_size);

        for (std::size_t i = n; i < allocated.size(); i += n) {
            ASSERT_TRUE(pool.Free(allocated[i]));
            freed[i] = true;
        }

        const std::size_t num_frees =
            std::count(freed.begin(), freed.end(), true);

        const std::size_t expected_usage =
            expected_size - (element_size * num_frees);

        ASSERT_EQ(pool.InUse(), expected_usage);
        ASSERT_FALSE(pool.Full());

        // Free all remaining elements

        for (std::size_t i = 0; i < num_elements; i++) {
            if (!freed[i]) ASSERT_TRUE(pool.Free(allocated[i]));
        }

        ASSERT_EQ(pool.InUse(), 0u);
        ASSERT_FALSE(pool.Full());
    }
}

TEST(MemoryPool, free_all) {
    auto channel = std::make_shared<NullStreamChannel>();
    channel->Resize(1024);

    auto logger = std::make_shared<chess::Logger>("Test", channel);

    constexpr std::size_t n_elements = 10;
    constexpr std::size_t bytes = n_elements * sizeof(MemoryChunk);

    chess::MemoryPool<MemoryChunk> pool(bytes, logger);

    ASSERT_GE(pool.Size(), bytes);

    const std::uint8_t* prev_chunk = nullptr;
    const MemoryChunk* init_chunk = nullptr;

    for (std::size_t i = 0; i < n_elements; i++) {
        ASSERT_EQ(pool.InUse(), i * sizeof(MemoryChunk));

        const auto chunk = reinterpret_cast<std::uint8_t*>(pool.Allocate());
        if (prev_chunk != nullptr) {
            ASSERT_EQ(prev_chunk + sizeof(MemoryChunk), chunk);
        } else {
            init_chunk = reinterpret_cast<MemoryChunk*>(chunk);
        }

        prev_chunk = chunk;
    }

    ASSERT_TRUE(pool.Full());

    pool.Free();

    ASSERT_EQ(pool.InUse(), 0u);
    ASSERT_EQ(pool.Allocate(), init_chunk);
}

}  // namespace
