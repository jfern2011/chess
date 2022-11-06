/**
 *  \file   movegen_ut.cc
 *  \author Jason Fernandez
 *  \date   11/05/2022
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include "gtest/gtest.h"

#include "chess/debug.h"
#include "chess/movegen.h"
#include "chess/position.h"

namespace {
TEST(MoveGen, GeneratePawnAdvances) {
    auto pos = chess::Position();
    EXPECT_EQ(pos.Reset("4r2b/4P3/5P2/1q1PKP1r/3P4/4P3/1q1Pr3/5k2 w - - 0 1"),
              chess::Position::FenError::kSuccess);

    std::uint64_t pinned = pos.PinnedPieces<chess::Player::kWhite>();
    std::uint64_t target = ~pos.Occupied();

    std::array<std::uint32_t, 256> moves;
    
    std::size_t n_moves = chess::GeneratePawnAdvances<chess::Player::kWhite>(
                            pos, target, pinned, moves.data());

    EXPECT_EQ(n_moves, 2u);
}
}  // namespace
