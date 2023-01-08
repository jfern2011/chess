/**
 *  \file   stream_channel.h
 *  \author Jason Fernandez
 *  \date   12/22/2021
 */

#ifndef CHESS_STREAM_CHANNEL_H_
#define CHESS_STREAM_CHANNEL_H_

#include <cstdio>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "chess/data_buffer.h"

namespace chess {
/**
 * @brief Sends commands to the engine
 */
class InputStreamChannel {
public:
    virtual ~InputStreamChannel() = default;

    /**
     * Close this channel. Future calls to Poll() will not produce
     * any messages
     */
    virtual void Close() noexcept = 0;

    /**
     * Poll the input stream. Messages are emitted via the emit_
     * callable, if assigned; otherwise, they are dropped
     */
    virtual void Poll() noexcept = 0;

    /**
     * Check if this channel is closed, i.e. futher calls to
     * Poll() will not produce messages
     *
     * @return True if this channel is closed
     */
    virtual bool IsClosed() const noexcept = 0;

    /**
     * Callable object through which stream messages are emitted
     */
    std::function<void(const ConstDataBuffer&)> emit_;
};

/**
 * @brief Sends engine output to clients
 */
class OutputStreamChannel {
public:
    OutputStreamChannel();

    virtual ~OutputStreamChannel() = default;

    /**
     * @brief Flush all buffered data to the output stream
     */
    virtual void Flush() noexcept = 0;

    /**
     * @brief Write to the output stream. Note that data may be buffered and
     *        not immediately written to the stream; @see Flush()
     *
     * @param buffer The data to write
     */
    virtual void Write(const ConstDataBuffer& buffer) noexcept = 0;

    OutputStreamChannel& operator<<(const std::string& str);

    void Resize(std::size_t size) noexcept;

    template <typename... Ts>
    void Write(const char* format, Ts&&... args) noexcept;

    void Write(const char* format) noexcept;

private:
    /**
     * The formatted output message to emit from this channel
     */
    std::vector<char> message_;
};

/**
 * @brief Write to the output stream. Note that data may be buffered and
 *        not immediately written to the stream; @see Flush()
 *
 * @param format std::printf-like format string
 */
template <typename... Ts>
void OutputStreamChannel::Write(const char* format, Ts&&... args) noexcept {
    const int len = std::snprintf(message_.data(), message_.size(),
                                  format, std::forward<Ts>(args)...);
    if (len > 0) {
        Write(ConstDataBuffer(
            message_.data(), static_cast<std::size_t>(len)));
    }
}

}  // namespace chess

#endif  // CHESS_STREAM_CHANNEL_H_
