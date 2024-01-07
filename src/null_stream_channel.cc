/**
 *  \file   null_stream_channel.cc
 *  \author Jason Fernandez
 *  \date   01/07/2024
 */

#include "chess/null_stream_channel.h"

namespace chess {
/**
 * @see OutputStreamChannel::Flush()
 *
 * @note This is a no-op
 */
void NullOstreamChannel::Flush() noexcept {
}

/**
 * @see OutputStreamChannel::Write()
 *
 * @note This is a no-op
 */
void NullOstreamChannel::Write(const ConstDataBuffer& /* buffer */) noexcept {
}

}  // namespace chess
