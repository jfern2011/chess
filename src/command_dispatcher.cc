/**
 *  \file   command_dispatcher.cc
 *  \author Jason Fernandez
 *  \date   11/13/2022
 */

#include "chess/command_dispatcher.h"

#include "superstring/superstring.h"

namespace chess {
/**
 * @brief Forward a command to downstream handlers
 *
 * @param buf The raw command
 */
void CommandDispatcher::HandleCommand(const ConstDataBuffer& buf) {
    const jfern::superstring sstring(std::string(buf.data(), buf.size()));

    const std::vector<std::string> tokens = sstring.split();
    if (!tokens.empty()) {
        const std::string& command = tokens[0];
        auto iter = commands_.find(command);
        if (iter != commands_.end()) {
            const std::vector<std::string>
                args(tokens.begin()+1, tokens.end());
            iter->second(args);
        } else if (error_callback_) {
            error_callback_(buf);
        }
    }
}

/**
 * @brief Register a new command
 * 
 * @param name    The name of this command
 * @param handler The command handler
 *
 * @return True on success. On failure, this method is a no-op
 */
bool CommandDispatcher::RegisterCommand(
    const std::string& name, cmd_handler_t handler) {
    if (name.empty() || !handler) return false;

    commands_[name] = handler;
    return true;
}

}  // namespace chess
