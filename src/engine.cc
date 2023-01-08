/**
 *  \file   engine.cc
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#include "chess/engine.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param uci_channel Channel through which to emit UCI outputs
 * @param log_channel Channel through which to log internal engine info
 */
Engine::Engine(std::shared_ptr<OutputStreamChannel> uci_channel,
               std::shared_ptr<OutputStreamChannel> log_channel)
    : debug_mode_(false),
      is_running_(false),
      log_channel_(log_channel),
      master_(),
      uci_channel_(uci_channel) {
    master_.Reset();
}

/**
 * Handler for the UCI "uci" command
 */
void Engine::Uci() noexcept {
    uci_channel_->Write("id name NoName 1.0\n");
    uci_channel_->Write("id author jfern\n");
    uci_channel_->Write("uciok\n");
}

/**
 * @brief Handler for the UCI "debug" command
 *
 * @param enable True to enable debugging mode
 */
void Engine::DebugMode(bool enable) noexcept {
    debug_mode_ = enable;
}

/**
 * @brief Handler for the UCI "isready" command
 *
 * @return True if the engine is ready
 */
bool Engine::IsReady() const noexcept {
    return true;
}

/**
 * @brief Handler for the UCI "setoption" command
 *
 * @param name The name of this option
 * @param args Arguments to this option. May be an empty string if no arguments
 *             are required
 *
 * @return True if the option was successfully set
 */
bool Engine::SetOption(const std::string& name,
                       const std::string& args) noexcept {
    if (name.empty() && args.empty()) return false;
    return true;
}

/**
 * @brief Handler for the UCI "ucinewgame" command
 */
void Engine::UciNewGame() noexcept {
    Stop(); master_.Reset();
}

/**
 * @brief Handler for the UCI "position" command
 *
 * @param fen The position encoded in Forsyth-Edwards notation (FEN)
 *
 * @return True if \a fen was a valid position
 */
bool Engine::Position(const std::string& fen) noexcept {
    Stop();
    return master_.Reset(fen) == Position::FenError::kSuccess;
}

/**
 * @brief Handler for the UCI "go" command
 */
void Engine::Go() noexcept {
    is_running_ = true;
}

/**
 * @brief Handler for the UCI "stop" command
 */
void Engine::Stop() noexcept {
    is_running_ = false;
}

/**
 * @brief Handler for the UCI "ponderhit" command
 */
void Engine::PonderHit() noexcept {
}

}  // namespace chess
