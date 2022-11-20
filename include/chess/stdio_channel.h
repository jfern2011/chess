/**
 *  \file   stdio_channel.h
 *  \author Jason Fernandez
 *  \date   12/23/2021
 */

#ifndef CHESS_STDIO_CHANNEL_H_
#define CHESS_STDIO_CHANNEL_H_

#include <atomic>
#include <deque>
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
    StdinChannel();

    StdinChannel(const StdinChannel& channel)            = delete;
    StdinChannel(StdinChannel&& channel)                 = default;
    StdinChannel& operator=(const StdinChannel& channel) = delete;
    StdinChannel& operator=(StdinChannel&& channel)      = default;

    ~StdinChannel();

    void Poll() noexcept override;

    bool IsClosed() const noexcept override;

private:
    void ReadInput();

    /**
     * Atomic operations @{
     */
    bool MessagesAvailable() const noexcept;
    void SetMessagesAvailable(bool value) noexcept;
    /** @} */

    /** True if this channel has been closed */
    bool closed_;

    /** The message queue */
    std::deque<std::string> messages_;

    /** True when new messages are available */
    std::atomic<bool> messages_avail_;

    /** Thread that sits and waits on standard input */
    std::thread stdin_thread_;

    /** Avoids race conditions on the queue */
    std::mutex queue_mutex_;
};

}  // namespace chess

#endif  // CHESS_STDIO_CHANNEL_H_
