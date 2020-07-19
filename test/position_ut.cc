/**
 *  \file   position_ut.cc
 *  \author Jason Fernandez
 *  \date   07/03/2020
 */

#include "gtest/gtest.h"
#include "chess/position.h"

namespace {

TEST(Position, FullMoveNumber) {
    chess::Position pos;
    EXPECT_EQ(pos.FullMoveNumber(), 0);
}

}  // namespace
