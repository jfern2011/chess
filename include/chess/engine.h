/**
 *  \file   engine.h
 *  \author Jason Fernandez
 *  \date   01/08/2023
 */

#ifndef CHESS_ENGINE_H_
#define CHESS_ENGINE_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "chess/engine_interface.h"
#include "chess/logger.h"
#include "chess/memory_pool.h"
#include "chess/movegen.h"
#include "chess/mtcs.h"
#include "chess/stream_channel.h"
#include "chess/position.h"

namespace chess {
/**
 * @brief UCI chess engine
 */
class Engine final : public EngineInterface {
public:
    Engine(std::shared_ptr<OutputStreamChannel> channel,
           std::shared_ptr<Logger> logger);

    Engine(const Engine& engine) = default;
    Engine(Engine&& engine) = default;
    Engine& operator=(const Engine& engine) = default;
    Engine& operator=(Engine&& engine) = default;

    ~Engine() = default;

    void Uci() noexcept override;
    void DebugMode(bool enable) noexcept override;
    bool IsReady() const noexcept override;
    bool SetOption(const std::string& name,
                   const std::vector<std::string>& args) noexcept override;
    void UciNewGame() noexcept override;
    bool Position (const std::vector<std::string>& args) noexcept override;
    void Go() noexcept override;
    void Stop() noexcept override;
    void PonderHit() noexcept override;

private:
    template <Player P>
    std::int16_t Search(std::uint32_t* bestmove);

    /**
     * Channel through which to emit UCI outputs
     */
    std::shared_ptr<OutputStreamChannel>
        channel_;

    /**
     * True if debugging mode is enabled
     */
    bool debug_mode_;

    /**
     * True if a calculation is in progress
     */
    bool is_running_;

    /**
     * Object through which to log internal info
     */
    std::shared_ptr<Logger> logger_;

    /**
     * Master position representing the root of the search tree
     */
    chess::Position master_;

    /**
     * Memory pool to allocate the game tree from
     */
    std::shared_ptr<MemoryPool<Mtcs::Node>>
        mem_pool_;
};

/**
 * Find the best move given the current position
 *
 * @param[out] bestmove The best move found by the search
 *
 * @return The calculated optimal score
 */
template <Player P>
std::int16_t Engine::Search(std::uint32_t* bestmove) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = master_.InCheck<P>() ?
            GenerateCheckEvasions<P>(master_, moves.data()) :
               GenerateLegalMoves<P>(master_, moves.data());

    logger_->Write("Node size = %zu\n", sizeof(Mtcs::Node));

    auto mtcs_log = std::make_shared<Logger>("MTCS", channel_);

    if (!mem_pool_) {
        auto mem_log = std::make_shared<Logger>("MemoryPool", channel_);

        mem_pool_ = std::make_shared<MemoryPool<Mtcs::Node>>(100000000, mem_log);
    } else {
        mem_pool_->Free();
    }

    auto mtcs = std::make_shared<Mtcs>(mem_pool_, mtcs_log);

    auto result = mtcs->Run(master_);
    mtcs_log->Write("Analysis: %s\n", util::ToLongAlgebraic(result).c_str());

    *bestmove = n_moves == 0 ? 0 : result;

    return 0;
}

}  // namespace chess

#endif  // CHESS_ENGINE_H_
