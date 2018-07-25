#ifndef __CHESS_UTIL__
#define __CHESS_UTIL__

#include <limits>
#include <random>

#include "DataTables4.h"

namespace Chess
{
	/**
	 * Returns the absolute value of an integer
	 *
	 * @param[in] value The operand
	 *
	 * @return   The absolute value of \a value
	 */
	inline int abs( int value ) { return value > 0 ? value : -value; }

	/**
	 * Clear the specified bit in a 64-bit word
	 *
	 * @param [in]     bit   Which bit to clear
	 * @param [in,out] word  The word
	 *
	 */
	inline void clear_bit64(int bit, uint64& word)
	{
		word &= DataTables::get().clear_mask[bit];
	}

	/**
	 * Simultaneously clears the bit at index \a c and sets the bit at
	 * index \a s for the given word
	 *
	 * @param[in]  c    The bit to clear
	 * @param[in]  s    The bit to set
	 * @param[out] word The 64-bit word to modify
	 */
	inline void clear_set64(int c, int s, uint64& word)
	{
		auto& tables = DataTables::get();

		word |= tables.set_mask[s];
			word &= tables.clear_mask[c];
	}

	/**
	 * @defgroup move_bits Move bit packing
	 *
	 * Moves are packed in 21 bits:
	 *
	 * 20...18: promotion piece
	 * 17...15: captured piece
	 * 14...12: piece moved
	 * 11... 6: destination square
	 *  5... 0: origin square
	 *
	 * @{
	 */

	/**
	 * Extract the piece captured from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The captured piece
	 */
	inline piece_t extract_captured(int32 move)
	{
		return static_cast<piece_t>((move >> 15) & 0x07);
	}

	/**
	 * Extract the origin square from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The origin square
	 */
	inline square_t extract_from(int32 move)
	{
		return static_cast<square_t>(move & 0x3f);
	}

	/**
	 * Extract the piece moved from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The moved piece
	 */
	inline piece_t extract_moved(int32 move)
	{
		return static_cast<piece_t>((move >> 12) & 0x07);
	}

	/**
	 * Extract the piece promoted to from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The piece promoted to
	 */
	inline piece_t extract_promote(int32 move)
	{
		return static_cast<piece_t>(move >> 18);
	}

	/**
	 * Extract the destination square from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The destination square
	 */
	inline square_t extract_to(int32 move)
	{
		return static_cast<square_t>((move >> 6) & 0x3f);
	}

	/** @} */

	/**
	 * Switch sides
	 *
	 * @param [in] player The player to swap out
	 *
	 * @return The opposite side
	 */
	inline player_t flip(int player)
	{
		return static_cast<player_t>(player ^ 1);
	}

	/**
	 * Get the file of a particular square, indexed from zero
	 *
	 * @note The H-file corresponds to index 0
	 *
	 * @param[in] square The square
	 *
	 * @return The file that \a square is on
	 */
	inline int get_file(int square)
	{
		return square & 0x7;
	}

	/**
	 * Get the rank of a particular square, indexed from zero
	 *
	 * @note White's back rank corresponds to index 0
	 *
	 * @param[in] square The square
	 *
	 * @return The rank that \a square is on
	 */
	inline int get_rank(int square)
	{
		return square >> 3;
	}

	/**
	 * Determine whether or not the given character represents a piece
	 * per algebraic notation
	 *
	 * @param [in] c The character to test
	 *
	 * @return True if this character represents a piece (note this is
	 *         NOT case-sensitive)
	 */
	inline bool is_piece(char c)
	{
		return (c == 'p' || c == 'P' ||
			    c == 'r' || c == 'R' ||
			    c == 'n' || c == 'N' ||
			    c == 'b' || c == 'B' ||
			    c == 'k' || c == 'K' ||
			    c == 'q' || c == 'Q');
	}

	/**
	 * Gets the least significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the least significant bit set, or -1 if no
	 *         bits are set
	 */
	inline int lsb64(uint64 qword)
	{
		qword &= (-qword);

		auto& tables = DataTables::get();

		if (qword < 0x0000000010000ULL)
	    	return tables.lsb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 + tables.lsb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 + tables.lsb[qword >> 32];
	    
	    return 48 +
	    	tables.lsb[qword >> 48 ];
	}

	/**
	 * Gets the most significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the most significant bit set, or -1 if no
	 *         bits are set
	 */
	inline int msb64(uint64 qword)
	{
		auto& tables = DataTables::get();

		if (qword < 0x0000000010000ULL)
	    	return tables.msb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 + tables.msb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 + tables.msb[qword >> 32];
		
		return 48 +
			tables.msb[qword >> 48 ];
	}

	/**
	 * Pack move data into its 21-bit representation
	 *
	 * @param[in] captured The captured piece
	 * @param[in] from     The origin square
	 * @param[in] moved    The piece that was moved
	 * @param[in] promote  The piece promoted to
	 * @param[in] to       The destination square
	 *
	 * @return The bit-packed move
	 */
	inline int32 pack_move(piece_t  captured,
						   square_t from,
						   piece_t  moved,
						   piece_t  promote,
						   square_t to)
	{
		return (captured << 15) |
			   (from) |
			   (moved << 12) |
			   (promote << 18)  |
			   (to << 6);
	}

	/**
	 * Get the enumeration equivalent for a piece given as a character
	 *
	 * @param [in] c The character to convert
	 *
	 * @return The enumerated value for this piece
	 */
	inline piece_t piece2enum(char c)
	{
		switch (c)
		{
			case 'n':
			case 'N':
				return piece_t::knight;
			case 'b':
			case 'B':
				return piece_t::bishop;
			case 'p':
			case 'P':
				return piece_t::pawn;
			case 'r':
			case 'R':
				return piece_t::rook;
			case 'k':
			case 'K':
				return piece_t::king;
			case 'q':
			case 'Q':
				return piece_t::queen;
			default:
				return piece_t::empty;
		}
	}

	/**
	 * Returns the population count (number of bits set) in a 64-bit
	 * word in constant time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The number of bits set
	 */
	inline int pop_cnt64(uint64 qword)
	{
		auto& tables = DataTables::get();

		if (qword < 0x10000ULL) return tables.pop[qword];

    	if ( qword  <  0x0000100000000ULL )
    		return (tables.pop[ qword & 0xffff] +
					tables.pop[(qword >> 16) & 0xffff]);

    	if ( qword  <  0x1000000000000ULL )
    		return (tables.pop[ qword & 0xffff] +
					tables.pop[(qword >> 16) & 0xffff] +
					tables.pop[(qword >> 32) & 0xffff]);

    	return (tables.pop[ qword & 0xffff] +
    			tables.pop[(qword >> 16) & 0xffff] +
    			tables.pop[(qword >> 32) & 0xffff] +
    				tables.pop[(qword >> 48) & 0xffff]);
	}

	/**
	 * Placeholder for generating random 64-bit integers used for
	 * hashing a position
	 *
	 * @todo Experiment with other RNGs
	 *
	 * @return A random 64-bit number
	 */
	inline uint64 rand64()
	{
		static std::default_random_engine generator;

		static std::uniform_int_distribution<uint64>
			dist(0,std::numeric_limits<uint64>::max());

		return dist(generator);
	}

	/**
	 * Shift a player's pawn bitboard. See the specializations for
	 * shifting by 7 or by 9, which implement safeguards for shifting
	 * pawns diagonally
	 *
	 * @tparam N The number of bits to shift by
	 *
	 * @param[in] pawns   The bitboard to shift
	 * @param[in] to_move The player whose pawns to shift
	 *
	 * @return The shifted pawn board
	 */
	template <int N>
	inline uint64 shift_pawns(uint64 pawns, player_t to_move)
	{
		return (to_move == player_t::white) ?
			(pawns << N) : (pawns >> N);
	}

	/**
	 * Specialization of \ref shift_pawns(). Shifts the given pawn
	 * bitboard by 7 (e.g. to compute pawn captures). Pawns that would
	 * wrap around to the file on the opposite end of the board are
	 * discarded
	 *
	 * @param[in] pawns   The bitboard to shift
	 * @param[in] to_move The player whose pawns to shift
	 *
	 * @return The shifted pawn board
	 */
	template <>
	inline uint64 shift_pawns<7>(uint64 pawns, player_t to_move)
	{
		return (to_move == player_t::white) ?
			((pawns & (~file_h)) << 7) :
			((pawns & (~file_a)) >> 7);
	}

	/**
	 * Specialization of \ref shift_pawns(). Shifts the given pawn
	 * bitboard by 9 (e.g. to compute pawn captures). Pawns that would
	 * wrap around to the file on the opposite end of the board are
	 * discarded
	 *
	 * @param[in] pawns   The bitboard to shift
	 * @param[in] to_move The player whose pawns to shift
	 *
	 * @return The shifted pawn board
	 */
	template <>
	inline uint64 shift_pawns<9>(uint64 pawns, player_t to_move)
	{
		return (to_move == player_t::white) ?
			((pawns & (~file_a)) << 9) :
			((pawns & (~file_h)) >> 9);
	}
}

#endif
