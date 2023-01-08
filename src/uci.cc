/**
 *  \file   uci.cc
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#include "chess/uci.h"

namespace chess {
/**
 * @brief Forwards the "uci" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleUciCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "debug" command to the engine
 *
 * @param args The debugging mode enable flag
 *
 * @return True on success
 */
bool UciProtocol::HandleDebugCommand(const std::vector<std::string>& args) {

}

/**
 * @brief Forwards the "isready" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleIsReadyCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "setoption" command to the engine
 *
 * @param args Option name and arguments
 *
 * @return True on success
 */
bool UciProtocol::HandleSetOptionCommand(const std::vector<std::string>& args) {

}

/**
 * @brief Forwards the "ucinewgame" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleUciNewGameCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "position" command to the engine
 *
 * @param args The FEN-encoded position
 *
 * @return True on success
 */
bool UciProtocol::HandlePositionCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "go" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleGoCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "stop" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleStopCommand(const std::vector<std::string>& ) {

}

/**
 * @brief Forwards the "ponderhit" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandlePonderHitCommand(const std::vector<std::string>& ) {

}

}  // namespace chess
