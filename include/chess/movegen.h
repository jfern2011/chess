/**
 *  \file   movegen.h
 *  \author Jason Fernandez
 *  \date   01/09/2022
 */

#ifndef CHESS_MOVEGEN_H_
#define CHESS_MOVEGEN_H_

#include <array>
#include <cstddef>
#include <cstdint>

#include "chess/attacks.h"
#include "chess/position.h"
#include "chess/util.h"

namespace chess {
namespace detail {
/**
 * Check if it is safe to move the king to the specified square (i.e. moving
 * here will not put the king in check)
 *
 * @tparam P The player whose king to test for safety
 *
 * @param[in] pos    The current position
 * @param[in] square The square being considered for the king
 *
 * @return True if moving to \a square is safe
 */
template <Player P>
constexpr bool SafeForKing(const Position& pos, Square square) noexcept {
    const auto& info = pos.GetPlayerInfo<P>();
    const auto& opponent = pos.GetPlayerInfo<util::opponent<P>()>();

    if (data_tables::kPawnAttacks<P>[square] & opponent.Pawns()) {
        return false;
    }

    if (data_tables::kKingAttacks[square] & opponent.King()) {
        return false;
    }

    if (data_tables::kKnightAttacks[square] & opponent.Knights()) {
        return false;
    }

    // If the king is in check by a sliding piece, make sure we're not
    // attempting to move it along the direction of the check

    const std::uint64_t occupied = pos.Occupied() ^ info.King();

    if (AttacksFrom<Piece::ROOK>(square, occupied)
            & (opponent.Rooks() | opponent.Queens())) {
        return false;
    } else if (AttacksFrom<Piece::BISHOP>(square, occupied)
            & (opponent.Bishops() | opponent.Queens())) {
        return false;
    }

    return true;
}

}  // namespace detail

/**
 * @brief Generate pawn advances for the given player (both 1 and 2 steps)
 *
 * @note Does not handle promotions
 *
 * @tparam P The player to generate moves for
 *
 * @param[in] pos     The position from which to generate moves
 * @param[in] target  Limit moves to these squares
 * @param[in] pinned  The set of pieces pinned on the king
 * @param[out] moves  The set of legal moves
 *
 * @return The number of moves generated
 */
inline template <Player P>
std::size_t GeneratePawnAdvances(const Position& pos,
                                 std::uint64_t target,
                                 std::uint64_t pinned,
                                 std::uint32_t* moves) noexcept {
    const auto& info = pos.GetPlayerInfo<P>();
    const Square king_square = info.KingSquare();

    const std::uint64_t vacant = ~pos.Occupied();

    constexpr std::uint64_t kBackRank =
        data_tables::kBackRank<util::opponent<P>()>;
    constexpr std::uint64_t k3rdRank = data_tables::k3rdRank<P>;

    std::uint64_t advances1 =
        util::AdvancePawns1<P>(pawns) & (~kBackRank) & vacant & target;
    std::uint64_t advances2 =
        util::AdvancePawns1<P>(advances1) & k3rdRank & vacant & target;

    std::size_t n_moves = 0;

    while (advances1) {
        const std::int8_t to = util::Msb(advances1);

        const auto from = static_cast<int>(data_tables::kMinus8<P>[to]);

        const bool immovable =
            (pinned & data_tables::kSetMask[from]) &&
                data_tables::kDirections[king_square][from] !=
                    Direction::kAlongFile;

        if (!immovable) {
            moves[n_moves++] = util::PackMove(
                Piece::EMPTY, from, Piece::PAWN, Piece::EMPTY, to);
        } else {
            advances2 &= data_tables::kClearMask[from];
        }

        advances1 &= data_tables::kClearMask[to];
    }

    while (advances2) {
        const std::int8_t to = util::Msb(advances2);

        const auto from = static_cast<int>(data_tables::kMinus16<P>[to]);

        moves[n_moves++] = util::PackMove(
            Piece::EMPTY, from, Piece::PAWN, Piece::EMPTY, to);

        advances2 &= data_tables::kClearMask[to];
    }

    return n_moves;
}

/**
 * @brief Generate pawn captures and promotions for the given player
 *
 * @tparam P The player to generate moves for
 *
 * @param[in] pos     The position from which to generate moves
 * @param[in] target  Limit moves to these squares
 * @param[in] pinned  The set of pieces pinned on the king
 * @param[out] moves  The set of legal moves
 *
 * @return The number of moves generated
 */
inline template <Player P>
std::size_t GeneratePawnCaptures(const Position& pos,
                                 std::uint64_t target,
                                 std::uint64_t pinned,
                                 std::uint32_t* moves) noexcept {
    return 0;
}

/**
 * @brief Generate moves for the given player
 *
 * @tparam P The player to generate moves for
 *
 * @param[in] pos     The position from which to generate moves
 * @param[in] target  The target squares to move to
 * @param[in] pinned  The set of pieces pinned on the king
 * @param[out] moves  The set of legal moves
 *
 * @return The number of moves generated
 */
template <Player P>
std::size_t GenerateMoves(const Position& pos,
                          std::uint64_t target,
                          std::uint64_t pinned,
                          std::uint32_t* moves) noexcept {
    std::size_t n_moves = 0;

    const std::uint64_t occupied = pos.Occupied();

    const auto& info = pos.GetPlayerInfo<P>();

    const Square king_square = info.KingSquare();

    auto& generated = *moves;

    /*
     * Generate knight moves
     */
    for (std::uint64_t knights = info.Knights() & (~pinned); knights; ) {
        const std::int8_t from = util::Msb(knights);

        std::uint64_t attacks = data_tables::kKnightAttacks[from] & target;
        while (attacks) {
            const auto to = static_cast<Square>(util::Msb(attacks));

            generated[n_moves++] = util::PackMove(
                pos.PieceOn(to), from, Piece::KNIGHT, Piece::EMPTY, to);

            attacks &= data_tables::kClearMask[to];
        }

        knights &= data_tables::kClearMask[from];
    }

    /**
     * Generate rook moves
     */
    for (std::uint64_t rooks = info.Rooks(); rooks; ) {
        const std::int8_t from = util::Msb(rooks);

        /*
         * If this rook is pinned along a diagonal then we can't move it, so
         * don't bother generating an attacks_from bitboard. If pinned along a
         * rank, then clear the file bits of its attacks_from bitboard to
         * ensure we only keep moves along the direction of the pin (same goes
         * for when pinned along a file):
         */
        std::uint64_t restrict_attacks = ~0;

        if (data_tables::kSetMask[from] & pinned) {
            const Direction dir = data_tables::kDirections[from][king_square];

            if (dir == Direction::kAlongA1H8 || dir == Direction::kAlongH1A8) {
                rooks &= data_tables::kClearMask[from];
                continue;
            } else if (dir == Direction::kAlongRank) {
                restrict_attacks = data_tables::kRanks64[from];
            } else {
                restrict_attacks = data_tables::kFiles64[from];
            }
        }

        std::uint64_t attacks = AttacksFrom<Piece::ROOK>(from, occupied)
                                & target & restrict_attacks;
        while (attacks) {
            const auto to = static_cast<Square>(util::Msb(attacks));

            generated[n_moves++] = util::PackMove(
                pos.PieceOn(to), from, Piece::ROOK, Piece::EMPTY, to);

            attacks &= data_tables::kClearMask[to];
        }

        rooks &= data_tables::kClearMask[from];
    }

    /**
     * Generate bishop moves
     */
    for (std::uint64_t bishops = info.Bishops(); bishops; ) {
        const std::int8_t from = util::Msb(bishops);

        /*
         * If this bishop is pinned along a file or rank then we can't move it,
         * so don't bother generating an attacks_from bitboard. If pinned along
         * an a1-h8 diagonal, then clear the h1_a8 bits of its attacks_from
         * bitboard to ensure we only keep moves along the direction of the pin
         * (similar idea for when pinned along an h1-a8 diagonal)
         */
        std::uint64_t restrict_attacks = ~0;

        if (data_tables::kSetMask[from] & pinned) {
            const Direction dir = data_tables::kDirections[from][king_square];

            if (dir == Direction::kAlongFile || dir == Direction::kAlongRank) {
                bishops &= data_tables::kClearMask[from];
                continue;
            } else if (dir == Direction::kAlongA1H8) {
                restrict_attacks = data_tables::kA1H8_64[from];
            } else {
                restrict_attacks = data_tables::kH1A8_64[from];
            }
        }

        std::uint64_t attacks = AttacksFrom<Piece::BISHOP>(from, occupied)
                                & target & restrict_attacks;
        while (attacks) {
            const auto to = static_cast<Square>(util::Msb(attacks));

            generated[n_moves++] = util::PackMove(
                pos.PieceOn(to), from, Piece::BISHOP, Piece::EMPTY, to);

            attacks &= data_tables::kClearMask[to];
        }

        bishops &= data_tables::kClearMask[from];
    }

    /**
     * Generate queen moves
     */
    for (std::uint64_t queens = info.Queens(); queens; ) {
        const std::int8_t from = util::Msb(queens);

        /*
         * If the queen is pinned, restrict its movement to along the
         * direction of the pin
         */
        std::uint64_t restrict_attacks = ~0;

        if (data_tables::kSetMask[from] & pinned) {
            const Direction dir = data_tables::kDirections[from][king_square];

            if (dir == Direction::kAlongH1A8) {
                restrict_attacks = data_tables::kH1A8_64[from];
            } else if (dir == Direction::kAlongA1H8) {
                restrict_attacks = data_tables::kA1H8_64[from];
            } else if (dir == Direction::kAlongRank) {
                restrict_attacks = data_tables::kRanks64[from];
            } else {
                restrict_attacks = data_tables::kFiles64[from];
            }
        }

        std::uint64_t attacks = AttacksFrom<Piece::QUEEN>(from, occupied)
                                & target & restrict_attacks;
        while (attacks) {
            const auto to = static_cast<Square>(util::Msb(attacks));

            generated[n_moves++] = util::PackMove(
                pos.PieceOn(to), from, Piece::QUEEN, Piece::EMPTY, to);

            attacks &= data_tables::kClearMask[to];
        }

        queens &= data_tables::kClearMask[from];
    }

    return n_moves;
}

/**
 * @brief Generate moves for the king
 *
 * @tparam P The player to generate moves for
 *
 * @param[in] pos     The position from which to generate moves
 * @param[in] target  The target squares to move to
 * @param[out] moves  The set of legal moves
 *
 * @return The number of moves generated
 */
inline template <Player P> std::size_t GenerateKingMoves(
    const Position& pos,
    std::uint64_t target,
    std::array<std::uint32_t, 256>* moves) noexcept {
    std::size_t n_moves = 0;

    const Square king_square = pos.GetPlayerInfo<P>().KingSquare();

    auto& generated = *moves;

    std::uint64_t attacks = data_tables::kKingAttacks[king_square] & target;
    while (attacks) {
        const std::int8_t to = util::Msb(attacks);

        if (SafeForKing<P>(pos, to)) {
            generated[n_moves++] = util::PackMove(
                pos.PieceOn(to), from, Piece::KING, Piece::EMPTY, to);
        }

        attacks &= data_tables::kClearMask[to];
    }

    return n_moves;
}

template <Player player>
void GenerateCaptures(const Position& pos,
                      std::array<std::uint32_t, 256>* moves) noexcept;

template <Player player>
std::size_t GenerateCheckEvasions(
    const Position& pos, std::array<std::uint32_t, 256>* moves) noexcept {
    const std::uint64_t occupied = pos.Occupied();

    const auto& info = pos.GetPlayerInfo<P>();
    const auto& opponent = pos.GetPlayerInfo<util::opponent<P>()>();

    const Square king_square = info.KingSquare();

    auto& generated = *moves;

    /*
     * Step 1: Gather all squares which contain an enemy piece attacking
     *         our king
     */
    const std::uint64_t attackers = opponent.AttacksTo(king_square);

    /*
     * Step 2: Generate king moves that get out of check
     */
    std::size_t n_moves = GenerateKingMoves<P>(pos, ~info.Occupied(), moves);

    /*
     * Step 3a: If the king is attacked twice, we are done
     */
    if (attackers & (attackers-1))
        return n_moves;
    
    /*
     * Step 3b: Otherwise, (1) get the square of the attacking piece and (2)
     *          a bitboard connecting the king square and the attacking piece
     *          to generate interposing moves from
     */
    const std::int8_t attacker = util::Msb(attackers);
    const std::uint64_t target =
        data_tables::kRaySegment[king_square][attacker];

    /*
     * Step 4: Generate knight, rook, bishop, and queen interposing moves
     */
    
    const std::uint64_t pinned = pos.PinnedPieces<P>();

    n_moves += GenerateMoves<P>(pos, target, pinned, &generated[n_moves]);

    /*
     * Step 5: Generate pawn moves
     */
}
	
template <Player player>
void GenerateChecks(const Position& pos,
                    std::array<std::uint32_t, 256>* moves) noexcept;

template <Player player>
void GenerateLegalMoves(const Position& pos,
                        std::array<std::uint32_t, 256>* moves) noexcept;

template <Player player>
void GenerateNonCaptures(const Position& pos,
                         std::array<std::uint32_t, 256>* moves) noexcept;

template <Player player>
bool ValidateMove(const Position& pos, std::uint32_t move) noexcept;

}  // namespace chess

#endif  // CHESS_MOVEGEN_H_
