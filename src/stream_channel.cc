/**
 *  \file   stream_channel.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief Default constructor
 */
OutputStreamChannel::OutputStreamChannel() : message_(1024) {    
}

/**
 * @brief Stream insertion operator
 *
 * @param str The data to send. A std::string because this is the only useful
 *            type for the UCI protocol
 *
 * @return *this
 */
OutputStreamChannel& OutputStreamChannel::operator<<(const std::string& str) {
    Write(ConstDataBuffer(str.c_str(), str.size()));
    return *this;
}

/**
 * @brief Set the maximum size of each message sent from this channel
 *
 * @param size The max size, in bytes
 */
void OutputStreamChannel::Resize(std::size_t size) noexcept {
    message_.resize(size);
}

/**
 * @brief Write to the output stream. Note that data may be buffered and
 *        not immediately written to the stream; @see Flush()
 *
 * @note This overload covers the case where there are not format arguments
 *
 * @param message The text to write
 */
void OutputStreamChannel::Write(const char* message) noexcept {
    Write("%s", message);
}

}  // namespace chess
