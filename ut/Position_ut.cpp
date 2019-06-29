#include "gtest/gtest.h"

#include <iostream>

/*
 * Note: Include *after* gtest to protect against
 *       name conflicts with abort macros
 */

#include "src/Position4.h"
#include "util/bit_tools.h"
#include "util/str_util.h"

using namespace Chess;

namespace
{
    TEST(Position, hashCastleWhite)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        Position position(stream,
            "r3k2r/ppp2ppp/2nqbn2/2bpp3/2BPP3/2NQBN2/PPP2PPP/R3K2R w KQkq - 0 1");

        const auto& inputs = position.get_hash_inputs();
        int64 key = position.get_hash_key();

        {
            /*
             * White king castles short
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E1,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::G1);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_K_index]
                                ^ inputs.castle_rights[player_t::white][castle_Q_index]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::E1]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::G1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::H1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::F1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White king castles long
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E1,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::C1);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_K_index]
                                ^ inputs.castle_rights[player_t::white][castle_Q_index]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::E1]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::C1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::A1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::D1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White king advances
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E1,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::E2);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_K_index]
                                ^ inputs.castle_rights[player_t::white][castle_Q_index]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::E1]
                                ^ inputs.piece[player_t::white][piece_t::king][square_t::E2]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White's H-rook moves
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::H1,   // from
                                   piece_t::rook,  // moved
                                   piece_t::empty, // promote
                                   square_t::G1);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_K_index]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::H1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::G1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White's A-rook moves
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::A1,   // from
                                   piece_t::rook,  // moved
                                   piece_t::empty, // promote
                                   square_t::B1);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_Q_index]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::A1]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::B1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White captures Black's A-rook
             */

            ASSERT_TRUE(position.reset(
                "r3k2r/ppp2ppp/1Nnqbn2/2bpp3/2BPP3/2NQBN2/PPP2PPP/R3K2R w KQkq - 0 1"));

            key = position.get_hash_key();

            int32 move = pack_move(piece_t::rook,   // captured
                                   square_t::B6,    // from
                                   piece_t::knight, // moved
                                   piece_t::empty,  // promote
                                   square_t::A8);   // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_Q_index]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::A8]
                                ^ inputs.piece[player_t::white][piece_t::knight][square_t::B6]
                                ^ inputs.piece[player_t::white][piece_t::knight][square_t::A8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * White captures Black's H-rook
             */

            ASSERT_TRUE(position.reset(
                "r3k2r/ppp2ppp/2nqbnN1/2bpp3/2BPP3/2NQBN2/PPP2PPP/R3K2R w KQkq - 0 1"));

            key = position.get_hash_key();

            int32 move = pack_move(piece_t::rook,   // captured
                                   square_t::G6,    // from
                                   piece_t::knight, // moved
                                   piece_t::empty,  // promote
                                   square_t::H8);   // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_K_index]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::H8]
                                ^ inputs.piece[player_t::white][piece_t::knight][square_t::G6]
                                ^ inputs.piece[player_t::white][piece_t::knight][square_t::H8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }
    }

    TEST(Position, hashCastleBlack)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        Position position(stream,
            "r3k2r/ppp2ppp/2nqbn2/2bpp3/2BPP3/2NQBN2/PPP2PPP/R3K2R b KQkq - 0 1");

        const auto& inputs = position.get_hash_inputs();
        int64 key = position.get_hash_key();

        {
            /*
             * Black king castles short
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E8,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::G8);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_K_index]
                                ^ inputs.castle_rights[player_t::black][castle_Q_index]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::E8]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::G8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::H8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::F8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black king castles long
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E8,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::C8);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_K_index]
                                ^ inputs.castle_rights[player_t::black][castle_Q_index]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::E8]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::C8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::A8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::D8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black king advances
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E8,   // from
                                   piece_t::king,  // moved
                                   piece_t::empty, // promote
                                   square_t::E7);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_K_index]
                                ^ inputs.castle_rights[player_t::black][castle_Q_index]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::E8]
                                ^ inputs.piece[player_t::black][piece_t::king][square_t::E7]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black's H-rook moves
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::H8,   // from
                                   piece_t::rook,  // moved
                                   piece_t::empty, // promote
                                   square_t::G8);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_K_index]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::H8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::G8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black's A-rook moves
             */

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::A8,   // from
                                   piece_t::rook,  // moved
                                   piece_t::empty, // promote
                                   square_t::B8);  // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::black][castle_Q_index]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::A8]
                                ^ inputs.piece[player_t::black][piece_t::rook][square_t::B8]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black captures White's A-rook
             */

            ASSERT_TRUE(position.reset(
                "r3k2r/ppp2ppp/2nqbn2/2bpp3/2BPP3/1nNQBN2/PPP2PPP/R3K2R b KQkq - 0 1"));

            key = position.get_hash_key();

            int32 move = pack_move(piece_t::rook,   // captured
                                   square_t::B3,    // from
                                   piece_t::knight, // moved
                                   piece_t::empty,  // promote
                                   square_t::A1);   // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_Q_index]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::A1]
                                ^ inputs.piece[player_t::black][piece_t::knight][square_t::B3]
                                ^ inputs.piece[player_t::black][piece_t::knight][square_t::A1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black captures White's H-rook
             */

            ASSERT_TRUE(position.reset(
                "r3k2r/ppp2ppp/2nqbn2/2bpp3/2BPP3/2NQBNn1/PPP2PPP/R3K2R b KQkq - 0 1"));

            key = position.get_hash_key();

            int32 move = pack_move(piece_t::rook,   // captured
                                   square_t::G3,    // from
                                   piece_t::knight, // moved
                                   piece_t::empty,  // promote
                                   square_t::H1);   // to

            position.make_move(move);

            int64 new_key = key ^ inputs.castle_rights[player_t::white][castle_K_index]
                                ^ inputs.piece[player_t::white][piece_t::rook][square_t::H1]
                                ^ inputs.piece[player_t::black][piece_t::knight][square_t::G3]
                                ^ inputs.piece[player_t::black][piece_t::knight][square_t::H1]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }
    }

    TEST(Position, hashEnPassant)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        {
            Position position(stream, "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            for (int sq = square_t::A2; sq > square_t::A1; sq--)
            {
                const square_t from = static_cast<square_t>(sq);
                const square_t to   = static_cast<square_t>(sq+16);

                int32 move = pack_move(piece_t::empty, // captured
                                       from,           // from
                                       piece_t::pawn,  // moved
                                       piece_t::empty, // promote
                                       to);            // to

                position.make_move(move);

                int64 new_key = key ^ inputs.piece[player_t::white][piece_t::pawn][from]
                                    ^ inputs.piece[player_t::white][piece_t::pawn][to]
                                    ^ inputs.en_passant[get_file(from)]
                                    ^ inputs.to_move;

                EXPECT_EQ(position.get_hash_key(), new_key);

                position.unmake_move(move);

                EXPECT_EQ(position.get_hash_key(), key);
            }
        }

        {
            /*
             * White captures en passant
             */

            Position position(stream, "4k3/ppp1pppp/8/3pP3/8/8/PPPP1PPP/4K3 w - d6 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            const square_t from = square_t::E5;
            const square_t to   = square_t::D6;
            

            int32 move = pack_move(piece_t::pawn,  // captured
                                   from,           // from
                                   piece_t::pawn,  // moved
                                   piece_t::empty, // promote
                                   to);            // to

            position.make_move(move);

            int64 new_key = key ^ inputs.piece[player_t::white][piece_t::pawn][from]
                                ^ inputs.piece[player_t::white][piece_t::pawn][to]
                                ^ inputs.piece[player_t::black][piece_t::pawn][square_t::D5]
                                ^ inputs.en_passant[get_file(square_t::D6)]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }

        {
            /*
             * Black captures en passant
             */

            Position position(stream, "4k3/pppp1ppp/8/8/3Pp3/8/PPP2PPP/4K3 b - d3 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            const square_t from = square_t::E4;
            const square_t to   = square_t::D3;
            

            int32 move = pack_move(piece_t::pawn,  // captured
                                   from,           // from
                                   piece_t::pawn,  // moved
                                   piece_t::empty, // promote
                                   to);            // to

            position.make_move(move);

            int64 new_key = key ^ inputs.piece[player_t::black][piece_t::pawn][from]
                                ^ inputs.piece[player_t::black][piece_t::pawn][to]
                                ^ inputs.piece[player_t::white][piece_t::pawn][square_t::D4]
                                ^ inputs.en_passant[get_file(square_t::D3)]
                                ^ inputs.to_move;

            EXPECT_EQ(position.get_hash_key(), new_key);

            position.unmake_move(move);

            EXPECT_EQ(position.get_hash_key(), key);
        }
    }

    TEST(Position, hashPromotion)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        {
            /*
             * White promotes
             */

            Position position(stream, "8/3P4/1K6/8/2k5/8/8/8 w - - 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            for (auto piece : {piece_t::rook,
                               piece_t::knight,
                               piece_t::bishop,
                               piece_t::queen})
            {
                const square_t from = square_t::D7;
                const square_t to   = square_t::D8;

                int32 move = pack_move(piece_t::empty, // captured
                                       from,           // from
                                       piece_t::pawn,  // moved
                                       piece,          // promote
                                       to);            // to

                position.make_move(move);

                int64 new_key = key ^ inputs.piece[player_t::white][piece_t::pawn][from]
                                    ^ inputs.piece[player_t::white][piece][to]
                                    ^ inputs.to_move;

                EXPECT_EQ(position.get_hash_key(), new_key);

                position.unmake_move(move);

                EXPECT_EQ(position.get_hash_key(), key);
            }
        }

        {
            /*
             * White captures and promotes
             */

            Position position(stream, "4n3/3P4/1K6/8/2k5/8/8/8 w - - 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            for (auto piece : {piece_t::rook,
                               piece_t::knight,
                               piece_t::bishop,
                               piece_t::queen})
            {
                const square_t from = square_t::D7;
                const square_t to   = square_t::E8;

                int32 move = pack_move(piece_t::knight, // captured
                                       from,            // from
                                       piece_t::pawn,   // moved
                                       piece,           // promote
                                       to);             // to

                position.make_move(move);

                int64 new_key = key ^ inputs.piece[player_t::white][piece_t::pawn][from]
                                    ^ inputs.piece[player_t::white][piece][to]
                                    ^ inputs.piece[player_t::black][piece_t::knight][to]
                                    ^ inputs.to_move;

                EXPECT_EQ(position.get_hash_key(), new_key);

                position.unmake_move(move);

                EXPECT_EQ(position.get_hash_key(), key);
            }
        }

        {
            /*
             * Black promotes
             */

            Position position(stream, "8/8/1K6/8/2k5/8/3p4/4N3 b - - 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            for (auto piece : {piece_t::rook,
                               piece_t::knight,
                               piece_t::bishop,
                               piece_t::queen})
            {
                const square_t from = square_t::D2;
                const square_t to   = square_t::D1;

                int32 move = pack_move(piece_t::empty, // captured
                                       from,           // from
                                       piece_t::pawn,  // moved
                                       piece,          // promote
                                       to);            // to

                position.make_move(move);

                int64 new_key = key ^ inputs.piece[player_t::black][piece_t::pawn][from]
                                    ^ inputs.piece[player_t::black][piece][to]
                                    ^ inputs.to_move;

                EXPECT_EQ(position.get_hash_key(), new_key);

                position.unmake_move(move);

                EXPECT_EQ(position.get_hash_key(), key);
            }
        }

        {
            /*
             * Black captures and promotes
             */

            Position position(stream, "8/8/1K6/8/2k5/8/3p4/4N3 b - - 0 1");

            const auto& inputs = position.get_hash_inputs();
            int64 key = position.get_hash_key();

            for (auto piece : {piece_t::rook,
                               piece_t::knight,
                               piece_t::bishop,
                               piece_t::queen})
            {
                const square_t from = square_t::D2;
                const square_t to   = square_t::E1;

                int32 move = pack_move(piece_t::knight, // captured
                                       from,            // from
                                       piece_t::pawn,   // moved
                                       piece,           // promote
                                       to);             // to

                position.make_move(move);

                int64 new_key = key ^ inputs.piece[player_t::black][piece_t::pawn][from]
                                    ^ inputs.piece[player_t::black][piece][to]
                                    ^ inputs.piece[player_t::white][piece_t::knight][to]
                                    ^ inputs.to_move;

                EXPECT_EQ(position.get_hash_key(), new_key);

                position.unmake_move(move);

                EXPECT_EQ(position.get_hash_key(), key);
            }
        }
    }

    TEST(Position, make_unmake_pawn)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        Position position(stream);

        {
            /* Test pawn advances 1 */

            EXPECT_TRUE(position.reset());

            Position copy(position);

            ASSERT_EQ(position, copy);

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E2,   // from
                                   piece_t::pawn,  // moved
                                   piece_t::empty, // promote
                                   square_t::E3);  // to

            const uint64 orig_hash = position.get_hash_key();
            const uint64 orig_occupied = position.get_occupied(player_t::white);
            const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), 0);

            EXPECT_EQ(position.get_turn(), player_t::white);

            EXPECT_STREQ(position.get_fen().c_str(),
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_TRUE(position.make_move(move));

            EXPECT_STREQ(position.get_fen().c_str(),
                "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_NE(position.get_hash_key(), orig_hash);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), 0);

            uint64 new_occupied = orig_occupied;

            clear_set64(square_t::E2,
                        square_t::E3,
                        new_occupied);

            EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

            uint64 new_pawns = orig_pawns;

            clear_set64(square_t::E2,
                        square_t::E3,
                        new_pawns);

            EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

            EXPECT_EQ(position.get_turn(), player_t::black);

            EXPECT_EQ(position.piece_on(square_t::E2),
                piece_t::empty);
            EXPECT_EQ(position.piece_on(square_t::E3),
                piece_t::pawn);

            EXPECT_TRUE(position.unmake_move(move));

            EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test pawn advances 2 */

            EXPECT_TRUE(position.reset());

            Position copy(position);

            ASSERT_EQ(position, copy);

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::E2,   // from
                                   piece_t::pawn,  // moved
                                   piece_t::empty, // promote
                                   square_t::E4);  // to

            const uint64 orig_hash = position.get_hash_key();
            const uint64 orig_occupied = position.get_occupied(player_t::white);
            const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), 0);

            EXPECT_EQ(position.get_turn(), player_t::white);

            EXPECT_STREQ(position.get_fen().c_str(),
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_TRUE(position.make_move(move));

            EXPECT_STREQ(position.get_fen().c_str(),
                "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_NE(position.get_hash_key(), orig_hash);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), 0);

            uint64 new_occupied = orig_occupied;

            clear_set64(square_t::E2,
                        square_t::E4,
                        new_occupied);

            EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

            uint64 new_pawns = orig_pawns;

            clear_set64(square_t::E2,
                        square_t::E4,
                        new_pawns);

            EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

            EXPECT_EQ(position.get_turn(), player_t::black);

            EXPECT_EQ(position.piece_on(square_t::E2),
                piece_t::empty);
            EXPECT_EQ(position.piece_on(square_t::E4),
                piece_t::pawn);

            EXPECT_TRUE(position.unmake_move(move));

            EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test pawn captures left */

            piece_t pieces[] = {piece_t::rook, piece_t::knight, piece_t::bishop,
                                piece_t::queen};

            char piece_str[]  = {'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 4; n++)
            {
                std::string init_fen = "8/3X1n2/4P3/2K5/8/8/8/4k3 w - - 0 1";
                init_fen[3] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(pieces[n],      // captured
                                       square_t::E6,   // from
                                       piece_t::pawn,  // moved
                                       piece_t::empty, // promote
                                       square_t::D7);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                uint64 orig_xpiece64 = 0;

                int material = pawn_value - knight_value;

                if (pieces[n] == piece_t::rook)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                    material -= rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                    material -= knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                    material -= bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                    material -= queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "8/3P1n2/8/2K5/8/8/8/4k3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                if (pieces[n] == piece_t::rook)
                {
                    material += rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    material += knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    material += bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    material += queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::E6,
                            square_t::D7,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::D7, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpiece64 = orig_xpiece64;
                clear_bit64(square_t::D7, new_xpiece64);

                if (pieces[n] == piece_t::rook)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                }
                else if (pieces[n] == piece_t::knight)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                }
                else if (pieces[n] == piece_t::queen)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                }

                uint64 new_pawns = orig_pawns;

                clear_set64(square_t::E6,
                            square_t::D7,
                            new_pawns);

                EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::E6),
                    piece_t::empty);
                EXPECT_EQ(position.piece_on(square_t::D7),
                    piece_t::pawn);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
            }
        }

        {
            /* Test pawn captures right */

            piece_t pieces[] = {piece_t::rook, piece_t::knight, piece_t::bishop,
                                piece_t::queen};

            char piece_str[]  = {'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 4; n++)
            {
                std::string init_fen = "8/3n1X2/4P3/2K5/8/8/8/4k3 w - - 0 1";
                init_fen[5] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(pieces[n],      // captured
                                       square_t::E6,   // from
                                       piece_t::pawn,  // moved
                                       piece_t::empty, // promote
                                       square_t::F7);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                uint64 orig_xpiece64 = 0;

                int material = pawn_value - knight_value;

                if (pieces[n] == piece_t::rook)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                    material -= rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                    material -= knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                    material -= bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                    material -= queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "8/3n1P2/8/2K5/8/8/8/4k3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                if (pieces[n] == piece_t::rook)
                {
                    material += rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    material += knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    material += bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    material += queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::E6,
                            square_t::F7,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::F7, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpiece64 = orig_xpiece64;
                clear_bit64(square_t::F7, new_xpiece64);

                if (pieces[n] == piece_t::rook)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                }
                else if (pieces[n] == piece_t::knight)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                }
                else if (pieces[n] == piece_t::queen)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                }

                uint64 new_pawns = orig_pawns;

                clear_set64(square_t::E6,
                            square_t::F7,
                            new_pawns);

                EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::E6),
                    piece_t::empty);
                EXPECT_EQ(position.piece_on(square_t::F7),
                    piece_t::pawn);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
            }
        }

        {
            /* Test pawn captures left en passant */

                std::string init_fen = "4k3/8/8/3PpP2/8/8/8/4K3 w - e6 0 1";

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(piece_t::pawn,  // captured
                                       square_t::D5,   // from
                                       piece_t::pawn,  // moved
                                       piece_t::empty, // promote
                                       square_t::E6);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);
                const uint64 orig_xpawns = position.get_bitboard<piece_t::pawn>(player_t::black);

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), pawn_value);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "4k3/8/4P3/5P2/8/8/8/4K3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), 2*pawn_value);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::D5,
                            square_t::E6,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::E5, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpawns = orig_xpawns;
                clear_bit64(square_t::E5, new_xpawns);

                EXPECT_EQ(new_xpawns, position.get_bitboard<piece_t::pawn>(player_t::black));

                uint64 new_pawns = orig_pawns;

                clear_set64(square_t::D5,
                            square_t::E6,
                            new_pawns);

                EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::E6),
                    piece_t::pawn);
                EXPECT_EQ(position.piece_on(square_t::D5),
                    piece_t::empty);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test pawn captures right en passant */

                std::string init_fen = "4k3/8/8/3PpP2/8/8/8/4K3 w - e6 0 1";

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(piece_t::pawn,  // captured
                                       square_t::F5,   // from
                                       piece_t::pawn,  // moved
                                       piece_t::empty, // promote
                                       square_t::E6);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);
                const uint64 orig_xpawns = position.get_bitboard<piece_t::pawn>(player_t::black);

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), pawn_value);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "4k3/8/4P3/3P4/8/8/8/4K3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), 2*pawn_value);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::F5,
                            square_t::E6,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::E5, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpawns = orig_xpawns;
                clear_bit64(square_t::E5, new_xpawns);

                EXPECT_EQ(new_xpawns, position.get_bitboard<piece_t::pawn>(player_t::black));

                uint64 new_pawns = orig_pawns;

                clear_set64(square_t::F5,
                            square_t::E6,
                            new_pawns);

                EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::E6),
                    piece_t::pawn);
                EXPECT_EQ(position.piece_on(square_t::F5),
                    piece_t::empty);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test pawn advances 1 and promotes */

            EXPECT_TRUE(position.reset("8/4P3/8/2K5/8/8/8/4k3 w - - 0 1"));

            Position copy(position);

            ASSERT_EQ(position, copy);

            piece_t pieces[] = {piece_t::rook, piece_t::knight, piece_t::bishop,
                                piece_t::queen};

            char piece_str[]  = {'R', 'N', 'B', 'Q'};

            for (int i = 0 ; i < 4; i++)
            {
                int32 move = pack_move(piece_t::empty, // captured
                                       square_t::E7,   // from
                                       piece_t::pawn,  // moved
                                       pieces[i],      // promote
                                       square_t::E8);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), pawn_value);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(),
                    "8/4P3/8/2K5/8/8/8/4k3 w - - 0 1");

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "4X3/8/8/2K5/8/8/8/4k3 b - - 0 1";
                fen_str[1]          = piece_str[i];

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                if (pieces[i] == piece_t::rook)
                {
                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), rook_value);

                    EXPECT_EQ(position.get_bitboard<piece_t::rook>(player_t::white),
                        Util::get_bit<uint64>(square_t::E8));
                }
                else if (pieces[i] == piece_t::knight)
                {
                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), knight_value);

                    EXPECT_EQ(position.get_bitboard<piece_t::knight>(player_t::white),
                        Util::get_bit<uint64>(square_t::E8));
                }
                else if (pieces[i] == piece_t::bishop)
                {
                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), bishop_value);

                    EXPECT_EQ(position.get_bitboard<piece_t::bishop>(player_t::white),
                        Util::get_bit<uint64>(square_t::E8));
                }
                else if (pieces[i] == piece_t::queen)
                {
                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), queen_value);

                    EXPECT_EQ(position.get_bitboard<piece_t::queen>(player_t::white),
                        Util::get_bit<uint64>(square_t::E8));
                }

                uint64 new_occupied = orig_occupied;

                clear_set64(square_t::E7,
                            square_t::E8,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_pawns = orig_pawns;

                clear_bit64(square_t::E7, new_pawns);

                EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::E7),
                    piece_t::empty);
                EXPECT_EQ(position.piece_on(square_t::E8),
                    pieces[i]);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
            }
        }

        {
            /* Test pawn captures left and promotes */

            piece_t pieces[] = {piece_t::rook, piece_t::knight, piece_t::bishop,
                                piece_t::queen};

            char piece_str[]  = {'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 4; n++)
            {
                std::string init_fen = "3X1n2/4P3/8/2K5/8/8/8/4k3 w - - 0 1";
                init_fen[1] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                for (int i = 0 ; i < 4; i++)
                {
                    int32 move = pack_move(pieces[n],      // captured
                                           square_t::E7,   // from
                                           piece_t::pawn,  // moved
                                           pieces[i],      // promote
                                           square_t::D8);  // to

                    const uint64 orig_hash = position.get_hash_key();
                    const uint64 orig_occupied = position.get_occupied(player_t::white);
                    const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                    const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                    uint64 orig_xpiece64 = 0;

                    int material = pawn_value - knight_value;

                    if (pieces[n] == piece_t::rook)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                        material -= rook_value;
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                        material -= knight_value;
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                        material -= bishop_value;
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                        material -= queen_value;
                    }

                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), material);

                    EXPECT_EQ(position.get_turn(), player_t::white);

                    EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                    EXPECT_EQ(position.get_fullmove_number(), 1);

                    EXPECT_TRUE(position.make_move(move));

                    std::string fen_str = "3X1n2/8/8/2K5/8/8/8/4k3 b - - 0 1";
                    fen_str[1]          = piece_str[i];

                    EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                    EXPECT_EQ(position.get_fullmove_number(), 1);

                    EXPECT_NE(position.get_hash_key(), orig_hash);

                    if (pieces[i] == piece_t::rook)
                    {
                        material += rook_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::rook>(player_t::white),
                            Util::get_bit<uint64>(square_t::D8));
                    }
                    else if (pieces[i] == piece_t::knight)
                    {
                        material += knight_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::knight>(player_t::white),
                            Util::get_bit<uint64>(square_t::D8));
                    }
                    else if (pieces[i] == piece_t::bishop)
                    {
                        material += bishop_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::bishop>(player_t::white),
                            Util::get_bit<uint64>(square_t::D8));
                    }
                    else if (pieces[i] == piece_t::queen)
                    {
                        material += queen_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::queen>(player_t::white),
                            Util::get_bit<uint64>(square_t::D8));
                    }

                    if (pieces[n] == piece_t::rook)
                    {
                        material += rook_value;
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        material += knight_value;
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        material += bishop_value;
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        material += queen_value;
                    }

                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), material);

                    uint64 new_occupied  = orig_occupied;

                    clear_set64(square_t::E7,
                                square_t::D8,
                                new_occupied);

                    EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                    uint64 new_xoccupied = orig_xoccupied;
                    clear_bit64(square_t::D8, new_xoccupied);

                    EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                    uint64 new_xpiece64 = orig_xpiece64;
                    clear_bit64(square_t::D8, new_xpiece64);

                    if (pieces[n] == piece_t::rook)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                    }

                    uint64 new_pawns = orig_pawns;

                    clear_bit64(square_t::E7, new_pawns);

                    EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                    EXPECT_EQ(position.get_turn(), player_t::black);

                    EXPECT_EQ(position.piece_on(square_t::E7),
                        piece_t::empty);
                    EXPECT_EQ(position.piece_on(square_t::D8),
                        pieces[i]);

                    EXPECT_TRUE(position.unmake_move(move));

                    EXPECT_TRUE(position.equals(copy, 0));
                }
            }
        }

        {
            /* Test pawn captures right and promotes */

            piece_t pieces[] = {piece_t::rook, piece_t::knight, piece_t::bishop,
                                piece_t::queen};

            char piece_str[]  = {'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 4; n++)
            {
                std::string init_fen = "3n1X2/4P3/8/2K5/8/8/8/4k3 w - - 0 1";
                init_fen[3] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                for (int i = 0 ; i < 4; i++)
                {
                    int32 move = pack_move(pieces[n],      // captured
                                           square_t::E7,   // from
                                           piece_t::pawn,  // moved
                                           pieces[i],      // promote
                                           square_t::F8);  // to

                    const uint64 orig_hash = position.get_hash_key();
                    const uint64 orig_occupied = position.get_occupied(player_t::white);
                    const uint64 orig_pawns = position.get_bitboard<piece_t::pawn>(player_t::white);
                    const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                    uint64 orig_xpiece64 = 0;

                    int material = pawn_value - knight_value;

                    if (pieces[n] == piece_t::rook)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                        material -= rook_value;
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                        material -= knight_value;
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                        material -= bishop_value;
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                        material -= queen_value;
                    }

                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), material);

                    EXPECT_EQ(position.get_turn(), player_t::white);

                    EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                    EXPECT_EQ(position.get_fullmove_number(), 1);

                    EXPECT_TRUE(position.make_move(move));

                    std::string fen_str = "3n1X2/8/8/2K5/8/8/8/4k3 b - - 0 1";
                    fen_str[3]          = piece_str[i];

                    EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                    EXPECT_EQ(position.get_fullmove_number(), 1);

                    EXPECT_NE(position.get_hash_key(), orig_hash);

                    if (pieces[i] == piece_t::rook)
                    {
                        material += rook_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::rook>(player_t::white),
                            Util::get_bit<uint64>(square_t::F8));
                    }
                    else if (pieces[i] == piece_t::knight)
                    {
                        material += knight_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::knight>(player_t::white),
                            Util::get_bit<uint64>(square_t::F8));
                    }
                    else if (pieces[i] == piece_t::bishop)
                    {
                        material += bishop_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::bishop>(player_t::white),
                            Util::get_bit<uint64>(square_t::F8));
                    }
                    else if (pieces[i] == piece_t::queen)
                    {
                        material += queen_value - pawn_value;
                        EXPECT_EQ(position.get_bitboard<piece_t::queen>(player_t::white),
                            Util::get_bit<uint64>(square_t::F8));
                    }

                    if (pieces[n] == piece_t::rook)
                    {
                        material += rook_value;
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        material += knight_value;
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        material += bishop_value;
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        material += queen_value;
                    }

                    EXPECT_EQ(position.get_material(player_t::white) -
                              position.get_material(player_t::black), material);

                    uint64 new_occupied  = orig_occupied;

                    clear_set64(square_t::E7,
                                square_t::F8,
                                new_occupied);

                    EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                    uint64 new_xoccupied = orig_xoccupied;
                    clear_bit64(square_t::F8, new_xoccupied);

                    EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                    uint64 new_xpiece64 = orig_xpiece64;
                    clear_bit64(square_t::F8, new_xpiece64);

                    if (pieces[n] == piece_t::rook)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::knight)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::bishop)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                    }
                    else if (pieces[n] == piece_t::queen)
                    {
                        EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                    }

                    uint64 new_pawns = orig_pawns;

                    clear_bit64(square_t::E7, new_pawns);

                    EXPECT_EQ(new_pawns, position.get_bitboard<piece_t::pawn>(player_t::white));

                    EXPECT_EQ(position.get_turn(), player_t::black);

                    EXPECT_EQ(position.piece_on(square_t::E7),
                        piece_t::empty);
                    EXPECT_EQ(position.piece_on(square_t::F8),
                        pieces[i]);

                    EXPECT_TRUE(position.unmake_move(move));

                    EXPECT_TRUE(position.equals(copy, 0));
                }
            }
        }
    }

    TEST(Position, make_unmake_rook)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        Position position(stream);

        {
            /* Test rook moves */

            EXPECT_TRUE(position.reset("4k3/2p5/8/8/2R5/8/8/4K3 w - - 0 1"));

            Position copy(position);

            ASSERT_EQ(position, copy);

            int32 move = pack_move(piece_t::empty, // captured
                                   square_t::C4,   // from
                                   piece_t::rook,  // moved
                                   piece_t::empty, // promote
                                   square_t::D4);  // to

            const uint64 orig_hash = position.get_hash_key();
            const uint64 orig_occupied = position.get_occupied(player_t::white);
            const uint64 orig_rooks = position.get_bitboard<piece_t::rook>(player_t::white);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), rook_value - pawn_value);

            EXPECT_EQ(position.get_turn(), player_t::white);

            EXPECT_STREQ(position.get_fen().c_str(),
                "4k3/2p5/8/8/2R5/8/8/4K3 w - - 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_TRUE(position.make_move(move));

            EXPECT_STREQ(position.get_fen().c_str(),
                "4k3/2p5/8/8/3R4/8/8/4K3 b - - 1 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_NE(position.get_hash_key(), orig_hash);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), rook_value - pawn_value);

            uint64 new_occupied = orig_occupied;

            clear_set64(square_t::C4,
                        square_t::D4,
                        new_occupied);

            EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

            uint64 new_rooks = orig_rooks;

            clear_set64(square_t::C4,
                        square_t::D4,
                        new_rooks);

            EXPECT_EQ(new_rooks, position.get_bitboard<piece_t::rook>(player_t::white));

            EXPECT_EQ(position.get_turn(), player_t::black);

            EXPECT_EQ(position.piece_on(square_t::C4),
                piece_t::empty);
            EXPECT_EQ(position.piece_on(square_t::D4),
                piece_t::rook);

            EXPECT_TRUE(position.unmake_move(move));

            EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test rook captures */

            piece_t pieces[] = {piece_t::pawn, piece_t::rook, piece_t::knight,
                                piece_t::bishop, piece_t::queen};

            char piece_str[]  = {'P', 'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 5; n++)
            {
                std::string init_fen = "4k3/2X5/8/8/2R5/8/8/4K3 w - - 0 1";
                init_fen[5] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(pieces[n],      // captured
                                       square_t::C4,   // from
                                       piece_t::rook,  // moved
                                       piece_t::empty, // promote
                                       square_t::C7);  // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_rooks = position.get_bitboard<piece_t::rook>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                uint64 orig_xpiece64 = 0;

                int material = rook_value;

                if (pieces[n] == piece_t::pawn)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::pawn>(player_t::black);
                    material -= pawn_value;
                }
                else if (pieces[n] == piece_t::rook)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                    material -= rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                    material -= knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                    material -= bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                    material -= queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "4k3/2R5/8/8/8/8/8/4K3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                if (pieces[n] == piece_t::pawn)
                {
                    material += pawn_value;
                }
                else if (pieces[n] == piece_t::rook)
                {
                    material += rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    material += knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    material += bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    material += queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::C4,
                            square_t::C7,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::C7, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpiece64 = orig_xpiece64;
                clear_bit64(square_t::C7, new_xpiece64);

                if (pieces[n] == piece_t::pawn)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::pawn>(player_t::black));
                }
                else if (pieces[n] == piece_t::pawn)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                }
                else if (pieces[n] == piece_t::knight)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                }
                else if (pieces[n] == piece_t::queen)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                }

                uint64 new_rooks = orig_rooks;

                clear_set64(square_t::C4,
                            square_t::C7,
                            new_rooks);

                EXPECT_EQ(new_rooks, position.get_bitboard<piece_t::rook>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::C4),
                    piece_t::empty);
                EXPECT_EQ(position.piece_on(square_t::C7),
                    piece_t::rook);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
            }
        }
    }

    TEST(Position, make_unmake_queen)
    {
        Handle<std::ostream>
            stream(new std::ostream(std::cout.rdbuf()));

        Position position(stream);

        {
            /* Test queen moves */

            EXPECT_TRUE(position.reset("4k3/2p5/8/8/2Q5/8/8/4K3 w - - 0 1"));

            Position copy(position);

            ASSERT_EQ(position, copy);

            int32 move = pack_move(piece_t::empty,  // captured
                                   square_t::C4,    // from
                                   piece_t::queen,  // moved
                                   piece_t::empty,  // promote
                                   square_t::D4);   // to

            const uint64 orig_hash = position.get_hash_key();
            const uint64 orig_occupied = position.get_occupied(player_t::white);
            const uint64 orig_queens = position.get_bitboard<piece_t::queen>(player_t::white);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), queen_value - pawn_value);

            EXPECT_EQ(position.get_turn(), player_t::white);

            EXPECT_STREQ(position.get_fen().c_str(),
                "4k3/2p5/8/8/2Q5/8/8/4K3 w - - 0 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_TRUE(position.make_move(move));

            EXPECT_STREQ(position.get_fen().c_str(),
                "4k3/2p5/8/8/3Q4/8/8/4K3 b - - 1 1");

            EXPECT_EQ(position.get_fullmove_number(), 1);

            EXPECT_NE(position.get_hash_key(), orig_hash);

            EXPECT_EQ(position.get_material(player_t::white) -
                      position.get_material(player_t::black), queen_value - pawn_value);

            uint64 new_occupied = orig_occupied;

            clear_set64(square_t::C4,
                        square_t::D4,
                        new_occupied);

            EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

            uint64 new_queens = orig_queens;

            clear_set64(square_t::C4,
                        square_t::D4,
                        new_queens);

            EXPECT_EQ(new_queens, position.get_bitboard<piece_t::queen>(player_t::white));

            EXPECT_EQ(position.get_turn(), player_t::black);

            EXPECT_EQ(position.piece_on(square_t::C4),
                piece_t::empty);
            EXPECT_EQ(position.piece_on(square_t::D4),
                piece_t::queen);

            EXPECT_TRUE(position.unmake_move(move));

            EXPECT_TRUE(position.equals(copy, 0));
        }

        {
            /* Test queen captures */

            piece_t pieces[] = {piece_t::pawn, piece_t::rook, piece_t::knight,
                                piece_t::bishop, piece_t::queen};

            char piece_str[]  = {'P', 'R', 'N', 'B', 'Q'};

            for (int n = 0 ; n < 5; n++)
            {
                std::string init_fen = "4k3/2X5/8/8/2Q5/8/8/4K3 w - - 0 1";
                init_fen[5] = Util::to_lower(piece_str[n]);

                EXPECT_TRUE(position.reset(init_fen));

                Position copy(position);

                ASSERT_EQ(position, copy);

                int32 move = pack_move(pieces[n],       // captured
                                       square_t::C4,    // from
                                       piece_t::queen,  // moved
                                       piece_t::empty,  // promote
                                       square_t::C7);   // to

                const uint64 orig_hash = position.get_hash_key();
                const uint64 orig_occupied = position.get_occupied(player_t::white);
                const uint64 orig_queens = position.get_bitboard<piece_t::queen>(player_t::white);
                const uint64 orig_xoccupied = position.get_occupied(player_t::black);

                uint64 orig_xpiece64 = 0;

                int material = queen_value;

                if (pieces[n] == piece_t::pawn)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::pawn>(player_t::black);
                    material -= pawn_value;
                }
                else if (pieces[n] == piece_t::rook)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::rook>(player_t::black);
                    material -= rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::knight>(player_t::black);
                    material -= knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::bishop>(player_t::black);
                    material -= bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    orig_xpiece64 = position.get_bitboard<piece_t::queen>(player_t::black);
                    material -= queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                EXPECT_EQ(position.get_turn(), player_t::white);

                EXPECT_STREQ(position.get_fen().c_str(), init_fen.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_TRUE(position.make_move(move));

                std::string fen_str = "4k3/2Q5/8/8/8/8/8/4K3 b - - 0 1";

                EXPECT_STREQ(position.get_fen().c_str(), fen_str.c_str());

                EXPECT_EQ(position.get_fullmove_number(), 1);

                EXPECT_NE(position.get_hash_key(), orig_hash);

                if (pieces[n] == piece_t::pawn)
                {
                    material += pawn_value;
                }
                else if (pieces[n] == piece_t::rook)
                {
                    material += rook_value;
                }
                else if (pieces[n] == piece_t::knight)
                {
                    material += knight_value;
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    material += bishop_value;
                }
                else if (pieces[n] == piece_t::queen)
                {
                    material += queen_value;
                }

                EXPECT_EQ(position.get_material(player_t::white) -
                          position.get_material(player_t::black), material);

                uint64 new_occupied  = orig_occupied;

                clear_set64(square_t::C4,
                            square_t::C7,
                            new_occupied);

                EXPECT_EQ(new_occupied, position.get_occupied(player_t::white));

                uint64 new_xoccupied = orig_xoccupied;
                clear_bit64(square_t::C7, new_xoccupied);

                EXPECT_EQ(new_xoccupied, position.get_occupied(player_t::black));

                uint64 new_xpiece64 = orig_xpiece64;
                clear_bit64(square_t::C7, new_xpiece64);

                if (pieces[n] == piece_t::pawn)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::pawn>(player_t::black));
                }
                else if (pieces[n] == piece_t::pawn)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::rook>(player_t::black));
                }
                else if (pieces[n] == piece_t::knight)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::knight>(player_t::black));
                }
                else if (pieces[n] == piece_t::bishop)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::bishop>(player_t::black));
                }
                else if (pieces[n] == piece_t::queen)
                {
                    EXPECT_EQ(new_xpiece64, position.get_bitboard<piece_t::queen>(player_t::black));
                }

                uint64 new_queens = orig_queens;

                clear_set64(square_t::C4,
                            square_t::C7,
                            new_queens);

                EXPECT_EQ(new_queens, position.get_bitboard<piece_t::queen>(player_t::white));

                EXPECT_EQ(position.get_turn(), player_t::black);

                EXPECT_EQ(position.piece_on(square_t::C4),
                    piece_t::empty);
                EXPECT_EQ(position.piece_on(square_t::C7),
                    piece_t::queen);

                EXPECT_TRUE(position.unmake_move(move));

                EXPECT_TRUE(position.equals(copy, 0));
            }
        }
    }
}
