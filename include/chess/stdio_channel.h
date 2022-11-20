/**
 *  \file   stdio_channel.h
 *  \author Jason Fernandez
 *  \date   12/23/2021
 */

#ifndef CHESS_STDIO_CHANNEL_H_
#define CHESS_STDIO_CHANNEL_H_

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "chess/data_buffer.h"
#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief Reads from the standard input stream
 * 
 * @note Copy construction and assignment are disabled to prevent multiple
 *       instances of this class reading from standard input
 */
class StdinChannel final : public InputStreamChannel {
public:
    explicit StdinChannel(bool synced);

    StdinChannel(const StdinChannel& channel)            = delete;
    StdinChannel(StdinChannel&& channel)                 = default;
    StdinChannel& operator=(const StdinChannel& channel) = delete;
    StdinChannel& operator=(StdinChannel&& channel)      = default;

    ~StdinChannel();

    void Close() noexcept override;

    void Poll() noexcept override;

    bool IsClosed() const noexcept override;

private:
    void PollAsync();
    void PollSync();
    void ReadInput();

    /**
     * Atomic operations @{
     */
    bool Closed() const noexcept;
    void SetClosed() noexcept;
    bool MessagesAvailable() const noexcept;
    void SetMessagesAvailable(bool value) noexcept;
    /** @} */

    /** True if this channel has been closed */
    std::atomic<bool> closed_;

    /** True if reads are done synchronously */
    bool is_synced_;

    /** The message queue */
    std::deque<std::string> messages_;

    /** True when new messages are available */
    std::atomic<bool> messages_avail_;

    /** Thread that sits and waits on standard input */
    std::unique_ptr<std::thread>
        stdin_thread_;

    /** Avoids race conditions on the queue */
    std::mutex queue_mutex_;
};

}  // namespace chess

#endif  // CHESS_STDIO_CHANNEL_H_
