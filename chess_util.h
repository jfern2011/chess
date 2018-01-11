#ifndef __CHESS_UTIL__
#define __CHESS_UTIL__

#include "DataTables.h"

namespace Util
{
	static bool compare_moves(int a, int b);

	/**
	 * Perform a bubble sort (one of the sorting algorithms used for
	 * move ordering)
	 *
	 * @param[in] items A list of items
	 * @param[in] numel The number of elements to sort
	 *
	 * @return  The number of passes performed as a result of having
	 *          to swap items
	 */
	inline int bubble_sort(int* items, size_t numel)
	{
		bool swapped = true;
		int passes = 0;

		while (swapped)
		{
			swapped = false;

			for (register size_t i = 1; i < numel; i++)
			{
				if ( !compare_moves(items[i-1], items[i]) )
				{
					swap(items[i-1], items[i]);
					swapped = true;
				}
			}
			passes++;

			// We've sorted the last element:
			numel--;
		}

		return passes;
	}

	/**
	 ******************************************************************
	 *
	 * Clear the specified bit in a 64-bit word
	 *
	 * @param [in]     bit  The bit to clear
	 * @param [in,out] word The word
	 *
	 ******************************************************************
	 */
	inline void clear_bit64(int bit, uint64& word)
	{
		word &= data_tables.clear_mask[bit];
	}

	/**
	 * Compare two moves by material gained. Moves are compared using
	 * the MVV/LVA approach, e.g. PxQ is ordered before PxR
	 *
	 * @param[in] a The first value
	 * @param[in] b The value to compare the first against
	 *
	 * @return True if \a a is greater than or equal to \a b; returns
	 *         false otherwise
	 */
	inline static bool compare_moves(int a, int b)
	{
		const int gain_a =
			piece_value[CAPTURED(a)] - piece_value[MOVED(a)];

		const int gain_b =
			piece_value[CAPTURED(b)] - piece_value[MOVED(b)];

		return gain_b <= gain_a;
	}

	/**
	 ******************************************************************
	 *
	 * Format a move in long algebraic notation
	 *
	 * @param [in] move     The move to format
	 * @param [in] in_check If true, append "+"
	 *
	 * @return The formatted string
	 *
	 ******************************************************************
	 */
	inline std::string format_move(int move, bool in_check=false)
	{
		const int captured = CAPTURED(move);
		const int from     = FROM(move);
		const int moved    = MOVED(move);
		const int promote  = PROMOTE(move);
		const int to       = TO(move);

		std::string out = "";

		if (moved == KING && _abs(from-to) == 2)
		{
			if (to > from) return "O-O-O";
			return "O-O";
		}
		else if (moved == PAWN
				 && captured != INVALID)
			out += SQUARE_STR[from][0];
		else
			out += enum2piece( static_cast<piece_t>(moved) )
				+ SQUARE_STR[from];

		if (captured != INVALID) out += "x";

		out += SQUARE_STR[to];

		if (in_check) out += "+";

		if (promote != INVALID)
		{
			out += "="
				+ enum2piece( static_cast<piece_t>(moved) );
		}

		return out;
	}

	/**
	 ******************************************************************
	 *
	 * Gets the least significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the least significant bit set, or -1 if no
	 *         bits are set
	 *
	 ******************************************************************
	 */
	inline int lsb64(uint64 qword)
	{
		qword &= (-qword);

		if (qword < 0x0000000010000ULL)
	    	return data_tables.lsb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 + data_tables.lsb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 + data_tables.lsb[qword >> 32];
	    
	    return 48 +
	    	data_tables.lsb[qword >> 48 ];
	}

	/**
	 ******************************************************************
	 *
	 * Gets the most significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the most significant bit set, or -1 if no
	 *         bits are set
	 *
	 ******************************************************************
	 */
	inline int msb64(uint64 qword)
	{
		if (qword < 0x0000000010000ULL)
	    	return data_tables.msb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 + data_tables.msb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 + data_tables.msb[qword >> 32];
		
		return 48 +
			data_tables.msb[qword >> 48 ];
	}

	/**
	 ******************************************************************
	 *
	 * Returns the population count (number of bits set) in a 64-bit
	 * word in constant time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The number of bits set
	 *
	 ******************************************************************
	 */
	inline int popCnt64(uint64 qword)
	{
		if (qword < 0x10000ULL) return data_tables.pop[qword];

    	if ( qword  <  0x0000100000000ULL )
    		return (data_tables.pop[ qword & 0xFFFF] +
					data_tables.pop[(qword >> 16) & 0xFFFF]);

    	if ( qword  <  0x1000000000000ULL )
    		return (data_tables.pop[ qword & 0xFFFF] +
					data_tables.pop[(qword >> 16) & 0xFFFF] +
					data_tables.pop[(qword >> 32) & 0xFFFF]);

    	return (data_tables.pop[ qword & 0xFFFF] +
    			data_tables.pop[(qword >> 16) & 0xFFFF] +
    			data_tables.pop[(qword >> 32) & 0xFFFF] +
    					data_tables.pop[(qword >> 48) & 0xFFFF]);
	}
}

#endif
