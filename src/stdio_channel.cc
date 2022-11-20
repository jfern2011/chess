/**
 *  \file   stdio_channel.cc
 *  \author Jason Fernandez
 *  \date   12/23/2021
 */

#include "chess/stdio_channel.h"

#include <iostream>

#include "superstring/superstring.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param synced If true, calls to Poll() perform synchronous reads
 */
StdinChannel::StdinChannel(bool synced)
    : closed_(false),
      is_synced_(synced),
      messages_(),
      messages_avail_(false),
      stdin_thread_(),
      queue_mutex_() {
    if (!synced) {
        stdin_thread_ = std::make_unique<std::thread>(
            &StdinChannel::ReadInput, this);
    }
}

/**
 * @brief Destructor
 */
StdinChannel::~StdinChannel() {
    if (!is_synced_) stdin_thread_->join();
}

/**
 * @see InputStreamChannel::Close()
 */
void StdinChannel::Close() noexcept {
    SetClosed();
}

/**
 * Poll the channel for messages. Messages are emitted through the
 * callable data member
 */
void StdinChannel::Poll() noexcept {
    is_synced_ ? PollSync() : PollAsync();
}

/**
 * @see InputStreamChannel::IsClosed()
 */
bool StdinChannel::IsClosed() const noexcept {
    return Closed();
}

/**
 * Asynchronous read from standard input
 */
void StdinChannel::PollAsync() {
    if (MessagesAvailable()) {
        queue_mutex_.lock();
        while (!messages_.empty()) {
            const std::string& input = messages_.front();

            if (emit_) emit_(ConstDataBuffer(input.c_str(), input.size()));
            messages_.pop_front();
        }

        SetMessagesAvailable(false);
        queue_mutex_.unlock();
    }
}

/**
 * Synchronous read from standard input
 */
void StdinChannel::PollSync() {
    if (!IsClosed()) {
        std::string input;
        std::getline(std::cin, input);

        if (emit_) {
            emit_(ConstDataBuffer(input.c_str(),
                  input.size()));
        }
    }
}

/**
 * This method is performed by a thread whose only task is to sit and
 * wait for messages to come in from standard input. When a message
 * arrives it is copied to the input buffer. A mutex is used to avoid
 * simultaneous buffer access by the thread and its parent
 */
void StdinChannel::ReadInput() {
    std::string input;
    while (!IsClosed()) {
        std::getline(std::cin, input);
        queue_mutex_.lock();
    
        messages_.push_back(input);

        SetMessagesAvailable(true);
        queue_mutex_.unlock();

        // Check for the UCI "quit" command. We shouldn't be parsing commands
        // here since the purpose of a channel object is to simply forward
        // content to consumers. However, as of C++14 this is probably the
        // simplest way of exiting this thread since there's no portable way
        // for the parent process to terminate this thread cleanly. Also,
        // std::getline() is a blocking call, which means we cannot
        // periodically check a condition to break this loop. Polling the
        // standard input buffer by using std::sync_with_stdio(false) combined
        // with std::cin.rdbuf()->in_avail() seems to work, but the C++
        // stream library doesn't guarantee the desired behavior

        const std::string cmd = jfern::superstring(input).to_lower().trim();
        if (cmd.find("quit") != std::string::npos) {
            SetClosed();
        }
    }
}

/**
 * @brief Check if this channel has been closed
 *
 * @return True if closed
 */
bool StdinChannel::Closed() const noexcept {
    return closed_.load(std::memory_order_relaxed);
}

/**
 * @brief Close the input stream
 */
void StdinChannel::SetClosed() noexcept {
    closed_.store(true, std::memory_order_relaxed);
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
 * @brief Set the message availability flag
 * 
 * @param value True to indicate standard input messages are ready
 */
void StdinChannel::SetMessagesAvailable(bool value) noexcept {
    messages_avail_.store(value, std::memory_order_relaxed);
}

}  // namespace chess
