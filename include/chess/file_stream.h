/**
 *  \file   file_stream.h
 *  \author Jason Fernandez
 *  \date   03/05/2023
 */

#ifndef CHESS_FILE_STREAM_H_
#define CHESS_FILE_STREAM_H_

#include <fstream>
#include <memory>
#include <string>

#include "chess/data_buffer.h"
#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief A stream channel which writes to a file on disk
 */
class FileStream final : public OutputStreamChannel {
public:
    explicit FileStream(const std::string& filename);

    FileStream(const FileStream& stream) = delete;
    FileStream(FileStream&& stream) = default;
    FileStream& operator=(const FileStream& stream) = delete;
    FileStream& operator=(FileStream&& stream) = default;

    ~FileStream() = default;

    void Flush() noexcept override;

    bool Good() const noexcept;

    void Write(const ConstDataBuffer& buffer) noexcept override;

private:
    /**
     * The name of the output file
     */
    std::string m_filename;

    /**
     * The output file stream
     */
    std::unique_ptr<std::ofstream>
        m_stream;
};

}  // namespace chess

#endif  // CHESS_FILE_STREAM_H_
