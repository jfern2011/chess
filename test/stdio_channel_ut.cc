/**
 *  \file   stdio_channel_ut.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include <chrono>
#include <sstream>
#include <thread>

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "chess/stdio_channel.h"
#include "superstring/superstring.h"

namespace {
TEST(stdin_channel, poll) {
    std::vector<std::string> commands;
    commands.push_back("Hello, world!\n");
    commands.push_back("Quit\n");

    const std::string test_input =
        jfern::superstring::build("", commands.begin(), commands.end());

    // Number of messages emitted by the channel
    unsigned int emit_count = 0;

    std::istringstream input(test_input);
    std::cin.rdbuf(input.rdbuf());

    // Create the channel AFTER assigning the stream buffer
    chess::StdinChannel channel;

    channel.emit_ =
        [&] (const chess::ConstDataBuffer& buf) {
        ASSERT_GT(commands.size(), emit_count);

        EXPECT_EQ(commands[emit_count++],
                  std::string(buf.data(), buf.size()) + "\n");
    };

    for (int attempts = 1; attempts <= 5; attempts++) {
        channel.Poll();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (emit_count == commands.size()) break;
    }

    EXPECT_EQ(emit_count, commands.size());
}

}  // namespace
