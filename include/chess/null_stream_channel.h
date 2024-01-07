/**
 *  \file   null_stream_channel.h
 *  \author Jason Fernandez
 *  \date   01/07/2024
 */

#ifndef CHESS_NULL_STREAM_CHANNEL_H_
#define CHESS_NULL_STREAM_CHANNEL_H_

#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief An OutputStreamChannel that writes to nothing
 */
class NullOstreamChannel final : public OutputStreamChannel {
public:
    NullOstreamChannel() = default;

    void Flush() noexcept override;

    void Write(const ConstDataBuffer& buffer) noexcept;
};

}  // namespace chess

#endif  // CHESS_NULL_STREAM_CHANNEL_H_
