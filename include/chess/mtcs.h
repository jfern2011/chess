/**
 *  \file   mtcs.h
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#ifndef CHESS_MTCS_H_
#define CHESS_MTCS_H_

#include <array>
#include <cstdint>
#include <cstddef>

#include "chess/chess.h"
#include "chess/search.h"

namespace chess {
/**
 * @brief Monte Carlo Tree Search
 */
class Mtcs final : public Search {
public:
    /**
     * @brief Represents a single node in the game tree
     */
    class Node final {
    public:
        static Node* Create(const Position& position);

        explicit Node(const Position& position);

        double Average() const;

        Node* Select(const Position& position);
    
    private:
        /**
         * Successor nodes from *this
         */
        Node* childs_;

        /**
         * The hash signature of the position at this node
         */
        std::uint64_t hash_;

        /**
         * Number of successor nodes from *this
         */
        std::uint8_t num_childs_;

        /**
         * The total sum of scores backpropagated to this node
         */
        std::int32_t sum_;

        /**
         * The total number of visits to this node
         */
        std::uint32_t visits_;
    };

    std::uint32_t Run(const Position& position) override;

private:
    template <Player P>
    std::uint32_t Internal(const Position& position);

    template <Player P>
    std::uint32_t Root(const Position& position);
};

template <Player P>
std::uint32_t Mtcs::Internal(const Position& position) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = pos.InCheck<P>() ?
            GenerateCheckEvasions<P>(pos, moves.data()) :
            GenerateLegalMoves<P>(pos, moves.data());

}

template <Player P>
std::uint32_t Mtcs::Root(const Position& position) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = pos.InCheck<P>() ?
            GenerateCheckEvasions<P>(pos, moves.data()) :
            GenerateLegalMoves<P>(pos, moves.data());

}

}  // namespace chess

#endif  // CHESS_MTCS_H_
