/**
 *  \file   uci.h
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#ifndef CHESS_UCI_H_
#define CHESS_UCI_H_

#include <functional>
#include <map>
#include <string>

namespace chess {
/**
 * @brief Implements the UCI communication protocol
 */
class UciProtocol final {
public:
    /**
     * UCI command handler
     */
    using cmd_handler_t = std::function<bool(const std::string& args)>;

    UciProtocol();

    UciProtocol(const UciProtocol& protocol)            = default;
    UciProtocol(UciProtocol&& protocol)                 = default;
    UciProtocol& operator=(const UciProtocol& protocol) = default;
    UciProtocol& operator=(UciProtocol&& protocol)      = default;

    ~UciProtocol() = default;

    bool Poll();

    bool RegisterCommand(const std::string& name, cmd_handler_t handler);

private:
    /**
     * Mapping from UCI command name to handler
     */
    std::map<std::string, cmd_handler_t>
        commands_;
};

}  // namespace chess

#endif  // CHESS_UCI_H_
