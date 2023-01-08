/**
 *  \file   logger_ut.cc
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "chess/logger.h"

namespace {
class MockOutputStreamChannel final : public chess::OutputStreamChannel {
public:
    MOCK_METHOD(void, Flush, (), (noexcept, override));
    MOCK_METHOD(void, Write, (const chess::ConstDataBuffer& buffer),
                (noexcept, override));
};

TEST(logger, all) {
    auto channel = std::make_shared<MockOutputStreamChannel>();
    channel->Resize(1024);

    const std::string expected("hello");
    const std::string name("Test");

    auto check_message1 = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual.find(name) != std::string::npos;
    };

    auto check_message2 = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual.find(expected) != std::string::npos;
    };

    EXPECT_CALL(*channel, Write(::testing::Truly(check_message1)))
        .Times(1);
    EXPECT_CALL(*channel, Write(::testing::Truly(check_message2)))
        .Times(1);
    EXPECT_CALL(*channel, Flush())
        .Times(1);
    
    chess::Logger logger(name, channel);
    logger.Write(expected.c_str());
}

}  // namespace
