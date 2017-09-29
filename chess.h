#ifndef __CHESS__
#define __CHESS__

#include <iostream>

#include "util.h"

//=====================================================================
// Macros
//=====================================================================

#define WHITE 1
#define BLACK 0

#define FILE_H 0x0101010101010101ULL
#define FILE_A (FILE_H << 7 )
#define RANK_1 0xFFULL
#define RANK_2 (RANK_1 << 08)
#define RANK_3 (RANK_1 << 16)
#define RANK_4 (RANK_1 << 24)
#define RANK_5 (RANK_1 << 32)
#define RANK_6 (RANK_1 << 40)
#define RANK_7 (RANK_1 << 48)
#define RANK_8 (RANK_1 << 56)

#define MAX_MOVES 256
#define MAX_PLY   64

#define RANK(a) ((a) >> 3)
#define FILE(a) ((a) &  7)

#define _abs(a)   ((a) < 0 ? -(a) : (a))
#define flip(a)   ((a)^1)
#define _max(a,b) ((a) > (b) ?(a) : (b))
#define _min(a,b) ((a) < (b) ?(a) : (b))

#define backRank(a) ((a) == WHITE ? RANK_1 : RANK_8)

#define swapUint32(a,b) \
{                       \
	uint32 temp = a;    \
	a = b;              \
	b = temp;           \
}

/**
 * Moves are packed in 21 bits:
 *
 * 20...18: promotion piece
 * 17...15: captured piece
 * 14...12: piece moved
 * 11... 6: destination square
 *  5... 0: origin square
 */
#define CAPTURED(a) (((a) >> 15) & 0x07)
#define FROM(a)     ((a) & 0x3F)
#define MOVED(a)    (((a) >> 12) & 0x07)
#define PROMOTE(a)  ((a) >> 18)
#define TO(a)       (((a) >> 6)  & 0x3F)

/**
 * Pack bits containing move data:
 */
#define pack(captured, from, moved, promote, to) \
	 (((captured) << 15) |   \
	  (from) |               \
	  ((moved) << 12) |      \
	  ((promote) << 18) |    \
	  ((to) << 6))

#define MATE_SCORE 1000000

//=====================================================================
// Externs
//=====================================================================

extern const char* SQUARE_STR[65];

//=====================================================================
// Types
//=====================================================================

// DO NOT CHANGE THE ORDER OF THESE!
typedef enum
{
	INVALID = 0,
	PAWN    = 1,
	ROOK    = 2,
	KNIGHT  = 3,
	BISHOP  = 4,
	QUEEN   = 5,
	KING    = 6
} piece_t;

enum
{
	PAWN_INDEX   = 0,
	KNIGHT_INDEX = 1,
	BISHOP_INDEX = 2,
	ROOK_INDEX   = 3,
	QUEEN_INDEX  = 4,
	KING_INDEX   = 5
};

typedef enum
{
	ALONG_RANK,
	ALONG_FILE,
	ALONG_A1H8,
	ALONG_H1A8,
	NONE

} direction_t;

typedef enum
{
	PAWN_VALUE   = 1000,
	KNIGHT_VALUE = 3250,
	BISHOP_VALUE = 3250,
	ROOK_VALUE   = 5000,
	QUEEN_VALUE  = 9750

} value_t;

/**
 * A mapping from piece enumeration to its value:
 */
const int piece_value[7] = 
{
	0            ,
	PAWN_VALUE   ,
	ROOK_VALUE   ,
	KNIGHT_VALUE ,
	BISHOP_VALUE ,
	QUEEN_VALUE  ,

	// Set the king's value to zero. When sorting
	// captures, this ensures that capturing 
	// with the king always wins material as long
	// as it is legal
	0
};

enum SQUARE
{
	H1, G1, F1, E1, D1, C1, B1, A1,
	H2, G2, F2, E2, D2, C2, B2, A2,
	H3, G3, F3, E3, D3, C3, B3, A3,
	H4, G4, F4, E4, D4, C4, B4, A4,
	H5, G5, F5, E5, D5, C5, B5, A5,
	H6, G6, F6, E6, D6, C6, B6, A6,
	H7, G7, F7, E7, D7, C7, B7, A7,
	H8, G8, F8, E8, D8, C8, B8, A8,
	BAD_SQUARE
};

namespace Util
{
	/**
	 ******************************************************************
	 *
	 * Base case for createBitboard(index1,...)
	 *
	 * @return 0
	 *
	 ******************************************************************
	 */
	uint64 createBitboard()
	{
		return 0;
	}

	/**
	 ******************************************************************
	 *
	 * Create a bitboard from a list of bit indexes. This is helpful if
	 * you want to build a bitboard and you know which squares you want
	 * to set
	 *
	 * @tparam T1 An integral type for the first argument
	 * @tparam T2 Integral types for additional arguments
	 *
	 * @param [in] index1  The first bit to set
	 * @param [in] indexes The indexes of any other bits to set
	 *
	 * @return A bitboard with bits set at each index specified
	 *
	 ******************************************************************
	 */
	template <typename T1, typename... T2>
	uint64 createBitboard(T1&& index1, T2&&... indexes)
	{
		uint64 one = 1;

		return (one << index1) |
				createBitboard(std::forward<T2>(indexes)...);
	}

	/**
	 ******************************************************************
	 *
	 * Convert a piece enumeration to its equivalent character
	 * representation in Algebraic notation
	 *
	 * @param[in] piece The piece to convert
	 *
	 * @return The upper case character equivalent
	 *
	 ******************************************************************
	 */
	char enum2piece(piece_t piece)
	{
		switch (piece)
		{
		case PAWN:
			return 'P';
		case KNIGHT:
			return 'N';
		case BISHOP:
			return 'B';
		case ROOK:
			return 'R';
		case QUEEN:
			return 'Q';
		case KING:
			return 'K';
		default:
			return '\0';
		}
	}

	/**
	 ******************************************************************
	 *
	 * Parse a move given in coordinate notation, retrieving the "from"
	 * and "to" squares and promotion piece (if applicable). The piece
	 * moved and/or captured cannot be deduced from coordinate notation
	 * alone. Examples:
	 *
	 * 1. e2-e4
	 * 2. e7e5
	 * 3. f7f8Q
	 *
	 * @param[in] _move The move to interpret
	 *
	 * @return The internal bit-packed move format, excluding the piece
	 *         moved and/or captured. Returns 0 on error
	 *
	 ******************************************************************
	 */
	static int32 parseCoordinate(const std::string& _move)
	{
		std::string from, to, move = to_lower(_move);
		piece_t promote;

		switch (move.back())
		{
			case 'n': promote = KNIGHT; break;
			case 'r': promote = ROOK;   break;
			case 'b': promote = BISHOP; break;
			case 'q': promote = QUEEN;  break;
			default:
				promote = INVALID;
		}

		if (promote != INVALID) move.pop_back();

		if (move.size() < 4) return 0;

		if (move.find("-") != std::string::npos)
		{
			str_v squares; split( move, squares, '-' );

			if (squares.size() != 2) return 0;

			from = squares[0];
			to   = squares[1];
		}
		else
		{
			from = move.substr(0,2);
			to   = move.substr(2,2);
		}

		/*
		 * Verify the sequence of four characters actually
		 * represents a move:
		 */
		int squares[2] = {BAD_SQUARE, BAD_SQUARE};

		for (int i = 0; i < 64; i++)
		{
			if (from == SQUARE_STR[i])
			{
				squares[0] = i; break;
			}
		}

		for (int i = 0; i < 64; i++)
		{
			if (to == SQUARE_STR[i])
			{
				squares[1] = i; break;
			}
		}

		/*
		 * Make sure the original and destination squares
		 * are not the same:
		 */
		if (squares[0] == BAD_SQUARE ||
			squares[1] == BAD_SQUARE ||
			squares[0] == squares[1])
			return 0;
		
		return pack(INVALID, squares[0],
			        INVALID,
			        static_cast<int>(promote),
			        squares[1]);
	}

	/**
	 ******************************************************************
	 *
	 * Display the given 64-bit integer as an 8x8 bit array
	 *
	 * @param[in] board The bitboard
	 *
	 ******************************************************************
	 */
	static void printBitboard(uint64 board)
	{
		int prev_rank = 8;
		uint64 one = 1;
		for (int sq = 63; sq >= -1; sq--)
		{
			if (RANK(sq) != prev_rank)
			{
				std::cout << "\n ---+---+---+---+---+---+---+--- \n";
				if (sq == -1) break;

				prev_rank = RANK(sq);
			}

			if (board & (one << sq))
				std::cout << "| * ";
			else
				std::cout << "|   ";

			if (sq % 8 == 0) std::cout << "|";
		}

		std::cout << std::endl;
	}

	/**
	 ******************************************************************
	 *
	 * Determine whether or not the given character represents a piece
	 * per algebraic notation
	 *
	 * @param [in] c The character to test
	 *
	 * @return True if this character represents a piece (note this is
	 *         NOT case-sensitive)
	 *
	 ******************************************************************
	 */
	static bool isPiece(char c)
	{
		return (c == 'p' || c == 'P' ||
			    c == 'r' || c == 'R' ||
			    c == 'n' || c == 'N' ||
			    c == 'b' || c == 'B' ||
			    c == 'k' || c == 'K' ||
			    c == 'q' || c == 'Q');
	}

	/**
	 ******************************************************************
	 *
	 * Get the enumeration equivalent for a piece given as a character
	 *
	 * @param [in] c The character to convert
	 *
	 * @return The enumerated value for this piece
	 *
	 ******************************************************************
	 */
	static piece_t piece2enum(char c)
	{
		switch (c)
		{
			case 'n':
			case 'N':
				return KNIGHT;
			case 'b':
			case 'B':
				return BISHOP;
			case 'p':
			case 'P':
				return PAWN;
			case 'r':
			case 'R':
				return ROOK;
			case 'k':
			case 'K':
				return KING;
			case 'q':
			case 'Q':
				return QUEEN;
			default:
				return INVALID;
		}
	}

	/**
	 ******************************************************************
	 *
	 * Get a human-readable form for a piece given as a piece_t
	 *
	 * @param [in] piece The piece
	 *
	 * @return The string representation for this piece
	 *
	 ******************************************************************
	 */
	static std::string piece2str(piece_t piece)
	{
		switch (piece)
		{
			case KNIGHT:
				return "knight";
			case BISHOP:
				return "bishop";
			case PAWN:
				return "pawn";
			case ROOK:
				return "rook";
			case KING:
				return "king";
			case QUEEN:
				return "queen";
			default:
				return "";
		}
	}

	/**
	 ******************************************************************
	 *
	 * Extract the move bits into human-readable form (mostly for
	 * diagnostic reasons)
	 *
	 * @param [in] move The move to parse
	 *
	 ******************************************************************
	 */
	static void printMove(int move)
	{
		const piece_t captured = static_cast<piece_t>(CAPTURED(move));
		const int from         = FROM(move);
		const piece_t moved    = static_cast<piece_t>(MOVED(move));
		const piece_t promote  = static_cast<piece_t>(PROMOTE(move) );
		const int to           = TO(move);

		std::cout << "captured: "
			<< piece2str(captured) << "\n";
		std::cout << "from:     "
			<< SQUARE_STR[from]    << "\n";
		std::cout << "moved:    "
			<< piece2str(moved)    << "\n";
		std::cout << "promote:  "
			<< piece2str(promote)  << "\n";
		std::cout << "to:       "
				<< SQUARE_STR[to] << std::endl;
	}

	/**
	 ******************************************************************
	 *
	 * Get the xBoard-compatible form of a set of internal move bits
	 *
	 * @param[in] move The move to convert
	 *
	 * @return The xBoard representation of this move
	 *
	 ******************************************************************
	 */
	static std::string printCoordinate(int move)
	{
		const SQUARE from     = static_cast<SQUARE>(FROM(move));
		const SQUARE to       = static_cast<SQUARE>(TO(move));
		const piece_t promote =
							static_cast<piece_t>(PROMOTE(move));

		std::string out(SQUARE_STR[from]);
		out += SQUARE_STR[to];
		out += enum2piece(promote);

		return out;
	}

	/**
	 ******************************************************************
	 *
	 * Display a position
	 *
	 * @param[in] pieces An array of 64 characters that represents the
	 *                   locations of all pieces
	 *
	 ******************************************************************
	 */
	static void showPosition(const char* pieces)
	{
		int prev_rank = 8;
		for (int sq = 63; sq >= -1; sq--)
		{
			if (RANK(sq) != prev_rank)
			{
				std::cout << "\n ---+---+---+---+---+---+---+--- \n";
				if (sq == -1) break;

				prev_rank = RANK(sq);
			}

			std::cout << "| " << pieces[sq] << " ";

			if (sq % 8 == 0)
				std::cout << "|";
		}

		std::cout << std::endl;
	}
}

#endif
