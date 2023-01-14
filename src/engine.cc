/**
 *  \file   engine.cc
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#include "chess/engine.h"
#include "chess/interactive.h"

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "superstring/superstring.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param channel Channel through which to emit UCI outputs
 * @param logger  For logging internal statistics/data
 */
Engine::Engine(std::shared_ptr<OutputStreamChannel> channel,
               std::shared_ptr<Logger> logger)
    : channel_(channel),
      debug_mode_(false),
      is_running_(false),
      logger_(logger),
      master_() {
    master_.Reset();
}

/**
 * Handler for the UCI "uci" command
 */
void Engine::Uci() noexcept {
    channel_->Write("id name NoName 1.0\n");
    channel_->Write("id author jfern\n");
    channel_->Write("uciok\n");
}

/**
 * @brief Handler for the UCI "debug" command
 *
 * @param enable True to enable debugging mode
 */
void Engine::DebugMode(bool enable) noexcept {
    debug_mode_ = enable;

    logger_->Write("Debug mode %s.\n", enable ? "enabled" : "disabled");
}

/**
 * @brief Handler for the UCI "isready" command
 *
 * @return True if the engine is ready
 */
bool Engine::IsReady() const noexcept {
    channel_->Write("readyok\n");
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
                       const std::vector<std::string>& args) noexcept {
    if (name.empty() && args.empty()) return false;
    return true;
}

/**
 * @brief Handler for the UCI "ucinewgame" command
 */
void Engine::UciNewGame() noexcept {
    Stop();

    logger_->Write("Resetting for a new game.\n");
    master_.Reset();
}

/**
 * @brief Handler for the UCI "position" command
 *
 * @param args Arguments to this command
 *
 * @return True if \a fen was a valid position
 */
bool Engine::Position(const std::vector<std::string>& args) noexcept {
    Stop();

    const std::string input_string =
        jfern::superstring::build(", ", args.begin(), args.end());

    logger_->Write("Received 'position' command with arguments [%s]\n",
                   input_string.c_str());

    if (args.empty()) return false;

    auto moves_start = std::find(args.begin(), args.end(), "moves");

    std::string fen;

    if (args[0] == "fen") {
        const std::vector<std::string> fen_v(std::next(args.begin()),
                                             moves_start);

        fen = jfern::superstring::build(" ", fen_v.begin(), fen_v.end());

    } else if (args[0] == "startpos") {
        fen = Position::kDefaultFen;
    } else {
        logger_->Write("Error: expected 'fen' or 'startpos'\n");
        return false;
    }

    chess::Position backup(master_);

    const Position::FenError error = master_.Reset(fen);
    if (error != Position::FenError::kSuccess) {
        logger_->Write("Invalid FEN position [%s]: %s\n",
                       fen.c_str(),
                       Position::ErrorToString(error).c_str());
        return false;
    }

    // Play out the supplied move sequence

    if (moves_start != args.end()) {
        for (auto iter = std::next(moves_start), end = args.end();
             iter != end; ++iter) {
            const std::uint32_t move = ResolveMove(master_, *iter);

            if (move == kNullMove) {
                logger_->Write("Bad move in sequence '%s'\n", iter->c_str());
                master_ = backup;  // restore to original
                return false;
            }

            if (master_.ToMove() == Player::kWhite) {
                master_.MakeMove<Player::kWhite>(move, 0);
            } else {
                master_.MakeMove<Player::kBlack>(move, 0);
            }
        }
    }

    return true;
}

/**
 * @brief Handler for the UCI "go" command
 */
void Engine::Go() noexcept {
    is_running_ = true;
    logger_->Write("Search has started.\n");
}

/**
 * @brief Handler for the UCI "stop" command
 */
void Engine::Stop() noexcept {
    if (is_running_) {
        logger_->Write("Search was stopped.\n");
    }

    is_running_ = false;
}

/**
 * @brief Handler for the UCI "ponderhit" command
 */
void Engine::PonderHit() noexcept {
}

}  // namespace chess
