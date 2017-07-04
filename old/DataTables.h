#ifndef __DATA_H__
#define __DATA_H__

#include <iostream>

#include "chess.h"
#include "util.h"

#define ATTACKS_ROOK_DB_SIZE   102400
#define ATTACKS_DIAG_DB_SIZE     5248

extern uint64 diag_magics[64];
extern uint64 rook_magics[64];


/**
 * @class DataTables
 *
 * An aggregate storage facility for commonly used databases. Moved from
 * the MoveGen class
 */
class DataTables
{
public:

	DataTables()
	{
		/*
		 * Initialize sliding piece attack databases:
		 */
		createDiagAttacksDatabase();
		createRookAttacksDatabase();

		/*
		 * Initialize en passant target squares:
		 */
		initEpTargets();

		/*
		 * Initialize king attack database:
		 */
		initKingAttacks();

		/*
		 * Initialize knight attack database:
		 */
		initKnightAttacks();

		/*
		 * Initialize pawn attack databases:
		 */
		initPawnAttacks();

		/*
		 * Initialize pawn advances databases:
		 */
		initPawnAdvances();

		/*
	 	 * Initlialize the bitscan tables:
	 	 */
		initXSB();

#if defined IS_VERBOSE
		std::cout << "Database tables initialized."
			<< std::endl;
#endif
	}

	~DataTables()
	{
	}

private:
	
	/**
	 **********************************************************************
	 *
	 * Given a board occupancy, computes the squares attacked by a bishop
	 * on \a square
	 *
	 * @param [in] square   The square the bishop is on
	 * @param [in] occupied The squares occupied by all other pieces
	 *
	 * @return A bit board with bits set for all squares attacked by this
	 *         bishop
	 *
	 **********************************************************************
	 */
#if defined (DEBUG_ATTACKS)
public:
#endif
	uint64 computeDiagAttacks(int square, uint64 occupied)
	{
		uint64 attacks =
			bishop_range_mask[square] ^ Util::getBit<uint64>(square);

		int blocker =
			Util::getLSB<uint64>(occupied & northEastMask[square]);

		if (blocker != -1)
			attacks ^= northEastMask[blocker];

		blocker =
			Util::getMSB<uint64>(occupied & southEastMask[square]);

		if (blocker != -1)
			attacks ^= southEastMask[blocker];

		blocker =
			Util::getLSB<uint64>(occupied & northWestMask[square]);

		if (blocker != -1)
			attacks ^= northWestMask[blocker];

		blocker =
			Util::getMSB<uint64>(occupied & southWestMask[square]);

		if (blocker != -1)
			attacks ^= southWestMask[blocker];

		return attacks;
	}

	/**
	 **********************************************************************
	 *
	 * Given the board occupancy, computes the squares attacked by a rook
	 * on \a square
	 *
	 * @param [in] square   The square the rook is on
	 * @param [in] occupied The squares occupied by all other pieces
	 *
	 * @return A bit board with bits set for all squares attacked by this
	 *         rook
	 *
	 **********************************************************************
	 */
	uint64 computeRookAttacks(int square, uint64 occupied)
	{
		uint64 attacks =
			rook_range_mask[square] ^ Util::getBit<uint64>(square);

		int blocker = 
			Util::getLSB<uint64>(occupied & northMask[square]);

		if (blocker != -1)
			attacks ^= northMask[blocker];

		blocker = 
			Util::getLSB<uint64>(occupied & westMask[square]);

		if (blocker != -1)
			attacks ^= westMask[blocker];

		blocker = 
			Util::getMSB<uint64>(occupied & eastMask[square]);

		if (blocker != -1)
			attacks ^= eastMask[blocker];

		blocker = 
			Util::getMSB<uint64>(occupied & southMask[square]);

		if (blocker != -1)
			attacks ^= southMask[blocker];

		return attacks;
	}
#if defined (DEBUG_ATTACKS)
private:
#endif

	/**
	 **********************************************************************
	 *
	 * Create the bishop attacks database. This database is initialized so
	 * that an attacks board can be looked up using the "magic bitboard"
	 * move generation algorithm. This routine also initializes the offset
	 * and bit shift arrays used by the lookup
	 *
	 **********************************************************************
	 */
	void createDiagAttacksDatabase(void)
	{
#if defined (IS_VERBOSE)
		std::cout << "Creating bishop attacks database..."
			<< std::endl;
#endif
		genBishopMasks();

		/*
		 * Initialize the offsets into the bishop attacks database as
		 * well as bit shifts required to obtain an index into the
		 * database according to the "magic bitboard" move generation
		 * algorithm:
		 */
		bishop_offsets[0] = 0;
		bishop_db_shifts[0] =
				64 - Util::bitCount(bishop_attacks_mask[0]);
		uint64 one = 1;
		for (int sq = 1; sq < 64; sq++)
		{
			int variations =
				Util::bitCount(bishop_attacks_mask[sq-1]);
			bishop_db_shifts[sq] = 64 -
				  Util::bitCount(bishop_attacks_mask[sq]);

			bishop_offsets[sq] =
				 bishop_offsets[sq-1] + (one << variations);
		}

		/*
		 * Initialize the bishop attacks database:
		 */
		for (int i = 0; i < 64; i++)
		{
			uint64_v occ_set; genOccupanciesDiag(i, occ_set);

#if defined (DEBUG_ATTACKS)

			if (occ_set.size() !=
				one << Util::bitCount(bishop_attacks_mask[i]))
			{
				std::cout << "Occupancy set size is wrong: "
					<< "Square = " << SQUARE_STR[i]
					<< ", set size = " << occ_set.size() << std::endl;
				return;
			}
#endif
			
			for (int j = 0; j < occ_set.size(); j++)
			{
				uint32 index = bishop_offsets[i] +
					((diag_magics[i] * occ_set[j])
							>> bishop_db_shifts[i]);

				uint64 attacks =
						computeDiagAttacks(i, occ_set[j]);

				bishop_attacks[index] = attacks;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Create the rook attacks database. This database gets initialized so
	 * that an attacks board can be looked up using the "magic bitboard"
	 * move generation algorithm. This routine also initializes the offset
	 * and bit shift arrays used by the lookup
	 *
	 **********************************************************************
	 */
	void createRookAttacksDatabase(void)
	{
#if defined (IS_VERBOSE)
		std::cout << "Creating rook attacks database..."
			<< std::endl;
#endif
		genRookMasks();

		/*
		 * Initialize the offsets into the rook attacks database as
		 * well as bit shifts required to obtain an index into the
		 * database according to the magic bitboard move generation
		 * algorithm:
		 */
		rook_offsets[0] = 0;
		rook_db_shifts[0] =
				64 - Util::bitCount(rook_attacks_mask[0]);
		uint64 one = 1;
		for (int sq = 1; sq < 64; sq++)
		{
			int variations =
				Util::bitCount(rook_attacks_mask[sq-1]);
			rook_db_shifts[sq] = 64 -
					Util::bitCount(rook_attacks_mask[sq]);

			rook_offsets[sq] =
				 rook_offsets[sq-1] + (one << variations);
		}


		/*
		 * Initialize the rook attacks database:
		 */
		for (int i = 0; i < 64; i++)
		{
			uint64_v occ_set; genOccupanciesRook(i, occ_set);

#if defined (DEBUG_ATTACKS)

			if (occ_set.size()
						 != one << Util::bitCount(rook_attacks_mask[i]))
			{
				std::cout << "Occupancy set size is wrong: "
					<< "Square = " << SQUARE_STR[i]
					<< ", set size is " << occ_set.size() << std::endl;
				return;
			}
#endif
			
			for (int j = 0; j < occ_set.size(); j++)
			{
				uint32 index = rook_offsets[i] +
					((rook_magics[i] * occ_set[j]) >> rook_db_shifts[i]);

				uint64 attacks = computeRookAttacks(i, occ_set[j]);

				rook_attacks[index] = attacks;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the bishop "range" and bishop attacks masks. The range
	 * mask is the set of all squares reachable by a bishop from a
	 * particular square, including the square itself, if the bishop is
	 * not obstructed in any direction. The attacks mask is the one
	 * applied to the occupied squares board in the magic bitboard move
	 * generation algorithm
	 *
	 **********************************************************************
	 */
	void genBishopMasks(void)
	{
		const uint64 FRAME = RANK_1 | RANK_8 | FILE_A | FILE_H;

		for (int i = 0; i < 64; i++)
		{
			const uint64 diag_a1h8 = getDiagA1H8(i);
			const uint64 diag_h1a8 = getDiagH1A8(i);

			uint64 scope = diag_a1h8 | diag_h1a8;
			bishop_range_mask[i] = scope;

			scope ^= scope & (FRAME | Util::getBit<uint64>(i));
			bishop_attacks_mask[i] = scope;
		}

#if defined (DEBUG_GEN_MASK_B)

		std::cout << "*** BISHOP RANGE MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(bishop_range_mask[i]);
			std::cin.get();
		}

		std::cout << "*** BISHOP ATTACK MASKS ***\n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(bishop_attacks_mask[i]);
			std::cin.get();
		}
#endif

		/*
		 * Initialize "partial" range masks:
		 */
		uint64 one = 1;

		for (int i = 0; i < 64; i++)
		{
			Util::clearBits<uint64>(~0, northEastMask[i]);
			Util::clearBits<uint64>(~0, northWestMask[i]);
			Util::clearBits<uint64>(~0, southEastMask[i]);
			Util::clearBits<uint64>(~0, southWestMask[i]);

			uint64 diag = getDiagA1H8(i);

			for (int sq = i; diag & (one << sq); sq += 7)
			{
				if (sq > 63) break;
				if (sq != i)
					northEastMask[i] |= one << sq;
			}

			for (int sq = i; diag & (one << sq); sq -= 7)
			{
				if (sq < 0) break;
				if (sq != i)
					southWestMask[i] |= one << sq;
			}

			diag = getDiagH1A8(i);

			for (int sq = i; diag & (one << sq); sq += 9)
			{
				if (sq > 63) break;
				if (sq != i)
					northWestMask[i] |= one << sq;
			}

			for (int sq = i; diag & (one << sq); sq -= 9)
			{
				if (sq < 0) break;
				if (sq != i)
					southEastMask[i] |= one << sq;
			}
		}

#if defined (DEBUG_GEN_MASK_B)

		std::cout << "*** NORTHEAST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(northEastMask[i]);
			std::cin.get();
		}

		std::cout << "*** NORTHWEST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(northWestMask[i]);
			std::cin.get();
		}

		std::cout << "*** SOUTHEAST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(southEastMask[i]);
			std::cin.get();
		}

		std::cout << "*** SOUTHWEST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(southWestMask[i]);
			std::cin.get();
		}
#endif
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Generate the occupancy set for a bishop on the given square
	 *
	 * @details
	 * An "occupancy set" is the set of all occupancy bit masks that would
	 * affect the range of squares a bishop on \a square could attack.
	 * For example, the squares attacked by a bishop on E4 are determined
	 * by the occupancies of the H1-A8 and B1-H7 diagonals. Because as
	 * many as 9 squares may be occupied (excluding the edges of the board
	 * and the square the bishop is on), there are a total of 2^9 occupancy
	 * masks in the set
	 *
	 * @param [in] square         The square to generate the occupancy set
	 *                            for
	 * @param [out] occupancy_set The occupancy set
	 *
	 ********************************************************************** 
	 */
	void genOccupanciesDiag(int square, uint64_v& occupancy_set)
	{
		uint64 diag = bishop_attacks_mask[square];

		int nbits = Util::bitCount<uint64>(diag);

		std::vector<uint64> bit_masks;

		uint32_v set_bits;
		Util::getSetBits<uint64>(diag, set_bits);

		for (int i = 0; i < set_bits.size(); i++)
		{
			bit_masks.push_back(Util::getBit<uint64>(set_bits[i]));
		}

		for (int i = 0; i < (1 << nbits); i++)
		{
			uint32_v i_bits;
			Util::getSetBits<int>(i, i_bits);
			uint64 mask = 0;

			for (int j = 0; j < i_bits.size(); j++)
				mask |= bit_masks[i_bits[j]];

			occupancy_set.push_back(mask);
		}

#if defined (DEBUG_GEN_OCCUPANCY_B)

		std::cout << "Occupancy set for " << SQUARE_STR[square]
			<< ":\n";
		std::cout << "Variations = "
			<< occupancy_set.size() << std::endl;

		for (size_t ind = 0; ind < occupancy_set.size(); ind++)
		{
			printBitboard ( occupancy_set[ind] );
			std::cin.get();
		}
#endif
	}

	/**
	 **********************************************************************
	 *
	 * @brief
	 * Generate the occupancy set for a rook on the given square
	 *
	 * @details
	 * An "occupancy set" is the set of all occupancy bit masks that would
	 * affect the range of squares a rook on \a square could attack.
	 * For example, the squares attacked by a rook on E4 are determined
	 * by the occupancies of the 4th rank and the E-file. Because as
	 * many as 10 squares may be occupied (excluding the edges of the board
	 * and the square the rook is on), there are a total of 2^10 occupancy
	 * masks in the set
	 *
	 * @param [in] square         The square to generate the occupancy set
	 *                            for
	 * @param [out] occupancy_set The occupancy set
	 *
	 ********************************************************************** 
	 */
	void genOccupanciesRook(int square, uint64_v& occupancy_set)
	{
		uint64 range = rook_attacks_mask[square];

		int nbits = Util::bitCount<uint64>(range);

		std::vector<uint64> bit_masks;

		uint32_v set_bits;
		Util::getSetBits<uint64>(range, set_bits);

		for (int i = 0; i < set_bits.size(); i++)
		{
			bit_masks.push_back(Util::getBit<uint64>(set_bits[i]));
		}

		for (int i = 0; i < (1 << nbits); i++)
		{
			uint32_v i_bits;
			Util::getSetBits<int>(i, i_bits);
			uint64 mask = 0;

			for (int j = 0; j < i_bits.size(); j++)
				mask |= bit_masks[i_bits[j]];

			occupancy_set.push_back(mask);
		}

#if defined (DEBUG_GEN_OCCUPANCY_R)

		std::cout << "Occupancy set for " << SQUARE_STR[square]
			<< ":\n";
		std::cout << "Variations = "
			<< occupancy_set.size() << std::endl;

		for (size_t ind = 0; ind < occupancy_set.size(); ind++)
		{
			printBitboard ( occupancy_set[ind] );
			std::cin.get();
		}
#endif
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the rook "range" and rook attacks masks. The range mask
	 * is the set of all squares reachable by a rook from a particular
	 * square, including the square itself, if the rook is not obstructed
	 * in any direction. The attacks mask is the one bitwise ANDed with
	 * the occupied squares board in the "magic bitboard" move generation
	 * algorithm
	 *
	 **********************************************************************
	 */
	void genRookMasks(void)
	{
		const uint64 FRAME = RANK_1 | RANK_8 | FILE_A | FILE_H;

		for (int i = 0; i < 64; i++)
		{
			const uint64 rank = getRank(i);
			const uint64 file = getFile(i);

			uint64 scope = rank | file;
			rook_range_mask[i] = scope;

			uint64 one = 1, tmp_frame = FRAME;

			switch (FILE(i))
			{
				case 0:
					tmp_frame ^= FILE_H ^ (one << H1) ^ (one << H8);
					break;
				case 7:
					tmp_frame ^= FILE_A ^ (one << A1) ^ (one << A8);
					break;
			}

			switch (RANK(i))
			{
				case 0:
					tmp_frame ^= RANK_1 ^ (one << A1) ^ (one << H1);
					break;
				case 7:
					tmp_frame ^= RANK_8 ^ (one << A8) ^ (one << H8);
					break;
			}

			scope ^=
				scope & (tmp_frame | (one << i));
			rook_attacks_mask[i] = scope;
		}

#if defined (DEBUG_GEN_MASK_R)

		std::cout << "*** ROOK RANGE MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(rook_range_mask[i]);
			std::cin.get();
		}

		std::cout << "*** ROOK ATTACK MASKS ***\n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(rook_attacks_mask[i]);
			std::cin.get();
		}
#endif

		/*
		 * Initialize "partial" range masks:
		 */
		uint64 one = 1;

		for (int i = 0; i < 64; i++)
		{
			Util::clearBits<uint64>(~0, southMask[i]);
			Util::clearBits<uint64>(~0, northMask[i]);
			Util::clearBits<uint64>(~0, eastMask [i]);
			Util::clearBits<uint64>(~0, westMask [i]);

			for (int sq = i+8; sq < 64; sq += 8)
				northMask[i] |= one << sq;

			for (int sq = i-8; sq >= 0; sq -= 8)
				southMask[i] |= one << sq;

			for (int sq = i+1; sq < 64 && RANK(sq) == RANK(i); sq++)
				westMask[i] |= (one << sq);

			for (int sq = i-1; sq >= 0 && RANK(sq) == RANK(i); sq--)
				eastMask[i] |= (one << sq);
		}

#if defined (DEBUG_GEN_MASK_R)

		std::cout << "*** NORTH MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(northMask[i]);
			std::cin.get();
		}

		std::cout << "*** SOUTH MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(southMask[i]);
			std::cin.get();
		}

		std::cout << "*** EAST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(eastMask[i]);
			std::cin.get();
		}

		std::cout << "*** WEST MASKS *** \n";
		for (int i = 0; i < 64; i++)
		{
			std::cout << SQUARE_STR[i] << ":"
				<< std::endl;

			printBitboard(westMask[i]);
			std::cin.get();
		}
#endif
	}

	/**
	 **********************************************************************
	 *
	 * Get the diagonal that the given square is on in the A1-H8 direction
	 *
	 * @param [in] square The square
	 *
	 * @return The diagonal containing this square
	 *
	 **********************************************************************
	 */
	uint64 getDiagA1H8(int square)
	{
		uint64 one = 1, diag = 0;
		int sq = square;

		for (int sq = square; (sq < 64); sq += 7)
		{
			diag |= one << sq;
			if (FILE(sq) == 0) break;
		}

		for (int sq = square; (sq >= 0); sq -= 7)
		{
			diag |= one << sq;
			if (FILE(sq) == 7) break;
		}

		return diag;
	}

	/**
	 **********************************************************************
	 *
	 * Get the diagonal that the given square is on in the H1-A8 direction
	 *
	 * @param [in] square The square
	 *
	 * @return The diagonal containing this square
	 *
	 **********************************************************************
	 */
	uint64 getDiagH1A8(int square)
	{
		uint64 one = 1, diag = 0;
		int sq = square;

		for (int sq = square; (sq < 64); sq += 9)
		{
			diag |= one << sq;
			if (FILE(sq) == 7) break;
		}

		for (int sq = square; (sq >= 0); sq -= 9)
		{
			diag |= one << sq;
			if (FILE(sq) == 0) break;
		}

		return diag;
	}

	/**
	 **********************************************************************
	 *
	 * Get the bitmask corresponding to the file the given square is on
	 *
	 * @param [in] square The square
	 *
	 * @return The file containing this square
	 *
	 **********************************************************************
	 */
	uint64 getFile(int square)
	{
		uint64 mask = FILE_H;

		return mask << FILE(square);
	}

	/**
	 **********************************************************************
	 *
	 * Get the bitmask corresponding to the rank the given square is on
	 *
	 * @param [in] square The square
	 *
	 * @return The rank containing this square
	 *
	 **********************************************************************
	 */
	uint64 getRank(int square)
	{
		uint64 mask = 0xFF;

		return mask << (8*RANK(square));
	}

	/**
	 **********************************************************************
	 *
	 * Initialize en passant targets.  The target for a square on the 4th
	 * rank is its adjacent square on the 3rd rank. The target for a
	 * square on the 5th rank is its adjacent square on the 6th rank. For
	 * all other squares, the target is invalid
	 *
	 **********************************************************************
	 */
	void initEpTargets(void)
	{
		for (int i = 0; i < 64; i++)
		{
			switch (RANK(i))
			{
				case 3:
					ep_target[i] = i-8; break;
				case 4:
					ep_target[i] = i+8; break;
				default:
					ep_target[i] = BAD_SQUARE;
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the king attack bitboards
	 *
	 **********************************************************************
	 */
	void initKingAttacks(void)
	{
		for (int i = 0; i < 64; i++)
		{
			king_attacks[i] = 0;

			if (FILE(i) < 7)
			{
					king_attacks[i] |= Util::getBit<uint64>(i+1);
				if (RANK(i) < 7)
					king_attacks[i] |= Util::getBit<uint64>(i+9);
				if (RANK(i) > 0)
					king_attacks[i] |= Util::getBit<uint64>(i-7);
			}

			if (RANK(i) < 7)
				king_attacks[i] |= Util::getBit<uint64>(i+8);
			if (RANK(i) > 0)
				king_attacks[i] |= Util::getBit<uint64>(i-8);

			if (FILE(i) > 0)
			{
					king_attacks[i] |= Util::getBit<uint64>(i-1);
				if (RANK(i) > 0)
					king_attacks[i] |= Util::getBit<uint64>(i-9);
				if (RANK(i) < 7)
					king_attacks[i] |= Util::getBit<uint64>(i+7);
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the knight attack bitboards
	 *
	 **********************************************************************
	 */
	void initKnightAttacks(void)
	{
		for (int i = 0; i < 64; i++)
		{
			knight_attacks[i] = 0;

			if (FILE(i) < 7)
			{
				if (RANK(i) < 6)
					knight_attacks[i] |= Util::getBit<uint64>(i+17);
				if (RANK(i) > 1)
					knight_attacks[i] |= Util::getBit<uint64>(i-15);
			}

			if (FILE(i) < 6)
			{
				if (RANK(i) < 7)
					knight_attacks[i] |= Util::getBit<uint64>(i+10);
				if (RANK(i) > 0)
					knight_attacks[i] |= Util::getBit<uint64>(i- 6);
			}

			if (FILE(i) > 0)
			{
				if (RANK(i) < 6)
					knight_attacks[i] |= Util::getBit<uint64>(i+15);
				if (RANK(i) > 1)
					knight_attacks[i] |= Util::getBit<uint64>(i-17);
			}

			if (FILE(i) > 1)
			{
				if (RANK(i) < 7)
					knight_attacks[i] |= Util::getBit<uint64>(i+ 6);
				if (RANK(i) > 0)
					knight_attacks[i] |= Util::getBit<uint64>(i-10);
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the pawn advance bitboards, i.e. set bits for the squares
	 * a pawn can advance to from each square
	 *
	 * NOTE: Probably get rid of this, since we simultaneously advance all
	 *       pawns with a single bit shift
	 *
	 **********************************************************************
	 */
	void initPawnAdvances(void)
	{
		uint64 one = 1;

		for (unsigned int i = 0; i < BAD_SQUARE; i += 1)
		{
			/*
			 * Note this is nonsensical for pawns on either the 1st or
			 * 8th rank, but useful in other routines:
			 */
			pawn_advances[WHITE][i] = one << (i+8);
			pawn_advances[BLACK][i] = one << (i-8);

			if (RANK(i) == 1)
				pawn_advances[WHITE][i] |= one << (i+16);
			if (RANK(i) == 6)
				pawn_advances[BLACK][i] |= one << (i-16);
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the pawn attack bitboards
	 *
	 **********************************************************************
	 */
	void initPawnAttacks(void)
	{
		uint64 one = 1;

		for (unsigned int i = 0; i < 64; i++)
		{
			/*
			 * Note: This is nonsensical for pawns on either the 1st or 8th
			 * rank, but useful in other routines:
			 */
			pawn_attacks[WHITE][i] = 0;
			pawn_attacks[BLACK][i] = 0;

			if (FILE(i) < 7)
			{
				pawn_attacks[WHITE][i] |= one << (i+9);
				pawn_attacks[BLACK][i] |= one << (i-7);
			}

			if (FILE(i) > 0)
			{
				pawn_attacks[WHITE][i] |= one << (i+7);
				pawn_attacks[BLACK][i] |= one << (i-9);
			}
		}
	}

	/**
	 **********************************************************************
	 *
	 * Initialize the least/most significant bit and population count
	 *  These tables immediately return a value for 16-bit
	 * words, and are therefore used to produce the LSB, MSB, and pop
	 * count for larger types
	 *
	 * This also sets the clear/set_mask[] databases
	 *
	 **********************************************************************
	 */
	void initXSB(void)
	{
		for (register uint32 i = 0; i < 65536; i++)
		{
			lsb[i] = Util::getLSB<uint16>(i);
			msb[i] = Util::getMSB<uint16>(i);
			pop[i] =
				Util::bitCount<uint16>(i);
		}

		for (int i = 0; i < 64; i++)
		{
			clear_mask[i] = ~(((uint64)1) << i);
			set_mask[i]   =
				~clear_mask[i];
		}

		for (int i = 0; i < 64; i++)
		{
			rankAdjacent[i] = 0;

			if (i > 0)
				rankAdjacent[i] |= set_mask[i-1];
			if (i < 63)
				rankAdjacent[i] |= set_mask[i+1];
		}
	}

public:
	/*
	 * Database containing "attacks from" bitboards for a bishop
	 */
	uint64 bishop_attacks[ATTACKS_DIAG_DB_SIZE];

	/*
	 * The occupancy squares we mask the occupied squares bitboard with
	 * to obtain a key into the bishop database
	 */
	uint64 bishop_attacks_mask[64];

	/*
	 * The amount we need to bit shift to obtain an index into the
	 * bishop database
	 */
	uint32 bishop_db_shifts[64];

	/*
	 * Offset into the bishop database that marks the start of the
	 * "attacks from" boards for a particular square
	 */
	uint32 bishop_offsets[64];

	/*
	 * All squares reachable by a bishop from a given square including
	 * the square itself
	 */
	uint64 bishop_range_mask[64];

	/*
	 * En-passant target squares. These are invalid except for the 4th
	 * and 5th ranks
	 */
	uint64 ep_target[64];

	/*
	 * Database of "attacks from" bitboards for a king
	 */
	uint64 king_attacks[64];

	/*
	 * Database of "attacks from" bitboards for a knight
	 */
	uint64 knight_attacks[64];

	/*
	 * Database of the squares that a pawn can advance to (both sides)
	 */
	uint64 pawn_advances[2][64];

	/*
	 * Database of squares attacked by a pawn from a particular square
	 * (for both sides)
	 */
	uint64 pawn_attacks[2][64];

	/*
	 * Database containing "attacks from" bitboards for a rook
	 */
	uint64 rook_attacks[ATTACKS_ROOK_DB_SIZE];

	/*
	 * The occupancy squares we mask the occupied squares bitboard with
	 * to obtain a key into the rook database
	 */
	uint64 rook_attacks_mask[64];

	/*
	 * The amount we need to bit shift to obtain an index into the rook
	 * database
	 */
	uint32 rook_db_shifts[64];

	/*
	 * Offset into the rook database that marks the start of the
	 * "attacks from" boards for a particular square
	 */
	uint32 rook_offsets[64];

	/*
	 * All squares reachable by a rook from a given square, including
	 * the square itself
	 */
	uint64 rook_range_mask[64];

	/*
	 * All squares "east" of a particular square, from white's
	 * perspective
	 */
	uint64 eastMask[64];

	/*
	 * All squares "north" of a particular square from white's
	 * perspective
	 */
	uint64 northMask[64];

	/*
	 * All squares "northeast" of a particular square, from white's
	 * perspective
	 */
	uint64 northEastMask[64];

	/*
	 * All squares "northwest" of a particular square, from white's
	 * perspective
	 */
	uint64 northWestMask[64];

	/*
	 * All squares "south" of a particular square, from white's
	 * perspective
	 */
	uint64 southMask[64];

	/*
	 * All squares "southeast" of a particular square, from white's
	 * perspective
	 */
	uint64 southEastMask[64];

	/*
	 * All squares "southwest" of a particular square, from white's
	 * perspective
	 */
	uint64 southWestMask[64];

	/*
	 * All squares "west" of a particular square, from white's
	 * perspective
	 */
	uint64 westMask[64];

	/*
	 * Returns the LSB for every possible unsigned 16-bit value
	 */
	short lsb[65536];

	/*
	 * Returns the MSB for every possible unsigned 16-bit value
	 */
	short msb[65536];

	/*
	 * Returns the population count for every possible unsigned
	 * 16-bit value
	 */
	short pop[65536];

	/*
	 * Bitmasks used to clear single bits. All bits are set except
	 * at the corresponding index
	 */
	uint64 clear_mask[64];

	/*
	 * Bitmasks used to set single bits. All bits are clear except
	 * at the corresponding index
	 */
	uint64 set_mask[64];

	/*
	 * Bitmasks indicating the squares adjacent to each square and
	 * on the same rank
	 *
	 * E.g. rankAdjacent[E4] = D4 | F4
	 */
	uint64 rankAdjacent[64];
};

#endif