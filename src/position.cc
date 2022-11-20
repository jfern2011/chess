/**
 *  \file   position.cc
 *  \author Jason Fernandez
 *  \date   07/04/2020
 */

#include "chess/position.h"

#include <cctype>
#include <cstddef>
#include <map>

#include "bitops/bitops.h"
#include "superstring/superstring.h"
#include "chess/util.h"

namespace chess {

/** The starting position */
const char Position::kDefaultFen[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

/** Mapping from FEN error code to description */
static const std::map<Position::FenError, std::string> kFenErrorToString {
    { Position::FenError::kNumberOfRanks,
        "Number of ranks must be 8"            },
    { Position::FenError::kInvalidCharacter,
        "Invalid character in sequence"        },
    { Position::FenError::kSizeOfRank,
        "Number of squares per rank must be 8" },
    { Position::FenError::kFullMoveNumber,
        "Minimum fullmove number is 1"         },
    { Position::FenError::kHalfMoveClock,
        "Minimum halfmove clock is 0"          },
    { Position::FenError::kEnPassantSquare,
        "Invalid en passant target square"     },
    { Position::FenError::kCastlingRights,
        "Unrecognized castling specification"  },
    { Position::FenError::kInvalidColor,
        "Invalid color"                        },
    { Position::FenError::kMissingColor,
        "Player 'w' or 'b' must be specified"  },
    { Position::FenError::kPawnsOnBackRank,
        "Back rank pawns are not allowed"      },
    { Position::FenError::kNumberOfKings,
        "Expected one king per side"           },
    { Position::FenError::kKingCanBeCaptured,
        "Player in check is not on move"       },
    { Position::FenError::kWhiteMayNotCastle,
        "White may not castle"                 },
    { Position::FenError::kBlackMayNotCastle,
        "Black may not castle"                 },
    { Position::FenError::kWhiteMayNotCastleLong,
        "White may not castle long"            },
    { Position::FenError::kBlackMayNotCastleLong,
        "Black may not castle long"            },
    { Position::FenError::kWhiteMayNotCastleShort,
        "White may not castle short"           },
    { Position::FenError::kBlackMayNotCastleShort,
        "Black may not castle short"           },
    { Position::FenError::kTooManyPawns,
        "Player has too many pawns"            },
    { Position::FenError::kTooManyRooks,
        "Player has too many rooks"            },
    { Position::FenError::kTooManyKnights,
        "Player has too many knights"          },
    { Position::FenError::kTooManyBishops,
        "Player has too many bishops"          },
    { Position::FenError::kTooManyQueens,
        "Player has too many queens"           },
    { Position::FenError::kSuccess,
        "Position is OK"                       }
};

/**
 * Compare this object to another
 *
 * @param[in] other The object to compare against
 *
 * @return True if the two are the same
 */
bool Position::PieceSet::operator==(const PieceSet& other) const noexcept {
    return pieces64 == other.pieces64 &&
            king_square[Piece::KING] == other.king_square[Piece::KING];
}

/**
 * Constructor
 */
Position::Position() :
    black_(),
    white_(),
    en_passant_target_(Square::Overflow),
    full_move_number_(0),
    half_move_number_(0),
    history_(),
    pieces_(),
    to_move_(Player::kBoth) {
}

/**
 * Display the current position
 *
 * @param[in] stream Write to this output stream
 */
void Position::Display(std::ostream& stream) const {
    int prev_rank = 8;
    const std::uint64_t one = 1;

    for (int sq = 63; sq >= -1; sq--) {
        if (util::GetRank(sq) != prev_rank) {
            stream << "\n ---+---+---+---+---+---+---+--- \n";
            if (sq == -1) break;

            prev_rank = util::GetRank(sq);
        }

        const bool black_occupies =
            OccupiedBy<Player::kBlack>(static_cast<Square>(sq));

        stream << "| "
               << util::PieceToChar(pieces_[sq], black_occupies)
               << " ";

        if (sq % 8 == 0) stream << "|";
    }

    stream << std::endl;
}

/**
 * Get the current position as a FEN string
 *
 * @return The current FEN-encoded position
 */
std::string Position::GetFen() const {
    std::string fen = "";
    int empty_count = 0;

    for (Square square = Square::A8; square >= Square::H1; square--) {
        if (pieces_[square] != Piece::EMPTY) {
            const bool is_white =
                OccupiedBy<Player::kWhite>(square);

            if (empty_count != 0) {
                fen += std::to_string(empty_count);
                empty_count = 0;
            }
            switch (pieces_[square]) {
              case Piece::PAWN:
                fen += is_white ? "P" : "p";
                break;
              case Piece::KNIGHT:
                fen += is_white ? "N" : "n";
                break;
              case Piece::BISHOP:
                fen += is_white ? "B" : "b";
                break;
              case Piece::ROOK:
                fen += is_white ? "R" : "r";
                break;
              case Piece::QUEEN:
                fen += is_white ? "Q" : "q";
                break;
              default:
                fen += is_white ? "K" : "k";
                break;
            }
        } else {
            empty_count++;
        }

        // Time to start the next rank?
        if (square % 8 == 0) {
            if (empty_count != 0) {
                fen += std::to_string(empty_count);
                empty_count = 0;
            }

            if (square != Square::H1) {
                fen += "/";
            }
        }
    }

    fen += to_move_ == Player::kWhite ? " w " : " b ";

    bool can_castle_any = false;

    if (white_.CanCastleShort()) {
        can_castle_any = true;
        fen += "K";
    }

    if (white_.CanCastleLong()) {
        can_castle_any = true;
        fen += "Q";
    }

    if (black_.CanCastleShort()) {
        can_castle_any = true;
        fen += "k";
    }

    if (black_.CanCastleLong()) {
        can_castle_any = true;
        fen += "q";
    }

    if (!can_castle_any) {
        fen += "-";
    }

    fen += " ";
    fen += en_passant_target_ != Square::Overflow ?
        kSquareStr[en_passant_target_] : "-";
    
    fen += " " + std::to_string(half_move_number_) +
           " " + std::to_string(full_move_number_);

    return fen;
}

/**
 * Reset this position
 *
 * @param[in] fen_ Encodes the position to set up
 *
 * @return One of the \ref FenError codes
 */
auto Position::Reset(const std::string& fen_) -> FenError {
    Position pos;

    jfern::superstring fen(fen_);

    auto tokens = fen.split("/");
    if (tokens.size() != 8) {
        return FenError::kNumberOfRanks;
    }

    auto square = 63;
    for (std::size_t i = 0; i < tokens.size(); i++) {
        int squares_on_rank = 0;
        for (std::size_t j = 0; j < tokens[i].size(); j++) {
            const char c = tokens[i][j];
            const Piece piece = util::CharToPiece(c);
            if (piece != Piece::EMPTY) {
                pos.pieces_[square] = piece;
                const auto squareE = static_cast<Square>(square);
                if (std::tolower(c) == c) {
                    pos.GetPlayerInfo<Player::kBlack>().Drop(piece, squareE);
                } else {
                    pos.GetPlayerInfo<Player::kWhite>().Drop(piece, squareE);
                }

                squares_on_rank++;
                square--;
            } else if (std::isdigit(c)) {
                const long n_squares = std::stol(std::string(&c,1));
                squares_on_rank += n_squares;
                for (long i = 1; i <= n_squares; i++) {
                    pos.pieces_[square--] = piece;
                }
            } else if (std::isspace(c)) {
                break;  // Move onto whose turn
            } else {
                return FenError::kInvalidCharacter;
            }
        }

        // Verify 8 squares accounted for between consecutive forward
        // slashes
        if (squares_on_rank != 8) {
            return FenError::kSizeOfRank;
        }
    }

    tokens = jfern::superstring(tokens.back()).split();

    pos.full_move_number_ = 1;
    pos.half_move_number_ = 0;

    switch (tokens.size()) {
      default:
        // Ignore anything beyond the 6th token instead of
        // returning an error
      case 6u:
        try {
            pos.full_move_number_ = std::stol(tokens[5]);
        } catch (...) {
            return FenError::kFullMoveNumber;
        }
        if (pos.full_move_number_ < 1) {
            return FenError::kFullMoveNumber;
        }
      case 5u:
        try {
            pos.half_move_number_ = std::stol(tokens[4]);
        } catch (...) {
            return FenError::kHalfMoveClock;
        }
        if (pos.half_move_number_ < 0) {
            return FenError::kHalfMoveClock;
        }
      case 4u:
        if (tokens[3] != "-") {
            const std::string ep_target =
                jfern::superstring(tokens[3]).to_lower();

            pos.en_passant_target_ = util::StrToSquare(ep_target);

            if (pos.en_passant_target_ == Square::Overflow) {
                return FenError::kEnPassantSquare;
            }
        }
      case 3u:
        if (tokens[2] != "-") {
            for (std::size_t i = 0; i < tokens[2].size(); i++) {
                switch (tokens[2][i]) {
                  case 'K':
                    pos.white_.CanCastleShort() = true; break;
                  case 'Q':
                    pos.white_.CanCastleLong() = true; break;
                  case 'k':
                    pos.black_.CanCastleShort() = true; break;
                  case 'q':
                    pos.black_.CanCastleLong() = true; break;
                  default:
                    return FenError::kCastlingRights;
                }
            }
        }
      case 2u: {
        const std::string color =
            jfern::superstring(tokens[1]).to_lower();

        if (color != "w" && color != "b") {
            return FenError::kInvalidColor;
        } else {
            pos.to_move_ =
                color == "w" ? Player::kWhite : Player::kBlack;
        }  // case 2u
        break;
      }
      case 1u:
        return FenError::kMissingColor;
    }

    const auto error_code = Validate(pos);
    if (error_code == FenError::kSuccess) {
        *this = std::move(pos);
    }

    return error_code;
}

/**
 * Compare this object to another
 *
 * @param[in] other The object to compare against
 *
 * @return True if the two are the same
 */
bool Position::operator==(const Position& other) const noexcept {
    bool same =
        black_ == other.black_ &&
        white_ == other.white_ &&
        full_move_number_ == other.full_move_number_ &&
        half_move_number_ == other.half_move_number_ &&
        to_move_ == other.to_move_;

    same = same && en_passant_target_ == other.en_passant_target_;

    for (int i = 0; i < 65; i++) {
        same = same && pieces_[i] == other.pieces_[i];
    }

    return same;
}

/**
 *
 * @param[in] error A \ref FenError code
 *
 * @return The 
 */
std::string Position::ErrorToString(FenError error) {
    return kFenErrorToString.at(error);
}

/**
 * Validate a position. The following rules are checked against:
 *
 * 1. No pawns on the 1st or 8th ranks
 * 2. Only two kings on board
 * 3. Side to move cannot capture the opposing king
 * 4. Castling rights make sense (e.g. if the king is not on its home square,
 *    then castling is not possible)
 * 5. En passant target makes sense (e.g. there must be a pawn that has
 *    advanced by two squares)
 * 6. Maximum of 8 pawns per side
 * 7. At most 10 of any type of piece, per side
 *
 * @return One of the \ref FenError codes
 */
auto Position::Validate(const Position& pos) -> FenError {
    const auto& white = pos.GetPlayerInfo<Player::kWhite>();
    const auto& black = pos.GetPlayerInfo<Player::kBlack>();

    const auto white_pawns = white.Pawns();
    const auto black_pawns = black.Pawns();
    const auto white_kings = white.King();
    const auto black_kings = black.King();

    if ((white_pawns | black_pawns) & (kRank1 | kRank8)) {
        return FenError::kPawnsOnBackRank;
    }

    if ((jfern::bitops::count(white_kings) != 1u) ||
        (jfern::bitops::count(black_kings) != 1u)) {
        return FenError::kNumberOfKings;
    }

    if ((pos.ToMove() == Player::kWhite && pos.InCheck<Player::kBlack>()) ||
        (pos.ToMove() == Player::kBlack && pos.InCheck<Player::kWhite>())) {
        return FenError::kKingCanBeCaptured;
    }

    const auto white_king_square = white.KingSquare();
    const auto black_king_square = black.KingSquare();

    const bool may_castle_short_w = white.CanCastleShort();
    const bool may_castle_long_w  = white.CanCastleLong();
    const bool may_castle_short_b = black.CanCastleShort();
    const bool may_castle_long_b  = black.CanCastleLong();

    const bool white_may_castle = may_castle_short_w || may_castle_long_w;
    const bool black_may_castle = may_castle_short_b || may_castle_long_b;

    if (white_king_square != Square::E1 && white_may_castle) {
        return FenError::kWhiteMayNotCastle;
    }

    if (black_king_square != Square::E8 && black_may_castle) {
        return FenError::kBlackMayNotCastle;
    }

    const auto black_rook_on_h8 =
        black.Rooks() & data_tables::kSetMask[Square::H8];
    const auto black_rook_on_a8 =
        black.Rooks() & data_tables::kSetMask[Square::A8];

    if (may_castle_short_b && !black_rook_on_h8) {
        return FenError::kBlackMayNotCastleShort;
    }

    if (may_castle_long_b && !black_rook_on_a8) {
        return FenError::kBlackMayNotCastleLong;
    }

    const auto white_rook_on_h1 =
        white.Rooks() & data_tables::kSetMask[Square::H1];
    const auto white_rook_on_a1 =
        white.Rooks() & data_tables::kSetMask[Square::A1];

    if (may_castle_short_w && !white_rook_on_h1) {
        return FenError::kWhiteMayNotCastleShort;
    }

    if (may_castle_long_w && !white_rook_on_a1) {
        return FenError::kWhiteMayNotCastleLong;
    }

    const auto ep_target   = pos.EnPassantTarget();
    const auto ep_target64 = data_tables::kSetMask[ep_target];

    if (ep_target != Square::Overflow &&
        ep_target != Square::Underflow) {
        if (pos.ToMove() == Player::kWhite) {
            if (util::GetRank(ep_target) != 5 ||
                (pos.Occupied() & ep_target64)) {
                return FenError::kEnPassantSquare;
            }
        } else {
            if (util::GetRank(ep_target) != 2 ||
                (pos.Occupied() & ep_target64)) {
                return FenError::kEnPassantSquare;
            }
        }
    }

    if ((jfern::bitops::count(white_pawns) > 8u) ||
        (jfern::bitops::count(black_pawns) > 8u)) {
        return FenError::kTooManyPawns;
    }

    if ((jfern::bitops::count(white.Knights()) > 10u) ||
        (jfern::bitops::count(black.Knights()) > 10u)) {
        return FenError::kTooManyKnights;
    }

    if ((jfern::bitops::count(white.Rooks()) > 10u) ||
        (jfern::bitops::count(black.Rooks()) > 10u)) {
        return FenError::kTooManyRooks;
    }

    if ((jfern::bitops::count(white.Queens()) > 10u) ||
        (jfern::bitops::count(black.Queens()) > 10u)) {
        return FenError::kTooManyQueens;
    }

    if ((jfern::bitops::count(white.Bishops()) > 10u) ||
        (jfern::bitops::count(black.Bishops()) > 10u)) {
        return FenError::kTooManyBishops;
    }

    return FenError::kSuccess;
}

/**
 * Clear this struct. Invalidates the origin/target squares
 */
void Position::EnPassantInfo::clear() {
    from[0] = from[1] = target = Square::Overflow;
}

}  // namespace chess
