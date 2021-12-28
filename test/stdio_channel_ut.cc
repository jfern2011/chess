/**
 *  \file   stdio_channel_ut.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include <chrono>
#include <sstream>
#include <thread>

#include "gtest/gtest.h"

#include "chess/stdio_channel.h"

namespace {
TEST(stdin_channel, poll) {
    std::string test_input("Hello, world!\n");

    int emit_count = 0;
    std::cin.sync_with_stdio(false);

    std::istringstream input(test_input);
    std::cin.rdbuf(input.rdbuf());

    // Create the channel AFTER assigning the stream buffer
    chess::StdinChannel channel;

    channel.emit_ =
        [&] (const chess::ConstDataBuffer& buf) {
        EXPECT_EQ(test_input, std::string(buf.data(), buf.size()) + "\n");
        emit_count++;
    };

    for (int attempts = 1; attempts <= 5; attempts++) {
        channel.Poll();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (emit_count == 1) break;
    }

    EXPECT_EQ(emit_count, 1);
}

}  // namespace
