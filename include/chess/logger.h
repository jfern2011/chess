/**
 *  \file   logger.h
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#ifndef CHESS_LOG_H_
#define CHESS_LOG_H_

#include <ctime>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief Logs messages from an individual engine component
 */
class Logger final {
public:
    Logger(const std::string& name,
           std::shared_ptr<OutputStreamChannel> channel);

    Logger(const Logger& logger)            = default;
    Logger(Logger&& logger)                 = default;
    Logger& operator=(const Logger& logger) = default;
    Logger& operator=(Logger&& logger)      = default;

    ~Logger() = default;

    std::string Name() const noexcept;

    template <typename... Ts>
    void Write(const char* format, Ts&&... args) noexcept;

    void Write(const char* message) noexcept;

private:
    /**
     * The channel to emit messages through
     */
    std::shared_ptr<OutputStreamChannel> channel_;

    /**
     * The name of this log source
     */
    std::string name_;

    /**
     * Buffer sized to contain message prefix
     */
    std::string prefix_buffer_;
};

/**
 * @brief Write a message to the log
 *
 * @param format std::printf-like format string
 */
template <typename... Ts>
void Logger::Write(const char* format, Ts&&... args) noexcept {
    std::time_t time = std::time({});
    std::strftime(prefix_buffer_.data(), prefix_buffer_.size(),
                  "%F %T GMT", std::gmtime(&time));

    channel_->Write("%s (%s): ", prefix_buffer_.c_str(), name_.c_str());
    channel_->Write(format, std::forward<Ts>(args)...);
    channel_->Flush();
}

}  // namespace chess

#endif  // CHESS_LOG_H_
