/**
 *  \file   uci.cc
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#include "chess/uci.h"

namespace chess {
/**
 * @brief Constructor
 */
UciProtocol::UciProtocol() : m_commands() {
}

/**
 * @brief Register a new UCI command
 * 
 * @param name    The command name, e.g. "isready"
 * @param handler The command handler
 *
 * @return True on success. On failure, this method is a no-op
 */
bool UciProtocol::RegisterCommand(
    const std::string& name, cmd_handler_t handler) {
    if (name.empty() || !handler) return false;

    m_commands[name] = handler;
    return true;
}

}  // namespace chess
