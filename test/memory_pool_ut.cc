/**
 *  \file   memory_pool_ut.cc
 *  \author Jason Fernandez
 *  \date   04/22/2023
 */

#include <cstdint>
#include <memory>

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

}  // namespace
