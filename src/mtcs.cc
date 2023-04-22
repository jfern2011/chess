/**
 *  \file   mtcs.cc
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#include "chess/mtcs.h"

#include "chess/evaluate.h"

namespace chess {
/**
 * @brief Factory function for creating nodes within the game tree
 *
 * @param position Position the new node will represent
 *
 * @return The newly created node or nullptr if allocation fails
 */
auto Mtcs::Node::Create(const Position& position) -> Node* {

}

/**
 * @see Search::Run()
 */
std::uint32_t Mtcs::Run(const Position& position) {
    if (GameResult(position) == Result::kGameNotOver) {
        return kNullMove;
    }


}

}  // namespace chess
