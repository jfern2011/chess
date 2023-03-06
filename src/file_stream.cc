/**
 *  \file   file_stream.cc
 *  \author Jason Fernandez
 *  \date   03/05/2023
 */

#include "chess/file_stream.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param filename The name of the output file to open
 */
FileStream::FileStream(const std::string& filename)
    : m_filename(filename),
      m_stream(new std::ofstream(filename, std::ios::out |
                                           std::ios::trunc)) {
}

/**
 * @see OutputStreamChannel::Flush()
 */
void FileStream::Flush() noexcept {
    m_stream->flush();
}

/**
 * @brief Check if the stream is OK
 *
 * @see std::ostream::good()
 * @see std::ofstream::is_open()
 *
 * @return True if the stream is OK
 */
bool FileStream::Good() const noexcept {
    return m_stream->is_open() && m_stream->good();
}

/**
 * @see OutputStreamChannel::Write()
 */
void FileStream::Write(const ConstDataBuffer& buffer) noexcept {
    m_stream->write(buffer.data(), buffer.size());
}

}  // namespace chess
