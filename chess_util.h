#ifndef __CHESS_UTIL__
#define __CHESS_UTIL__

#include "DataTables.h"

namespace Util
{
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
