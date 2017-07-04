#ifndef __UTIL__
#define __UTIL__

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "abort.h"
#include "chess.h"
#include "types.h"

namespace Util
{
	typedef std::vector<std::string> str_v;

	/*
	 * Forward declarations:
	 */
	static void split(const std::string&, str_v&, char);
	static std::string to_lower(const std::string&);

	/**
	 **********************************************************************
	 *
	 * Count the number of bits set in a word in O(n) time
	 *
	 * @param [in] word An n-bit word
	 *
	 * @return The number of bits set in the word
	 *
	 **********************************************************************
	 */
	template<typename T> static int bitCount(T word)
	{
		int count = 0;

		for (count = 0; word; count++)
			word &= word - 1;

		return count;
	}

	/**
	 **********************************************************************
	 *
	 * Clear the specified bit within a word
	 *
	 * @param [in]     bit  The bit to clear
	 * @param [in,out] word An n-bit word
	 *
	 **********************************************************************
	 */
	template<typename T> inline void clearBit(int bit, T& word)
	{
		word ^= (word & ((T)1 << bit));
	}

	/**
	 **********************************************************************
	 *
	 * Clear the specified bits of a word
	 *
	 * @param [in]     mask Specifies the bits within \a word to clear
	 * @param [in,out] word An n-bit word
	 *
	 **********************************************************************
	 */
	template<typename T> inline void clearBits(T mask, T& word)
	{
		word ^= (word & mask);
	}

	/**
	 **********************************************************************
	 *
	 * Retrieve a bitmask with only the specified bit set
	 *
	 * @param [in] bit The desired bit, indexed from 0
	 *
	 * @return A power of 2 whose base-2 logarithm = bit. If bit exceeds
	 *         the size of T (in bits), -1 is returned, i.e. the maximum
	 *         value of the unsigned type T
	 *
	 **********************************************************************
	 */
	template<typename T> inline T getBit(uint32 bit)
	{
		AbortIfNot(bit < 8*sizeof(T), (T)~0);
		return ((T)1) << bit;
	}

	/**
	 **********************************************************************
	 *
	 * Get the index of the least significant bit set. This uses an O(n)
	 * algorithm, and should not be used beyond initialization for speed
	 * considerations
	 *
	 * @param [in] word The word to scan
	 *
	 * @return The LSB, or -1 if no bits are set
	 *
	 **********************************************************************
	 */
	template<typename T> static int getLSB(T word)
	{
		int bit = 0;
		T mask = 1;

		while (mask)
		{
			if (mask & word) return bit;
			mask <<= 1; bit += 1;
		}

		return -1;
	}

	/**
	 **********************************************************************
	 *
	 * Get the index of the most significant bit set. This uses an O(n)
	 * algorithm and should not be used beyond initialization for speed
	 * considerations
	 *
	 * @param [in] word The word to scan
	 *
	 * @return The MSB, or -1 if no bits are set
	 *
	 **********************************************************************
	 */
	template<typename T> static int getMSB(T word)
	{
		int bit = (8 * sizeof(T) - 1 );
		T mask = ((T)1) << bit;

		while (mask)
		{
			if (mask & word) return bit;
			mask >>= 1; bit -= 1;
		}

		return -1;
	}

	/**
	 **********************************************************************
	 *
	 * Returns the indexes of all bits set in a word
	 *
	 * @param [in]  word    The word to parse
	 * @param [out] indexes A list of bit indexes set
	 *
	 **********************************************************************
	 */
	template<typename T> static bool getSetBits(T word, uint32_v& indexes)
	{
		AbortIfNot(indexes.empty(), false);

		while (word)
		{
			int lsb = getLSB(word);
			indexes.push_back(lsb);

			clearBit(lsb, word);
		}

		return true;
	}

	/**
	 **********************************************************************
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
	 **********************************************************************
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
	 **********************************************************************
	 *
	 * Display the given 64-bit integer as an 8x8 bit array
	 *
	 * @param[in] board The bitboard
	 *
	 **********************************************************************
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
	 **********************************************************************
	 *
	 * Determine whether or not the given character represents a piece per
	 * algebraic notation
	 *
	 * @param [in] c The character to test
	 *
	 * @return True if the character represents a piece, false otherwise
	 *
	 **********************************************************************
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
	 **********************************************************************
	 *
	 * Get the enumeration equivalent for the piece given as a character
	 *
	 * @param [in] c The character to convert
	 *
	 * @return The enumerated value for this piece
	 *
	 **********************************************************************
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
	 **********************************************************************
	 *
	 * Set the specified bit in a word
	 *
	 * @param [in]     bit  The desired bit, indexed from 0
	 * @param [in,out] word The word to modify
	 *
	 **********************************************************************
	 */
	template<typename T> inline void setBit(uint32 bit, T& word)
	{
		word |= ((T)1) << bit;
	}
	
	/**
	 **********************************************************************
	 *
	 * Split a string into tokens. A delimiter is specified to indicate
	 * how to slice the string
	 *
	 * @param [in] str     The string to split
	 * @param [out] tokens A vector of the elements of the split string
	 * @param [in] delim   A delimiter (char)
	 *
	 **********************************************************************
	 */
	static void split(const std::string& str, str_v& tokens, char delim=' ')
	{
		std::string line = str;
		size_t ind, start = 0;

		while (true)
		{
			if ((ind = line.find(delim, start)) != std::string::npos)
			{
				if (ind - start > 0)
					tokens.push_back(line.substr(start,ind - start));
				start = ind+1;
			}
			else
			{
				if (start < line.size())
					tokens.push_back(line.substr(start,std::string::npos));
				break;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Get the integer representation of a string
	 *
	 * @param [in] str  The string to convert
	 * @param [in] base Radix (see strtol())
	 *
	 * @return The signed integer representing the given string. On error,
	 *         returns 0
	 *
	 **********************************************************************
	 */
	static int32 str_to_int32(const std::string& str, int base)
	{
		long int i = strtol(str.c_str(), NULL, base);
		AbortIf(errno == ERANGE, 0);
		AbortIf( i < MIN_INT32 || i > MAX_INT32, 0 );
		return (int32)i;
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Convert a character to lower case. Only when using the default C
	 * locale is this equivalent to ::tolower()
	 *
	 * @details
	 * Converts the given character in the range A-Z to lower case. If
	 * it is already in lower case or if there is no lower case 
	 * equivalent (e.g. a digit), the original character is returned
	 *
	 * @param [in] c The character to convert to lower case
	 *
	 * @return The lower case equivalent (if it exists) of the character
	 *
	 **********************************************************************
	 */
	static char to_lower(char c)
	{
		if (c >= 65 && c <= 90)
			c += 32;

		return c;
	}

	/**
	 **********************************************************************
	 *
	 * Converts a string to lower case. See to_lower(char) for details
	 *
	 * @param[in] str The string to convert
	 *
	 * @return The lower case equivalent of the string
	 *
	 **********************************************************************
	 */
	static std::string to_lower(const std::string& str)
	{
		std::string res = str;
		for (size_t i = 0; i < str.size(); i++)
			res[i] = to_lower(res[i]);

		return res;
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Convert a character to upper case. Only when using the default C
	 * locale is this equivalent to ::toupper()
	 *
	 * @details
	 * Converts the given character in the range A-Z to upper case. If
	 * it is already in upper case or if there is no upper case 
	 * equivalent (e.g. a digit), the original character is returned
	 *
	 * @param [in] c The character to convert to upper case
	 *
	 * @return The upper case equivalent (if it exists) of the character
	 *
	 **********************************************************************
	 */
	static char to_upper(char c)
	{
		if (c >= 97 && c <= 122)
			c -= 32;

		return c;
	}

	/**
	 **********************************************************************
	 *
	 * Remove leading and trailing whitespace from a string. This includes
	 * the character set " \t\n\v\f\r"
	 *
	 * @param[in] str Input string
	 *
	 * @return   The input string with all leading and trailing whitespace
	 *           removed
	 *
	 **********************************************************************
	 */
	static std::string trim(const std::string& str)
	{
		if (str.empty()) return str;

		const std::string space = "\t\n\v\f\r ";

		size_t start =
			str.find_first_not_of(space);

		if (start == std::string::npos)
			return "";

		size_t stop = str.find_last_not_of(space);

		return
			str.substr(start,stop-start+1);
	}
}

#endif
