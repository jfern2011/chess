/**
 *  \file   position.h
 *  \author Jason Fernandez
 *  \date   07/03/2020
 */

#ifndef CHESS_POSITION_H_
#define CHESS_POSITION_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

#include "chess/attacks.h"
#include "chess/chess.h"
#include "chess/data_tables.h"

namespace chess {

/**
 * Represents a chess position
 */
class Position final {
public:
    /** FEN parsing error codes */
    enum class FenError {
        kNumberOfRanks,
        kInvalidCharacter,
        kNumberOfSquares,
        kFullMoveNumber,
        kHalfMoveClock,
        kEnPassantSquare,
        kCastlingRights,
        kInvalidColor,
        kMissingColor,
        kPawnsOnBackRank,
        kNumberOfKings,
        kKingCanBeCaptured,
        kWhiteMayNotCastle,
        kBlackMayNotCastle,
        kWhiteMayNotCastleLong,
        kBlackMayNotCastleLong,
        kWhiteMayNotCastleShort,
        kBlackMayNotCastleShort,
        kTooManyPawns,
        kTooManyRooks,
        kTooManyKnights,
        kTooManyBishops,
        kTooManyQueens,
        kSuccess
    };

    /** Pieces belonging to a single player */
    struct PieceSet {
        template <Piece piece>
        std::uint64_t& Get() noexcept(piece != Piece::EMPTY);

        template <Piece piece>
        void Put(Square sqr) noexcept(piece != Piece::EMPTY);

        /** The location of this player's king */
        Square king_square;

        /**
         * This player's pieces. Each index is a bitboard representing
         * all of a particular type of piece this player has
         */
        std::array<std::uint64_t, 6> pieces64;
    };

    /**
     * Position-related information for one player
     *
     * @tparam player Specifies which player
     */
    template <Player player>
    class PlayerInfo {
    public:
        PlayerInfo();

        PlayerInfo(const PlayerInfo& info) = default;
        PlayerInfo(PlayerInfo&& info) = default;
        PlayerInfo& operator=(const PlayerInfo& info) = default;
        PlayerInfo& operator=(PlayerInfo&& info) = default;
        ~PlayerInfo() = default;

        constexpr std::uint64_t AttacksTo(Square square) const noexcept;

        constexpr std::uint64_t Bishops() const noexcept;
        constexpr std::uint64_t King() const noexcept;
        constexpr std::uint64_t Knights() const noexcept;
        constexpr std::uint64_t Pawns() const noexcept;
        constexpr std::uint64_t Rooks() const noexcept;
        constexpr std::uint64_t Queens() const noexcept;

        constexpr bool CanCastleLong() const noexcept;
        bool& CanCastleLong() noexcept;
        constexpr bool CanCastleShort() const noexcept;
        bool& CanCastleShort() noexcept;

        template <Piece piece> void Drop(Square square) noexcept;
        template <Piece piece> void Lift(Square square) noexcept;

        void Drop(Piece piece, Square square) noexcept;
        void Lift(Piece piece, Square square) noexcept;

        constexpr Square KingSquare() const noexcept;

        constexpr std::uint64_t Occupied() const noexcept;

    private:
        /** True if this player can castle long */
        bool can_castle_long_;

        /** True if this player can castle short */
        bool can_castle_short_;

        /** The squares occupied by the player */
        std::uint64_t occupied_;

        /** This player's pieces */
        PieceSet pieces_;
    };

    /** The starting position */
    static constexpr char kDefaultFen[] =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Position();

    Position(const Position& position) = default;
    Position(Position&& position) = default;
    Position& operator=(const Position& position) = default;
    Position& operator=(Position&& position) = default;
    ~Position() = default;

    void Display(std::ostream& stream) const;

    constexpr Square EnPassantTarget() const;

    constexpr int FullMoveNumber() const noexcept;

    std::string GetFen() const;

    template <Player player>
    constexpr const PlayerInfo<player>&
        GetPlayerInfo() const noexcept(player != Player::kBoth);

    template <Player player>
    constexpr PlayerInfo<player>&
        GetPlayerInfo() noexcept(player != Player::kBoth);

    constexpr int HalfMoveNumber() const noexcept;

    template <Player player>
    constexpr bool InCheck() const noexcept;

    void MakeMove(std::uint32_t move) noexcept;

    constexpr std::uint64_t Occupied() const noexcept;

    template <Player player>
    constexpr bool OccupiedBy(Square square) const noexcept;

    constexpr Piece PieceOn(Square square) const noexcept;

    FenError Reset(const std::string& fen_ = kDefaultFen);

    constexpr Player ToMove() const noexcept;

    template<Player player>
    constexpr bool UnderAttack(Square square) const noexcept;

    void UnMakeMove(std::uint32_t move) noexcept;

    static std::string ErrorToString(FenError error);
    static FenError Validate(const Position& pos);

private:
    /** The side playing as Black */
    PlayerInfo<Player::kBlack> black_;

    /** The side playing as White */
    PlayerInfo<Player::kWhite> white_;

    /**
     * The destination square for a pawn to capture en passant (set
     * on each pawn double advancement)
     */
    Square en_passant_target_;
   
    /** The position's full move number */
    int full_move_number_;

    /** The position's half move number */
    int half_move_number_;

    /** All pieces currently on board (for both sides) */
    Piece pieces_[65];

    /** Whose turn it currently is */
    Player to_move_;
};

/**
 * @return The square from which a pawn can currently be captured
 * en passant (set for every pawn double advancement)
 */
constexpr Square Position::EnPassantTarget() const {
    return en_passant_target_;
}

/**
 * @return The position's current full-move number
 */
constexpr int Position::FullMoveNumber() const noexcept {
    return full_move_number_;
}

/**
 * @return The position's current half-move number
 */
constexpr int Position::HalfMoveNumber() const noexcept {
    return half_move_number_;
}

/**
 * @return A bitboard representing the squares occupied by both players
 */
constexpr std::uint64_t Position::Occupied() const noexcept {
    return white_.Occupied() | black_.Occupied();
}

/**
 * @param[in] square Check if this square is occupied
 *
 * @return True if \a square is occupied by the specified player
 */
template <Player player>
constexpr bool Position::OccupiedBy(Square square) const noexcept {
    return GetPlayerInfo<player>().Occupied() & (std::uint64_t(1) << square);
}

/**
 * Get the piece standing on the given square
 *
 * @param[in] square Get the piece on this square
 *
 * @return This piece on \a square
 */
constexpr Piece Position::PieceOn(Square square) const noexcept {
    return pieces_[square];
}

/**
 * @return Whose turn it is
 */
constexpr Player Position::ToMove() const noexcept {
    return to_move_;
}

/**
 * Check if the given square is being directly attacked by the specified
 * player, i.e. it could be moved to immediately
 *
 * @param[in] square The square of interest
 *
 * @return True if the player is attacking a\ square
 */
template<Player player>
constexpr bool Position::UnderAttack(Square square) const noexcept {
    const auto& info = GetPlayerInfo<player>();

    if (data_tables::kPawnAttacks<util::opponent<player>()>[square]
            & info.Pawns()) {
        return true;
    }

    if (data_tables::kKingAttacks[square] & info.King()) {
        return true;
    }

    if (data_tables::kKnightAttacks[square] & info.Knights()) {
        return true;
    }

    const std::uint64_t occupied = Occupied();

    if (AttacksFrom<Piece::ROOK>(square, occupied)
            & (info.Rooks() | info.Queens())) {
        return true;
    }

    if (AttacksFrom<Piece::BISHOP>(square, occupied)
            & (info.Bishops() | info.Queens())) {
        return true;
    }

    return false;
}

/**
 * Get the \ref PlayerInfo object for the specified player
 *
 * @{
 */
template <Player player>
constexpr auto Position::GetPlayerInfo()
    const noexcept(player != Player::kBoth) -> const PlayerInfo<player>& {
    throw std::logic_error(__func__);
}
template <>
constexpr auto Position::GetPlayerInfo<Player::kWhite>() const noexcept
    -> const PlayerInfo<Player::kWhite>& {
    return white_;
}
template <>
constexpr auto Position::GetPlayerInfo<Player::kBlack>() const noexcept
    -> const PlayerInfo<Player::kBlack>& {
    return black_;
}
/**
 * @}
 */

/**
 * Get the \ref PlayerInfo object for the specified player
 *
 * @{
 */
template <Player player>
constexpr auto Position::GetPlayerInfo()
    noexcept(player != Player::kBoth)
    -> PlayerInfo<player>& {
    throw std::logic_error(__func__);
}
template <>
constexpr auto Position::GetPlayerInfo<Player::kWhite>() noexcept
    -> PlayerInfo<Player::kWhite>& {
    return white_;
}
template <>
constexpr auto Position::GetPlayerInfo<Player::kBlack>() noexcept
    -> PlayerInfo<Player::kBlack>& {
    return black_;
}
/**
 * @}
 */

/**
 * @return True if the given player is currently in check
 */
template <Player player>
constexpr bool Position::InCheck() const noexcept {
    return UnderAttack<util::opponent<player>()>(
            GetPlayerInfo<player>().KingSquare());
}

/**
 * Constructor
 */
template <Player player>
Position::PlayerInfo<player>::PlayerInfo() :
    can_castle_long_(false),
    can_castle_short_(false),
    occupied_(0),
    pieces_() {
}

/**
 * Get the squares which have a piece attacking the given square
 *
 * @tparam player The player who is attacking
 *
 * @param[in] square The square being attacked
 *
 * @return The squares attacking \a square
 */
template <Player player>
constexpr std::uint64_t Position::PlayerInfo<player>::AttacksTo(Square square)
    const noexcept {
    std::uint64_t out = 0;

    out |= data_tables::kPawnAttacks<util::opponent<player>()>[square]
            & Pawns();

    out |= AttacksFrom<Piece::ROOK>(square, occupied_) & (Rooks() | Queens());

    out |= AttacksFrom<Piece::BISHOP>(square, occupied_)
            & (Bishops() | Queens());

    out |= data_tables::kKnightAttacks[square] & Knights();

    out |= data_tables::kKingAttacks[square] & King();

    return out;
}

/**
 * Drop a piece onto the given square
 *
 * @note No checks are performed regarding the legality of having the piece
 *       on this square
 *
 * @param[in] square The square onto which to drop the piece
 */
template <Player player>
template <Piece piece>
void Position::PlayerInfo<player>::Drop(Square square) noexcept {
    occupied_ |= std::uint64_t(1) << square;

    pieces_.Put<piece>(square);
}

/**
 * Lift (remove) a piece from the given square
 *
 * @note No checks are performed regarding the legality of removing the piece
 *       from this square
 *
 * @param[in] square The square from which to remove the piece
 */
template <Player player>
template <Piece piece>
void Position::PlayerInfo<player>::Lift(Square square) noexcept {
    occupied_ &= data_tables::kClearMask[square];

    pieces_.Get<piece>() &= data_tables::kClearMask[square];
}

/**
 * Drop a piece onto the given square
 *
 * @note No checks are performed regarding the legality of having the piece
 *       on this square
 *
 * @param[in] piece  The piece to drop
 * @param[in] square The square onto which to drop the piece
 */
template <Player player>
void Position::PlayerInfo<player>::Drop(Piece piece, Square square) noexcept {
    const auto mask = std::uint64_t(1) << square;

    pieces_.pieces64[piece] |= mask;
    occupied_               |= mask;
}

/**
 * Lift (remove) a piece from the given square
 *
 * @note No checks are performed regarding the legality of removing the piece
 *       from this square
 *
 * @param[in] piece  The piece to remove
 * @param[in] square The square from which to remove the piece
 */
template <Player player>
void Position::PlayerInfo<player>::Lift(Piece piece, Square square) noexcept {
    occupied_ &= data_tables::kClearMask[square];

    pieces_.pieces64[piece] &= data_tables::kClearMask[square];
}

/**
 * @return A bitboard representing all bishops belonging to the given player
 */
template <Player player>
constexpr
std::uint64_t Position::PlayerInfo<player>::Bishops() const noexcept {
    return pieces_.pieces64[Piece::BISHOP];
}

/**
 * @return A bitboard representing the specified player's king
 */
template <Player player>
constexpr std::uint64_t Position::PlayerInfo<player>::King() const noexcept {
    return pieces_.pieces64[Piece::KING];
}

/**
 * @return A bitboard representing all knights belonging to the given player
 */
template <Player player>
constexpr
std::uint64_t Position::PlayerInfo<player>::Knights() const noexcept {
    return pieces_.pieces64[Piece::KNIGHT];
}

/**
 * @return A bitboard representing all pawns belonging to the specified player
 */
template <Player player>
constexpr std::uint64_t Position::PlayerInfo<player>::Pawns() const noexcept {
    return pieces_.pieces64[Piece::PAWN];
}

/**
 * @return A bitboard representing all rooks belonging to the specified player
 */
template <Player player>
constexpr std::uint64_t Position::PlayerInfo<player>::Rooks() const noexcept {
    return pieces_.pieces64[Piece::ROOK];
}

/**
 * @return A bitboard representing all queens belonging to the specified player
 */
template <Player player>
constexpr std::uint64_t Position::PlayerInfo<player>::Queens() const noexcept {
    return pieces_.pieces64[Piece::QUEEN];
}

/**
 * @return A bitboard representing all squares occupied by the specified player
 */
template <Player player>
constexpr
std::uint64_t Position::PlayerInfo<player>::Occupied() const noexcept {
    return occupied_;
}

/**
 * @return True if the specified player can castle long
 */
template <Player player>
constexpr bool Position::PlayerInfo<player>::CanCastleLong() const noexcept {
    return can_castle_long_;
}

/**
 * Set this player's internal castling rights
 *
 * @return A reference to the player's internal can_castle_long_ flag
 */
template <Player player>
bool& Position::PlayerInfo<player>::CanCastleLong() noexcept {
    return can_castle_long_;
}

/**
 * @return True if the specified player can castle short
 */
template <Player player>
constexpr bool Position::PlayerInfo<player>::CanCastleShort() const noexcept {
    return can_castle_short_;
}

/**
 * Set this player's internal castling rights
 *
 * @return A reference to the player's internal can_castle_short_ flag
 */
template <Player player>
bool& Position::PlayerInfo<player>::CanCastleShort() noexcept {
    return can_castle_short_;
}

/**
 * @return The \ref Square on which this player's king is standing
 */
template <Player player>
constexpr Square Position::PlayerInfo<player>::KingSquare() const noexcept {
    return pieces_.king_square;
}

/**
 * Get a bitboard of all of a particular piece owned by this player
 *
 * @{
 */
template <Piece piece>
std::uint64_t& Position::PieceSet::Get() noexcept(piece != Piece::EMPTY) {
    throw std::logic_error(__func__);
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::QUEEN>() noexcept {
    return pieces64[Piece::QUEEN];
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::PAWN>() noexcept {
    return pieces64[Piece::PAWN];
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::ROOK>() noexcept {
    return pieces64[Piece::ROOK];
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::KNIGHT>() noexcept {
    return pieces64[Piece::KNIGHT];
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::BISHOP>() noexcept {
    return pieces64[Piece::BISHOP];
}
template <>
inline std::uint64_t& Position::PieceSet::Get<Piece::KING>() noexcept {
    return pieces64[Piece::KING];
}
/**
 * @}
 */

/**
 * Put a particular piece on the specified square for this player
 *
 * @{
 */
template <Piece piece>
void Position::PieceSet::Put(Square sqr) noexcept(piece != Piece::EMPTY) {
    throw std::logic_error(__func__);
}
template <>
inline void Position::PieceSet::Put<Piece::QUEEN>(Square sqr) noexcept {
    pieces64[Piece::QUEEN] |= std::uint64_t(1) << sqr;
}
template <>
inline void Position::PieceSet::Put<Piece::PAWN>(Square sqr) noexcept {
    pieces64[Piece::PAWN] |= std::uint64_t(1) << sqr;
}
template <>
inline void Position::PieceSet::Put<Piece::ROOK>(Square sqr) noexcept {
    pieces64[Piece::ROOK] |= std::uint64_t(1) << sqr;
}
template <>
inline void Position::PieceSet::Put<Piece::KNIGHT>(Square sqr) noexcept {
    pieces64[Piece::KNIGHT] |= std::uint64_t(1) << sqr;
}
template <>
inline void Position::PieceSet::Put<Piece::BISHOP>(Square sqr) noexcept {
    pieces64[Piece::BISHOP] |= std::uint64_t(1) << sqr;
}
template <>
inline void Position::PieceSet::Put<Piece::KING>(Square sqr) noexcept {
    pieces64[Piece::KING] |= std::uint64_t(1) << sqr;

    king_square = sqr;
}
/**
 * @}
 */

}  // namespace chess

#endif  // CHESS_POSITION_H_
