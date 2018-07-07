#ifndef __CHESS_UTIL__
#define __CHESS_UTIL__

#include "DataTables4.h"

namespace Chess
{
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
	inline int popCnt64(uint64 qword)
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
}
