/**
 *  \file   uci.h
 *  \author Jason Fernandez
 *  \date   12/21/2021
 */

#ifndef CHESS_UCI_H_
#define CHESS_UCI_H_

#include <memory>

#include "chess/command_dispatcher.h"
#include "chess/data_buffer.h"
#include "chess/logger.h"
#include "chess/engine_interface.h"
#include "chess/stream_channel.h"

namespace chess {
/**
 * @brief Implements the UCI communication protocol
 */
class UciProtocol final {
public:
    UciProtocol(std::shared_ptr<InputStreamChannel> channel,
                std::shared_ptr<Logger> logger,
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
    bool HandlePositionCommand(const std::vector<std::string>& args);
    bool HandleGoCommand(const std::vector<std::string>& );
    bool HandleStopCommand(const std::vector<std::string>& );
    bool HandlePonderHitCommand(const std::vector<std::string>& );
    bool HandleQuitCommand(const std::vector<std::string>& );
    void HandleCommandUnknown(const ConstDataBuffer& buf);

    /**
     * Handles user commands
     */
    CommandDispatcher dispatcher_;

    /**
     * Forward commands to this engine
     */
    std::shared_ptr<EngineInterface> engine_;

    /**
     * Channel to listen for commands
     */
    std::shared_ptr<InputStreamChannel>
        input_channel_;

    /**
     * Internal log messages written here
     */
    std::shared_ptr<Logger> logger_;
};

}  // namespace chess

#endif  // CHESS_UCI_H_
