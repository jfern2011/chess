/**
 *  \file   stream_channel.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include "chess/stream_channel.h"

namespace chess {
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

}  // namespace chess
