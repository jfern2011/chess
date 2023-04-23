/**
 *  \file   search.h
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#ifndef CHESS_SEARCH_H_
#define CHESS_SEARCH_H_

#include <cstdint>

#include "chess/position.h"

namespace chess {
/**
 * @brief Generic interface for a search algorithm
 */
class Search {
public:
    virtual ~Search() = default;

    /**
     * @brief Run a new search
     *
     * @param position[in] The root position to search
     *
     * @return The selected move, or a null move if the game is over
     */
    virtual std::uint32_t Run(const Position& position) = 0;
};

}  // namespace chess

#endif  // CHESS_SEARCH_H_
