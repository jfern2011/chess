/**
 *  \file   mtcs.h
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#ifndef CHESS_MTCS_H_
#define CHESS_MTCS_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "chess/chess.h"
#include "chess/evaluate.h"
#include "chess/logger.h"
#include "chess/memory_pool.h"
#include "chess/movegen.h"
#include "chess/search.h"

#define DEBUG_TRACE 1

namespace chess {
std::size_t random(std::size_t max_value);

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
        Node();

        Node(const Node& node) = default;
        Node(Node&& nodes) = default;
        Node& operator=(const Node& node) = default;
        Node& operator=(Node& node) = default;

        ~Node() = default;

        double Average() const;

        template <Player P>
        int Select(Position* position,
                   MemoryPool<Node>* pool,
                   std::size_t ply);

        std::uint32_t visits() const;

    private:
        Node* End();

        /**
         * Successor nodes from *this
         */
        Node* childs_;

        /**
         * The hash signature of the position at this node
         */
        std::uint64_t hash_;

        /**
         * Next sibling node to *this
         */
        Node* next_;

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

    Mtcs(std::shared_ptr<MemoryPool<Node>> pool,
         std::shared_ptr<Logger> logger);

    Mtcs(const Mtcs& algorithm) = default;
    Mtcs(Mtcs&& algorithm) = default;
    Mtcs& operator=(const Mtcs& algorithm) = default;
    Mtcs& operator=(Mtcs& algorithm) = default;

    ~Mtcs() = default;

    template <Player P>
    static int ComputeWin(const Position& position);

    std::uint32_t Run(const Position& position) override;

    template <Player P>
    static std::int32_t Simulate(Position* position, std::size_t ply);

private:
    template <Player P>
    std::pair<double, std::uint32_t> SelectRoot(Position* position);

    /**
     * Successor nodes from *this
     */
    std::vector<std::shared_ptr<Node>> childs_;

    /**
     * Number of iterations of this algorithm (i.e. calls to Run())
     */
    std::size_t iterations_;

    /**
     * For logging errors/diagnostics
     */
    std::shared_ptr<Logger> logger_;

    /**
     * Memory pool from which to allocate game tree nodes
     */
    std::shared_ptr<MemoryPool<Node>>
        node_pool_;
};

/**
 * @brief Select the next node to explore
 *
 * @param position The current position at this node
 * @param pool     The memory pool to allocate new nodes from
 * @param ply      The depth at this node
 *
 * @return The result of playing the selected move
 */
template <Player P>
int Mtcs::Node::Select(Position* position,
                       MemoryPool<Node>* pool,
                       std::size_t ply) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = position->InCheck<P>() ?
            GenerateCheckEvasions<P>(*position, moves.data()) :
            GenerateLegalMoves<P>(*position, moves.data());

    visits_++;

    if (n_moves == 0u) {
        const Result result = GameResult(*position);
        if (result == Result::kDraw) {
            return 0;
        } else {
            const int score = IsLostBy<P>(*position) ? -1 : 1;
            sum_ += score;

            return score;
        }
    }

    // First, search for a child node that has not been visited

    Node* selected = nullptr;
    std::size_t selected_index = 0;

    if (n_moves > num_childs_) {
        selected = new(pool->Allocate())Node;

        if (selected == nullptr) {
            return 0;
        }

        if (num_childs_ == 0u) {
            childs_ = selected;
        } else {
            End()->next_ = selected;
        }

        selected_index = num_childs_;

        num_childs_++;
    } else {
        // All nodes have been visited. Compute their UCB1 scores

        double best = -kInfinityF64;
        std::size_t index = 0;

        for (Node* node = childs_; node != nullptr; node = node->next_) {
            const double ucb1 = node->Average() +
                2.0 * std::sqrt(std::log(visits_) / node->visits_);

            if (ucb1 > best) {
                selected = node;
                selected_index = index;
                best = ucb1;
            }

            index++;
        }
    }

    // If the selected child is a terminal node, simulate a playout;
    // otherwise, step into it

    int result;

    if (selected->childs_) {
        position->MakeMove<P>(moves[selected_index], ply);

        result = -selected->Select<util::opponent<P>()>(position, pool, ply+1);

        position->UnMakeMove<P>(moves[selected_index], ply);
    } else {
        result = Mtcs::Simulate<P>(position, ply);
    }

    sum_ += result;

    return result;
}

/**
 * @brief Determine if the specified player has won
 *
 * @tparam P The player to check for victory
 *
 * @param position[in] The position to evaluate
 *
 * @return +1 for victory, -1 for loss, 0 for draw/game not over
 */
template <Player P>
int Mtcs::ComputeWin(const Position& position) {
    const Result result = GameResult(position);

    switch (result) {
      case Result::kBlackWon:
        return P == Player::kBlack ? 1 : -1;
      case Result::kWhiteWon:
        return P == Player::kWhite ? 1 : -1;
      default:
        return 0;
    }
}

/**
 * @brief Perform the selection step at the root node
 *
 * @tparam P The player whose turn it is
 *
 * @param position The current position
 *
 * @return Score and best move from this position
 */
template <Player P>
std::pair<double, std::uint32_t> Mtcs::SelectRoot(Position* position) {
    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = position->InCheck<P>() ?
            GenerateCheckEvasions<P>(*position, moves.data()) :
            GenerateLegalMoves<P>(*position, moves.data());

    iterations_++;

#if DEBUG_TRACE==1
    if (iterations_ >= 1999)
    logger_->Write("Iteration %zu\n", iterations_);
#endif

    // First, search for a child node that has not been visited

    std::shared_ptr<Node> selected;
    std::size_t selected_index = 0;

    if (n_moves > childs_.size()) {
        selected.reset(new Node());

        if (selected == nullptr) {
            return std::make_pair<double, std::uint32_t>(0, 0);
        }

        selected_index = childs_.size();

        childs_.push_back(selected);
#if DEBUG_TRACE==1
        if (iterations_ >= 1999)
        logger_->Write("(Root): Unexplored child %zu: %s\n",
                       selected_index,
                       util::ToLongAlgebraic(moves[selected_index]).c_str());
#endif
    } else {
        // All nodes have been visited. Compute their UCB1 scores

        double best = -kInfinityF64;
        std::size_t index = 0;

        for (std::shared_ptr<Node>& node : childs_) {
            const double ucb1 = node->Average() +
                2.0 * std::sqrt(std::log(iterations_) / node->visits());

            if (ucb1 > best) {
                selected = node;
                selected_index = index;
                best = ucb1;
            }

            index++;
        }
#if DEBUG_TRACE==1
        if (iterations_ >= 1999)
        logger_->Write("UCB1( selected => %s) = %0.6f with %u visits\n",
                       util::ToLongAlgebraic(moves[selected_index]).c_str(),
                       best,
                       childs_[selected_index]->visits());
#endif
    }

    // Step into the selected node. Note that playouts are never done at
    // the root

    constexpr std::size_t ply = 0;

    position->MakeMove<P>(moves[selected_index], ply);

    selected->Select<util::opponent<P>()>(position, node_pool_.get(), ply+1);

    position->UnMakeMove<P>(moves[selected_index], ply);

    // Select the move corresponding to the node with the maximum visits

    auto iter = std::max_element(childs_.begin(), childs_.end(),
                                 [](const std::shared_ptr<Node>& a,
                                    const std::shared_ptr<Node>& b) {
                                        return a->visits() < b->visits();
                                 });

    const auto index = std::distance(childs_.begin(), iter);

    return std::make_pair(childs_[index]->Average(), moves[index]);
}

/**
 * @brief Run the simulation step of Monte Carlo Tree Search
 *
 * @tparam P       The player whose turn it is
 * @param position Starting position to simulate from
 * @param ply      The depth at this position
 *
 * @return +1 if P has won, -1 if P has lost, 0 otherwise
 */
template <Player P>
std::int32_t Mtcs::Simulate(Position* position, std::size_t ply) {
    constexpr std::size_t max_ply = 200;
    static_assert(max_ply > 0u && max_ply <= kMaxPly);

    if (ply >= max_ply) return 0;

    std::array<std::uint32_t, kMaxMoves> moves;

    const std::size_t n_moves = position->InCheck<P>() ?
        GenerateCheckEvasions<P>(*position, moves.data()) :
        GenerateLegalMoves<P>(*position, moves.data());

    if (n_moves == 0) {
        return -ComputeWin<util::opponent<P>()>(*position);
    }

    const std::size_t selected_index = random(n_moves);

    const std::uint32_t move = moves[selected_index];

    position->MakeMove<P>(move, ply);

    const int result = -Simulate<util::opponent<P>()>(position, ply+1);

    position->UnMakeMove<P>(move, ply);

    return result;
}

}  // namespace chess

#endif  // CHESS_MTCS_H_
