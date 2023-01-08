/**
 *  \file   uci.h
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#ifndef CHESS_UCI_H_
#define CHESS_UCI_H_

#include <memory>

#include "chess/command_dispatcher.h"
#include "chess/engine_interface.h"
#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief Implements the UCI communication protocol
 */
class UciProtocol final {
public:
    UciProtocol(std::shared_ptr<chess::InputStreamChannel> channel,
                std::shared_ptr<EngineInterface> engine);

    UciProtocol(const UciProtocol& protocol)            = default;
    UciProtocol(UciProtocol&& protocol)                 = default;
    UciProtocol& operator=(const UciProtocol& protocol) = default;
    UciProtocol& operator=(UciProtocol&& protocol)      = default;

    ~UciProtocol() = default;

private:
    bool HandleUciCommand(const std::vector<std::string>& );
    bool HandleDebugCommand(const std::vector<std::string>& args);
    bool HandleIsReadyCommand(const std::vector<std::string>& );
    bool HandleSetOptionCommand(const std::vector<std::string>& args);
    bool HandleUciNewGameCommand(const std::vector<std::string>& );
    bool HandlePositionCommand(const std::vector<std::string>& );
    bool HandleGoCommand(const std::vector<std::string>& );
    bool HandleStopCommand(const std::vector<std::string>& );
    bool HandlePonderHitCommand(const std::vector<std::string>& );

    /**
     * Handles user commands
     */
    chess::CommandDispatcher dispatcher_;

    /**
     * Forward commands to this engine
     */
    std::shared_ptr<EngineInterface> engine_;

    /**
     * Channel to listen for commands
     */
    std::shared_ptr<chess::InputStreamChannel>
        input_channel_;
};

}  // namespace chess

#endif  // CHESS_UCI_H_
