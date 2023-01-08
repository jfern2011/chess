/**
 *  \file   uci.cc
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#include "chess/uci.h"

#include <vector>

#include "superstring/superstring.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param channel The channel to listen for UCI commands
 * @param logger  Write internal (non-UCI) info to this channel
 * @param engine  The engine to forward commands to
 */
UciProtocol::UciProtocol(std::shared_ptr<chess::InputStreamChannel> channel,
                         std::shared_ptr<Logger> logger,
                         std::shared_ptr<EngineInterface> engine)
    : dispatcher_(),
      engine_(engine),
      input_channel_(channel),
      logger_(logger) {
    dispatcher_.RegisterCommand(
        "uci",
        std::bind(&UciProtocol::HandleUciCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "debug",
        std::bind(&UciProtocol::HandleDebugCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "isready",
        std::bind(&UciProtocol::HandleIsReadyCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "setoption",
        std::bind(&UciProtocol::HandleSetOptionCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "ucinewgame",
        std::bind(&UciProtocol::HandleUciNewGameCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "position",
        std::bind(&UciProtocol::HandlePositionCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "go",
        std::bind(&UciProtocol::HandleGoCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "stop",
        std::bind(&UciProtocol::HandleStopCommand, this,
                  std::placeholders::_1));
    dispatcher_.RegisterCommand(
        "ponderhit",
        std::bind(&UciProtocol::HandlePonderHitCommand, this,
                  std::placeholders::_1));

    dispatcher_.error_callback_ =
        std::bind(&UciProtocol::HandleCommandUnknown, this,
                  std::placeholders::_1);

    input_channel_->emit_ =
        std::bind(&chess::CommandDispatcher::HandleCommand,
                  &dispatcher_,
                  std::placeholders::_1);
}

/**
 * @brief Forwards the "uci" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleUciCommand(const std::vector<std::string>& ) {
    engine_->Uci(); return true;
}

/**
 * @brief Forwards the "debug" command to the engine
 *
 * @param args The debugging mode enable flag
 *
 * @return True on success
 */
bool UciProtocol::HandleDebugCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        logger_->Write("HandleDebugCommand: no arguments.\n");
        return false;
    }

    if (args[0] == "on") {
        engine_->DebugMode(true);
        return true;
    } else if (args[0] == "off") {
        engine_->DebugMode(false);
        return true;
    } else {
        logger_->Write("HandleDebugCommand: argument '%s' is invalid.\n",
                       args[0].c_str());
        return false;
    }
}

/**
 * @brief Forwards the "isready" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleIsReadyCommand(const std::vector<std::string>& ) {
    return engine_->IsReady();
}

/**
 * @brief Forwards the "setoption" command to the engine
 *
 * @param args Option name and arguments
 *
 * @return True on success
 */
bool UciProtocol::HandleSetOptionCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        logger_->Write("HandleSetOptionCommand: no arguments.\n");
        return false;
    }

    const std::string& name = args[0];
    std::vector<std::string> settings(std::next(args.begin()), args.end());

    return engine_->SetOption(name, settings);
}

/**
 * @brief Forwards the "ucinewgame" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleUciNewGameCommand(const std::vector<std::string>& ) {
    engine_->UciNewGame(); return true;
}

/**
 * @brief Forwards the "position" command to the engine
 *
 * @param args The FEN-encoded position
 *
 * @return True on success
 */
bool UciProtocol::HandlePositionCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        logger_->Write("HandlePositionCommand: no arguments.\n");
        return false;
    }

    return engine_->Position(args);
}

/**
 * @brief Forwards the "go" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleGoCommand(const std::vector<std::string>& ) {
    engine_->Go(); return true;
}

/**
 * @brief Forwards the "stop" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandleStopCommand(const std::vector<std::string>& ) {
    engine_->Stop(); return true;
}

/**
 * @brief Forwards the "ponderhit" command to the engine
 *
 * @return True on success
 */
bool UciProtocol::HandlePonderHitCommand(const std::vector<std::string>& ) {
    engine_->PonderHit(); return true;
}

/**
 * Called back when an unknown command is issued
 *
 * @param buf The command data
 */
void UciProtocol::HandleCommandUnknown(const chess::ConstDataBuffer& buf) {
    const jfern::superstring sstring(std::string(buf.data(), buf.size()));

    const std::vector<std::string> tokens = sstring.split();
    if (!tokens.empty()) {
        logger_->Write("Unknown command \'%s\'\n", tokens[0].c_str());
    }
}

}  // namespace chess
