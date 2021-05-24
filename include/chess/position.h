/**
 *  \file   position.h
 *  \author Jason Fernandez
 *  \date   07/03/2020
 */

#ifndef CHESS_POSITION_H_
#define CHESS_POSITION_H_

#include <array>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <string>

#include "chess/attacks.h"
#include "chess/chess.h"
#include "chess/data_tables.h"
#include "chess/util.h"

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
        kSizeOfRank,
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

    /** A simple aggregate holding pieces belonging to a single player */
    struct PieceSet {
        /** Default constructor */
        PieceSet() : king_square(), pieces64({0}) {
            king_square.fill(Square::Overflow);
        }

        bool operator==(const PieceSet& other) const noexcept;

        template <Piece piece>
        std::uint64_t Get()  noexcept(piece != Piece::EMPTY);

        template <Piece piece>
        void Put(Square sqr) noexcept(piece != Piece::EMPTY);

        /**
         * The location of the player's king is Piece::KING. The other
         * indexes exist simply to avoid branching on piece type
         */
        std::array<Square, 6> king_square;

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

        PlayerInfo(const PlayerInfo& info)            = default;
        PlayerInfo(PlayerInfo&& info)                 = default;
        PlayerInfo& operator=(const PlayerInfo& info) = default;
        PlayerInfo& operator=(PlayerInfo&& info)      = default;
        ~PlayerInfo()                                 = default;

        constexpr std::uint64_t AttacksTo(Square square) const noexcept;

        constexpr std::uint64_t Bishops() const noexcept;
        constexpr std::uint64_t King()    const noexcept;
        constexpr std::uint64_t Knights() const noexcept;
        constexpr std::uint64_t Pawns()   const noexcept;
        constexpr std::uint64_t Rooks()   const noexcept;
        constexpr std::uint64_t Queens()  const noexcept;

        constexpr bool  CanCastle()      const noexcept;
        constexpr bool  CanCastleLong()  const noexcept;
                  bool& CanCastleLong()  noexcept;
        constexpr bool  CanCastleShort() const noexcept;
                  bool& CanCastleShort() noexcept;

        template <Piece piece>
        void Drop(Square square) noexcept;
        void Drop(Piece piece, Square square) noexcept;

        template <Piece piece>
        void Lift(Square square) noexcept;
        void Lift(Piece piece, Square square) noexcept;

        template <Piece piece>
        void Move(Square from, Square to) noexcept;
        void Move(Piece piece, Square from, Square to) noexcept;

        constexpr Square        KingSquare() const noexcept;
        constexpr std::int16_t  Material()   const noexcept;
        constexpr std::uint64_t Occupied()   const noexcept;

        void InhibitCastle() noexcept;

        bool operator==(const PlayerInfo& other) const noexcept;

    private:
        /** True if this player can castle long */
        bool can_castle_long_;

        /** True if this player can castle short */
        bool can_castle_short_;

        /** The sum of this player's material */
        std::int16_t material_;

        /** The squares occupied by the player */
        std::uint64_t occupied_;

        /** This player's pieces */
        PieceSet pieces_;
    };

    /** The starting position */
    static constexpr char kDefaultFen[] =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Position();

    Position(const Position& position)            = default;
    Position(Position&& position)                 = default;
    Position& operator=(const Position& position) = default;
    Position& operator=(Position&& position)      = default;
    ~Position()                                   = default;

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

    template<Player player>
    void MakeMove(std::int32_t move, std::uint32_t ply) noexcept;

    constexpr std::uint64_t Occupied() const noexcept;

    template <Player player>
    constexpr bool OccupiedBy(Square square) const noexcept;

    constexpr Piece PieceOn(Square square) const noexcept;

    FenError Reset(const std::string& fen_ = kDefaultFen);

    constexpr Player ToMove() const noexcept;

    template<Player player>
    constexpr bool UnderAttack(Square square) const noexcept;

    template<Player player>
    void UnMakeMove(std::int32_t move, std::uint32_t ply) noexcept;

    bool operator==(const Position& other) const noexcept;

    static std::string ErrorToString(FenError error);
    static FenError Validate(const Position& pos);

private:
    /**
     * En-passant move information
     */
    struct EnPassantInfo {
        void clear();

        /**
         * The square(s) from which the capture can be made
         */
        Square from[2];

        /**
         * The capture destination square
         */
        Square target;
    };

    /**
     * Maintains a record of select information over multiple plies
     */
    struct History {
        /**
         * Stored long castle rights info
         */
        bool can_castle_long[2][kMaxPly];

        /**
         * Stored short castle rights info
         */
        bool can_castle_short[2][kMaxPly];

        /**
         * Stored en passant target
         */
        Square ep_target[kMaxPly];

        /**
         * Consecutive irreversible moves
         */
        int half_move_number[kMaxPly];
    };

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

    /** Position history across multiple plies */
    History history_;

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
 * @return The position's current half-move number
 */
constexpr int Position::HalfMoveNumber() const noexcept {
    return half_move_number_;
}

/**
 * @return True if the given player is currently in check
 */
template <Player player>
constexpr bool Position::InCheck() const noexcept {
    return UnderAttack<util::opponent<player>()>(
            GetPlayerInfo<player>().KingSquare());
}

/**
 * Make a move
 *
 * @param[in] move The move to make
 * @param[in] ply  The current search ply
 */
template<Player who>
inline void Position::MakeMove(std::int32_t move, std::uint32_t ply) noexcept {
    /*
     * Extract player/opponent info
     */
    auto& player = GetPlayerInfo<who>();
    auto& opponent = GetPlayerInfo<util::opponent<who>()>();

    history_.half_move_number[ply] = HalfMoveNumber();

    /*
     * Back up castling rights and en passant target. Later, when we
     * UnMakeMove(), we will have a record of what these were
     */
    history_.can_castle_long [util::index<Player::kBlack>()][ply] =
        black_.CanCastleLong();
    history_.can_castle_long [util::index<Player::kWhite>()][ply] =
        white_.CanCastleLong();

    history_.can_castle_short[util::index<Player::kBlack>()][ply] =
        black_.CanCastleShort();
    history_.can_castle_short[util::index<Player::kWhite>()][ply] =
        white_.CanCastleShort();

    bool castling_changed = false;

    history_.ep_target[ply] = en_passant_target_;

    /*
     * Extract move information
     */
    Piece captured = util::ExtractCaptured(move);
    Square from = util::ExtractFrom(move);
    Piece moved = util::ExtractMoved(move);
    Piece promoted = util::ExtractPromoted(move);
    Square to = util::ExtractTo(move);

    /*
     * Update common position information
     */
    pieces_[from] = Piece::EMPTY;

    /*
     * Clear the en passant info as it is no longer valid
     */
    en_passant_target_ = Square::Overflow;

    /*
     * Update the player-specific info for the player who moved
     */
    if (moved != Piece::PAWN) {
        pieces_[to] = moved;
        player.Move(moved, from, to);
    }

    switch (moved) {
      case Piece::PAWN:
        player.template Lift<Piece::PAWN>(from);

        /*
         * Note the promotion piece will be a pawn if this was
         * not actually a pawn promotion
         */
        pieces_[to] = promoted;
        player.Drop(promoted, to);

        /*
         * If this was a double advance, set the en passant target
         */
        if (std::abs(from - to) == 16) {
            en_passant_target_ = data_tables::kMinus8<who>[to];
        }
        break;
      case Piece::ROOK:
        if (player.CanCastleLong() && util::GetFile(from) == 7) {
            player.CanCastleLong() = false;
            castling_changed = true;
        } else if (player.CanCastleShort() && util::GetFile(from) == 0) {
            player.CanCastleShort() = false;
            castling_changed = true;
        }
        break;
      case Piece::KING:
        if (std::abs(from - to) == 2) {
            /*
             * This was a castling move - update the rook data
             */
            if (util::GetFile(to) == 1) {
                pieces_[to - 1] = Piece::EMPTY;
                pieces_[to + 1] = Piece::ROOK;

                player.template Move<Piece::ROOK>(to - 1, to + 1);
            } else {
                pieces_[to + 2] = Piece::EMPTY;
                pieces_[to - 1] = Piece::ROOK;

                player.template Move<Piece::ROOK>(to + 2, to - 1);
            }
        }

        if (player.CanCastle()) {
            /*
             * Clear all castling rights for this player
             */
            player.InhibitCastle();
            castling_changed = true;
        }
        break;
      default:
        break;
    }

    /*
     * Update opponent info if we captured a piece
     */
    if (captured != Piece::EMPTY) {
        switch (captured) {
          case Piece::PAWN:
            if (opponent.Occupied() & data_tables::kSetMask[to]) {
                opponent.template Lift<Piece::PAWN>(to);
            } else {
                const Square minus8 = data_tables::kMinus8<who>[to];
                pieces_[minus8] = Piece::EMPTY;
                opponent.template Lift<Piece::PAWN>(minus8);
            }
            break;
          case Piece::ROOK:
            opponent.template Lift<Piece::ROOK>(to);

            /*
             * Update the opponent's castling rights if he could have castled
             * with this rook
             */
            if (opponent.CanCastle()) {
                const int file = util::GetFile(to);
                if (file == 7) {
                    opponent.CanCastleLong() = false;
                } else if (file == 0) {
                    opponent.CanCastleShort() = false;
                }
            }
            break;
          default:
            opponent.Lift(captured, to);
            break;
        }
    } else if (moved != Piece::PAWN
                && !castling_changed) {
        half_move_number_++;
    }

    full_move_number_ =
        util::IncrementIfBlack<who>(full_move_number_);

    to_move_ = util::opponent<who>();
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
 * player (as if ready to capture on that square)
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
 * Undo a move
 *
 * @param[in] move The move to undo
 * @param[in] ply  The current search ply
 */
template<Player who> inline
void Position::UnMakeMove(std::int32_t move, std::uint32_t ply) noexcept {
    /*
     * Extract player/opponent info
     */
    auto& player = GetPlayerInfo<who>();
    auto& opponent = GetPlayerInfo<util::opponent<who>()>();

    half_move_number_ = history_.half_move_number[ply];

    /*
     * Restore castling rights and en passant target
     */
    black_.CanCastleLong() =
        history_.can_castle_long [util::index<Player::kBlack>()][ply];
    white_.CanCastleLong() =
        history_.can_castle_long [util::index<Player::kWhite>()][ply];

    black_.CanCastleShort() =
        history_.can_castle_short[util::index<Player::kBlack>()][ply];
    white_.CanCastleShort() =
        history_.can_castle_short[util::index<Player::kWhite>()][ply];

    en_passant_target_ = history_.ep_target[ply];

    /*
     * Extract move information
     */
    Piece captured = util::ExtractCaptured(move);
    Square from = util::ExtractFrom(move);
    Piece moved = util::ExtractMoved(move);
    Piece promoted = util::ExtractPromoted(move);
    Square to = util::ExtractTo(move);

    /*
     * Revert common position information
     */
    pieces_[from] = moved;
    pieces_[to] = captured;  // Will correct if en-passant

    /*
     * Revert the player-specific info for the player who moved
     */
    if (moved != Piece::PAWN) {
        player.Move(moved, to, from);
    }

    switch (moved) {
      case Piece::PAWN:
        player.template Drop<Piece::PAWN>(from);

        /*
         * Note the promotion piece will be a pawn if this was not actually
         * a pawn promotion
         */
        player.Lift(promoted, to);
        break;
      case Piece::KING:
        if (std::abs(from - to) == 2) {
            /*
             * This was a castling move - update the rook data
             */
            if (util::GetFile(to) == 1) {
                pieces_[to - 1] = Piece::ROOK;
                pieces_[to + 1] = Piece::EMPTY;

                player.template Move<Piece::ROOK>(to + 1, to - 1);
            } else {
                pieces_[to + 2] = Piece::ROOK;
                pieces_[to - 1] = Piece::EMPTY;

                player.template Move<Piece::ROOK>(to - 1, to + 2);
            }
        }
        break;
      default:
        break;
    }

    /*
     * Update opponent info if we captured a piece
     */
    if (captured != Piece::EMPTY) {
        switch (captured) {
          case Piece::PAWN:
            if (to != en_passant_target_) {
                opponent.template Drop<Piece::PAWN>(to);
            } else {
                const Square minus8 = data_tables::kMinus8<who>[to];
                pieces_[minus8] = Piece::PAWN;
                opponent.template Drop<Piece::PAWN>(minus8);
                pieces_[to] = Piece::EMPTY;
            }
            break;
          default:
            opponent.Drop(captured, to);
            break;
        }
    }

    full_move_number_ =
        util::DecrementIfBlack<who>(full_move_number_);

    to_move_ = who;
}

/**
 * Constructor
 */
template <Player player>
Position::PlayerInfo<player>::PlayerInfo() :
    can_castle_long_(false),
    can_castle_short_(false),
    material_(0),
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
 * @return True if the specified player can castle
 */
template <Player player>
constexpr bool Position::PlayerInfo<player>::CanCastle() const noexcept {
    return can_castle_long_ || can_castle_short_;
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

    material_ += data_tables::kPieceValue[piece];
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

    pieces_.king_square[piece] = square;

    material_ += data_tables::kPieceValue[piece];
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

    pieces_.pieces64[piece] &= data_tables::kClearMask[square];

    material_ -= data_tables::kPieceValue[piece];
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

    material_ -= data_tables::kPieceValue[piece];
}

/**
 * Move a piece from one square to another
 *
 * @note No checks are performed regarding the legality of removing the piece
 *       from this square
 *
 * @param[in] from The origin square of the piece
 * @param[in] to   The destination square
 */
template <Player player>
template <Piece piece>
void Position::PlayerInfo<player>::Move(Square from, Square to) noexcept {
    const std::uint64_t clear_set = data_tables::kSetMask[from] |
                                    data_tables::kSetMask[to];

    pieces_.pieces64[piece] ^= clear_set;
    occupied_ ^= clear_set;
}

/**
 * Move a piece from one square to another
 *
 * @note No checks are performed regarding the legality of removing the piece
 *       from this square
 *
 * @param[in] piece The piece to move
 * @param[in] from  The origin square of the piece
 * @param[in] to    The destination square
 */
template <Player player>
void Position::PlayerInfo<player>::Move(Piece piece, Square from, Square to)
    noexcept {
    const std::uint64_t clear_set = data_tables::kSetMask[from] |
                                    data_tables::kSetMask[to];

    pieces_.pieces64[piece] ^= clear_set;
    occupied_ ^= clear_set;
}

/**
 * @return The \ref Square on which this player's king is standing
 */
template <Player player>
constexpr Square Position::PlayerInfo<player>::KingSquare() const noexcept {
    return pieces_.king_square[Piece::KING];
}

/**
 * @return The sum of the values of all of this player's pieces
 */
template <Player player>
constexpr std::int16_t Position::PlayerInfo<player>::Material() const
    noexcept {
    return material_;
}

/**
 * Forbid this player from castling in the future
 */
template <Player player>
void Position::PlayerInfo<player>::InhibitCastle() noexcept {
    can_castle_short_ = can_castle_long_ = false;
}

/**
 * Compare this object to another
 *
 * @param[in] other The object to compare against
 *
 * @return True if the two are the same
 */
template <Player player>
bool Position::PlayerInfo<player>::operator==(const PlayerInfo& other) const
    noexcept {
    bool same = material_ == other.material_ &&
                occupied_ == other.occupied_ &&
                pieces_ == other.pieces_;

    same = same && can_castle_long_ == other.can_castle_long_ &&
                   can_castle_short_ == other.can_castle_short_;
    return same;
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
 * Get a bitboard of all of a particular piece owned by this player
 *
 * @{
 */
template <Piece piece>
std::uint64_t Position::PieceSet::Get() noexcept(piece != Piece::EMPTY) {
    throw std::logic_error(__func__);
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::QUEEN>() noexcept {
    return pieces64[Piece::QUEEN];
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::PAWN>() noexcept {
    return pieces64[Piece::PAWN];
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::ROOK>() noexcept {
    return pieces64[Piece::ROOK];
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::KNIGHT>() noexcept {
    return pieces64[Piece::KNIGHT];
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::BISHOP>() noexcept {
    return pieces64[Piece::BISHOP];
}
template <>
inline std::uint64_t Position::PieceSet::Get<Piece::KING>() noexcept {
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

    king_square[Piece::KING] = sqr;
}
/**
 * @}
 */

}  // namespace chess

#endif  // CHESS_POSITION_H_
