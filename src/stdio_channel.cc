/**
 *  \file   stdio_channel.cc
 *  \author Jason Fernandez
 *  \date   12/23/2021
 */

#include "chess/stdio_channel.h"

#include <chrono>
#include <iostream>

namespace chess {
/**
 * @brief Constructor
 */
StdinChannel::StdinChannel()
    : messages_(),
      messages_avail_(false),
      stdin_thread_(&StdinChannel::ReadInput, this),
      queue_mutex_(),
      quit_(false) {
}

/**
 * @brief Destructor
 */
StdinChannel::~StdinChannel() {
    SetExitNow(true);
    stdin_thread_.join();
}

/**
 * Poll the channel for messages. Messages are emitted through the
 * callable data member
 */
void StdinChannel::Poll() noexcept {
    if (MessagesAvailable()) {
        queue_mutex_.lock();
        while (!messages_.empty()) {
            const std::string input = messages_.front();

            if (emit_) emit_(ConstDataBuffer(input.c_str(), input.size()));
            messages_.pop_front();
        }

        SetMessagesAvailable(false);
        queue_mutex_.unlock();
    }
}

/**
 * This method is performed by a thread whose only task is to sit and
 * wait for messages to come in from standard input. When a message
 * arrives it is copied to the input buffer. A mutex is used to avoid
 * simultaneous buffer access by the thread and its parent
 *
 * @todo Consider simply blocking on getline() even though tear
 *       down will look ugly
 */
void StdinChannel::ReadInput() {
    std::ios::sync_with_stdio(false);

    std::string input;
    std::streambuf* sb = std::cin.rdbuf();

    while (!ExitNow()) {
        if (sb->in_avail() > 0) {
            std::getline(std::cin, input);
            queue_mutex_.lock();

            messages_.push_back(input);

            SetMessagesAvailable(true);
            queue_mutex_.unlock();
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(200));
    }
}

/**
 * Check if the thread waiting on standard input should exit
 * 
 * @return True to exit
 */
bool StdinChannel::ExitNow() const noexcept {
    return quit_.load(std::memory_order_relaxed);
}

/**
 * @brief Check if messages are available from standard input
 * 
 * @return True if messages are available, false otherwise
 */
bool StdinChannel::MessagesAvailable() const noexcept {
    return messages_avail_.load(std::memory_order_relaxed);
}

/**
 * @brief Set the ExitNow() flag
 * 
 * @param value True to have the standard input thread exit
 */
void StdinChannel::SetExitNow(bool value) noexcept {
    quit_.store(value, std::memory_order_relaxed);
}

/**
 * @brief Set the message availability flag
 * 
 * @param value True to indicate standard input messages are ready
 */
void StdinChannel::SetMessagesAvailable(bool value) noexcept {
    messages_avail_.store(value, std::memory_order_relaxed);
}

}  // namespace chess
