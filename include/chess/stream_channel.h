/**
 *  \file   stream_channel.h
 *  \author Jason Fernandez
 *  \date   12/22/2021
 */

#ifndef CHESS_STREAM_CHANNEL_H_
#define CHESS_STREAM_CHANNEL_H_

#include <functional>
#include <string>

#include "chess/data_buffer.h"

namespace chess {
/**
 * @brief Sends commands to the engine
 */
class InputStreamChannel {
public:
    virtual ~InputStreamChannel() = default;

    /**
     * Poll the input stream. Messages are emitted via the emit_
     * callable, if assigned; otherwise, they are dropped
     */
    virtual void Poll() noexcept = 0;

    /**
     * Check if this channel is closed, i.e. futher calls to
     * Poll() will not produce data
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
};

}  // namespace chess

#endif  // CHESS_STREAM_CHANNEL_H_
