/**
 *  \file   stream_channel_ut.cc
 *  \author Jason Fernandez
 *  \date   01/07/2023
 */

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "chess/stream_channel.h"

namespace {
class MockOutputStreamChannel final : public chess::OutputStreamChannel {
public:
    MOCK_METHOD(void, Flush, (), (noexcept, override));
    MOCK_METHOD(void, Write, (const chess::ConstDataBuffer& buffer),
                (noexcept, override));
};

TEST(outputstreamchannel, stream_extraction) {
    MockOutputStreamChannel channel;
    channel.Resize(1024);

    const std::string expected("hello");

    auto check_message = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual == expected;
    };

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);
    
    channel << expected;
}

TEST(outputstreamchannel, format_specifiers) {
    MockOutputStreamChannel channel;
    channel.Resize(1024);

    const std::string expected("Hey Jason you're #1");

    auto check_message = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual == expected;
    };

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    chess::OutputStreamChannel& temp = channel;

    temp.Write("Hey %s you're #%d", "Jason", 1);
}

TEST(outputstreamchannel, no_format_specifiers) {
    MockOutputStreamChannel channel;
    channel.Resize(1024);

    const std::string expected("hello");

    auto check_message = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual == expected;
    };

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    chess::OutputStreamChannel& temp = channel;

    temp.Write(expected.c_str());
}

TEST(outputstreamchannel, resize) {
    MockOutputStreamChannel channel;
    channel.Resize(1);

    std::string expected("h");

    auto check_message = [&](const chess::ConstDataBuffer& buffer) -> bool {
        const std::string actual(buffer.data(), buffer.size());
        return actual == expected;
    };

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    chess::OutputStreamChannel& temp = channel;

    temp.Write("hello");

    // Resize the channel to include the entire message

    channel.Resize(5);

    expected = "hello";

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    temp.Write("hello");

    // Case with more complex formatting

    expected = "Hey J";

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    temp.Write("Hey %s I have %d apples.", "Jason", 2);

    channel.Resize(16);
    expected = "Hey Jason I have";

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    temp.Write("Hey %s I have %d apples.", "Jason", 2);

    channel.Resize(100);
    expected = "Hey Jason I have 2 apples.";

    EXPECT_CALL(channel, Write(::testing::Truly(check_message)))
        .Times(1);

    temp.Write("Hey %s I have %d apples.", "Jason", 2);
}

TEST(outputstreamchannel, suppress) {
    MockOutputStreamChannel channel;
    channel.Resize(0);

    EXPECT_CALL(channel, Write(::testing::_)).Times(0);

    chess::OutputStreamChannel& temp = channel;

    temp.Write("hello");
}

}  // namespace
