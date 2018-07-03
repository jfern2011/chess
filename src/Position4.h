#ifndef __POSITION_H__
#define __POSITION_H__

#include "DataTables4.h"

namespace Chess
{
	/**
	 * @class Position
	 *
	 * Represents a chess position
	 */
	class Position
	{
		/**
		 * Structure containing en passant information for
		 * a position
		 */
		struct EnPassant
		{
			/**
			 * The en passant target (i.e. "to") square
			 */
			int target;

			/**
			 * The origin square(s) from which a player may
			 * capture en passant
			 */
			BUFFER(int, src, 2);

			bool operator==(const EnPassant& rhs) const;

			void clear();
		};

		/**
		 * Structure that contains 781 64-bit integers used
		 * create a hash signature
		 */
		struct HashInput
		{
			/**
			 * 2 integers for castling rights for each
			 * player (4 total)
			 */
			BUFFER(uint64, castle_rights, 2, 2);

			/**
			 * 8 integers for the en passant square (1 per
			 * file)
			 */
			BUFFER(uint64, en_passant, 8);

			/**
			 * 1 integer for each piece on each square for
			 * both sides (768 total)
			 */
			BUFFER(uint64, piece, 2, 6, 64);

			/**
			 * 1 integer for the side to move
			 */
			uint64 to_move;
		};

	public:
		
	};

	/**
	 * Perform a byte-wise comparison between this object
	 * and another
	 *
	 * @param[in] rhs The object to compare against
	 *
	 * @return True if this EnPassant is the same as \a
	 *         rhs
	 */
	bool EnPassant::operator==(const EnPassant& rhs)
		const
	{
		return src[0] == rhs.src[0] && src[1] == rhs.src[1]
			&& target == rhs.target;
	}

	/**
	 * Set all members to their defaults
	 */
	void EnPassant::clear()
	{
		target = src[0] = src[1] = BAD_SQUARE;
	}

	/**
	 * Clear all entries
	 */
	void HashInput::clear()
	{
		for (int i = 0; i < 2; i++)
		{
			castle_rights[white][i] = 0;
			castle_rights[black][i] = 0;
		}

		for (int i = 0; i < 8; i++)
			en_passant[i] = 0;

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				piece[0][i][j] = 0;
				piece[1][i][j] = 0;
			}
		}

		to_move = 0;
	}

	/**
	 * Perform a byte-wise comparison between this object
	 * and another
	 *
	 * @param[in] rhs The object to compare against
	 *
	 * @return True if this HashInput is the same as \a
	 *         rhs
	 */
	bool operator==(const HashInput& rhs) const
	{
		bool same = true;

		for (int i = 0; i < 2; i++)
		{
			same = same &&
				castle_rights[white][i]
					== rhs.castle_rights[white][i];
			same = same && 
				castle_rights[black][i]
					== rhs.castle_rights[black][i];
		}

		for (int i = 0; i < 8; i++)
		{
			same = same &&
				en_passant[i] == rhs.en_passant[i];
		}

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				same = same &&
					piece[0][i][j] == rhs.piece[0][i][j];

				same = same &&
					piece[1][i][j] == rhs.piece[1][i][j];
			}
		}

		same = same &&
			to_move == rhs.to_move;

		return same;
	}
}

#endif
