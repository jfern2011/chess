/**
 *  \file   command_dispatcher.h
 *  \author Jason Fernandez
 *  \date   11/12/2022
 */

#ifndef COMMAND_DISPATCHER_H_
#define COMMAND_DISPATCHER_H_

#include <functional>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "chess/data_buffer.h"

namespace chess {
/**
 * @brief Forwards commands to registered handlers
 */
class CommandDispatcher final {
public:
    /**
     * Command handler
     */
    using cmd_handler_t =
        std::function<bool(const std::vector<std::string>& args)>;

    CommandDispatcher() = default;

    CommandDispatcher(const CommandDispatcher& dispatcher)            = default;
    CommandDispatcher(CommandDispatcher&& dispatcher)                 = default;
    CommandDispatcher& operator=(const CommandDispatcher& dispatcher) = default;
    CommandDispatcher& operator=(CommandDispatcher&& dispatcher)      = default;

    ~CommandDispatcher() = default;

    void HandleCommand(const ConstDataBuffer& buf);

    bool RegisterCommand(const std::string& name, cmd_handler_t handler);

    /**
     * Callback invoked when an unregistered command is received
     */
    std::function<void(const ConstDataBuffer&)>
        error_callback_;

private:
    /**
     * Mapping from command name to command handler
     */
    std::map<std::string, cmd_handler_t>
        commands_;
};

}  // namespace chess

#endif  // COMMAND_DISPATCHER_H_
