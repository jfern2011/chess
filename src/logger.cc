/**
 *  \file   logger.cc
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#include "chess/logger.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param name    The name of this log source
 * @param channel The channel through which to emit log messages
 */
Logger::Logger(const std::string& name,
               std::shared_ptr<OutputStreamChannel> channel)
    : channel_(channel),
      name_(name),
      prefix_buffer_(name.size() + 200, '\0') {
}

/**
 * Get the name of this log source
 *
 * @return The source name
 */
std::string Logger::Name() const noexcept {
    return name_;
}

/**
 * @brief Write a message to the log
 *
 * @note This overload covers the case where there are no format arguments
 *
 * @param message The text to write
 */
void Logger::Write(const char* message) noexcept {
    Write("%s", message);
}

}  // namespace chess
