/**
 *  \file   engine.h
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <memory>
#include <string>

#include "chess/engine_interface.h"
#include "chess/stream_channel.h"
#include "chess/position.h"

namespace chess {
/**
 * @brief UCI chess engine
 */
class Engine final : public EngineInterface {
public:
    Engine(std::shared_ptr<OutputStreamChannel> uci_channel,
           std::shared_ptr<OutputStreamChannel> log_channel);

    Engine(const Engine& engine) = default;
    Engine(Engine&& engine) = default;
    Engine& operator=(const Engine& engine) = default;
    Engine& operator(Engine&& engine) = default;

    ~Engine() = default;

    void Uci() noexcept override;
    void DebugMode(bool enable) noexcept override;
    bool IsReady() const noexcept override;
    bool SetOption(const std::string& name,
                   const std::string& args) noexcept override;
    void UciNewGame() noexcept override;
    bool Position(const std::string& fen) noexcept override;
    void Go() noexcept override;
    void Stop() noexcept override;
    void PonderHit() noexcept override;

private:
    /**
     * True if debugging mode is enabled
     */
    bool debug_mode_;

    /**
     * True if a calculation is in progress
     */
    bool is_running_;

    /**
     * Channel through which to log internal info
     */
    std::shared_ptr<OutputStreamChannel>
        log_channel_;

    /**
     * Master position representing the root of the search tree
     */
    Position master_;

    /**
     * Channel through which to emit UCI outputs
     */
    std::shared_ptr<OutputStreamChannel>
        uci_channel_;
};

}  // namespace chess

#endif  // CHESS_ENGINE_H_
