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
TEST(stdin_channel, PollAsync) {
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
    chess::StdinChannel channel(false);

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

    // Verify the channel was closed due to the "quit" command

    ASSERT_TRUE(channel.IsClosed());
}

TEST(stdin_channel, PollSync) {
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
    chess::StdinChannel channel(true);

    channel.emit_ =
        [&] (const chess::ConstDataBuffer& buf) {
        ASSERT_GT(commands.size(), emit_count);

        EXPECT_EQ(commands[emit_count++],
                  std::string(buf.data(), buf.size()) + "\n");
    };

    for (std::size_t i = 0; i < commands.size(); i++) {
        channel.Poll();
    }

    EXPECT_EQ(emit_count, commands.size());

    // Close the channel. Future polls should come back empty

    channel.Close();
    ASSERT_TRUE(channel.IsClosed());

    channel.Poll();    // should no longer block for messages

    EXPECT_EQ(emit_count, commands.size());
}

}  // namespace
