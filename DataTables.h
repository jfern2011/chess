#ifndef __DATA_H__
#define __DATA_H__

#include "chess.h"

#define ATTACKS_ROOK_DB_SIZE 0x19000
#define ATTACKS_DIAG_DB_SIZE 0x01480

extern uint64 diag_magics[64];
extern uint64 rook_magics[64];


/**
 **********************************************************************
 *
 * @class DataTables
 *
 * Manages databases used throughout the engine, which are strictly
 * read-only
 *
 **********************************************************************
 */
class DataTables
{
	friend class Position;
	friend class MoveGen;
	
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

		/*
	 	 * Initialize general-purpose tables:
	 	 */
		init_misc_masks();
	}

	~DataTables()
	{
	}

	/**
	 * Verify all tables were properly initialized by checking
	 * them against expected values. Indeed, we could (almost)
	 * just use this routine to initialize the tables, but the
	 * goal here is to generate expected values via different
	 * algorithms to make sure we end up with the same tabular
	 * entries
	 *
	 * @return True if the tests all passed
	 */
	bool run_test() const
	{
		uint64 one = 1;

		for (int square = 0; square < 64; square++)
		{
			uint64 temp = northEastMask[square] |
						  northWestMask[square] |
						  southEastMask[square] |
						  southWestMask[square] |
						  (one << square);

			AbortIfNot(bishop_range_mask  [square] == temp,
					   false);

			int xsb;

			xsb = Util::getMSB(northEastMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getMSB(northWestMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getLSB(southEastMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getLSB(southWestMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;

			temp ^= one << square;

			AbortIfNot(bishop_attacks_mask[square] == temp,
					   false);

			temp = northMask[square] | eastMask[square] |
				   southMask[square] | westMask[square] |
				   (one << square);

			AbortIfNot(rook_range_mask  [square]   == temp,
					   false);

			xsb = Util::getMSB(northMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getMSB(westMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getLSB(eastMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;
			xsb = Util::getLSB(southMask[square]);
			if (xsb != -1)
				temp ^= one << xsb;

			temp ^= one << square;

			AbortIfNot(rook_attacks_mask[square]   == temp,
					   false);

			int dir1_count =
				Util::bitCount(northWestMask[square]);
			int dir2_count =
				Util::bitCount(northEastMask[square]);
			int dir3_count =
				Util::bitCount(southWestMask[square]);
			int dir4_count =
				Util::bitCount(southEastMask[square]);

			uint64_v neMasks, nwMasks, seMasks, swMasks;

			for (int i = 0; i < (1 << dir1_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square + 9 * (lsb + 1));
				}

				nwMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir2_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square + 7 * (lsb + 1));
				}

				neMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir3_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square - 7 * (lsb + 1));
				}

				swMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir4_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square - 9 * (lsb + 1));
				}

				seMasks.push_back(dir);
			}

			uint64_v occupancies;

			for (size_t i = 0; i < nwMasks.size(); i++)
			{
				for (size_t j = 0; j < neMasks.size(); j++)
				{
					for (size_t k = 0; k < swMasks.size(); k++)
					{
						for (size_t l = 0; l < seMasks.size(); l++)
						{
							/*
			 				 * Append the next occupancy variation for a bishop
			 				 * on this square:
			 				 */
							occupancies.push_back(nwMasks[i] |
												  neMasks[j] |
												  swMasks[k] |
												  seMasks[l]);
						}
					}
				}
			}

			uint64 in_reach =
				bishop_range_mask[square] ^ (one << square);
			size_t expected =
				1 << Util::bitCount(in_reach);

			AbortIf( occupancies.size() != expected, false);

			for ( auto iter = occupancies.begin(), end = occupancies.end();
				  iter != end; ++iter )
			{
				uint64 nwAttacks, neAttacks;
				uint64 swAttacks, seAttacks;

				int lsb =
					Util::getLSB(northWestMask[square] & *iter);

				if (lsb != -1)
					nwAttacks = northWestMask[square] ^ northWestMask[lsb];
				else
					nwAttacks = northWestMask[square];

				lsb =
					Util::getLSB(northEastMask[square] & *iter);

				if (lsb != -1)
					neAttacks = northEastMask[square] ^ northEastMask[lsb];
				else
					neAttacks = northEastMask[square];

				int msb =
					Util::getMSB(southEastMask[square] & *iter);

				if (msb != -1)
					seAttacks = southEastMask[square] ^ southEastMask[msb];
				else
					seAttacks = southEastMask[square];

				msb =
					Util::getMSB(southWestMask[square] & *iter);

				if (msb != -1)
					swAttacks = southWestMask[square] ^ southWestMask[msb];
				else
					swAttacks = southWestMask[square];

				uint64 attacks = nwAttacks | neAttacks |
				                 swAttacks | seAttacks;

				uint64 db_attacks =
					bishop_attacks[bishop_offsets[square] +
					(((*iter & bishop_attacks_mask[square])
						* diag_magics[square]) >> bishop_db_shifts[square])];

				AbortIf(attacks != db_attacks,
						false);
			}

			dir1_count =
				Util::bitCount(northMask[square]);
			dir2_count =
				Util::bitCount(eastMask[square] );
			dir3_count =
				Util::bitCount(westMask[square] );
			dir4_count =
				Util::bitCount(southMask[square]);

			uint64_v nMasks, eMasks, wMasks, sMasks;

			for (int i = 0; i < (1 << dir1_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square + 8 * (lsb + 1));
				}

				nMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir2_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square - 1 * (lsb + 1));
				}

				eMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir3_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square + 1 * (lsb + 1));
				}

				wMasks.push_back(dir);
			}

			for (int i = 0; i < (1 << dir4_count); i++)
			{
				uint64 dir = 0;
				for (int lsb, temp = i; temp; Util::clearBit(lsb,temp))
				{
					lsb = Util::getLSB(temp);
								dir |= one << (square - 8 * (lsb + 1));
				}

				sMasks.push_back(dir);
			}

			occupancies.clear();

			for (size_t i = 0; i < nMasks.size(); i++)
			{
				for (size_t j = 0; j < eMasks.size(); j++)
				{
					for (size_t k = 0; k < wMasks.size(); k++)
					{
						for (size_t l = 0; l < sMasks.size(); l++)
						{
							/*
			 				 * Append the next occupancy variation for a rook
			 				 * on this square:
			 				 */
							occupancies.push_back(nMasks[i] |
												  eMasks[j] |
												  wMasks[k] |
												  sMasks[l]);
						}
					}
				}
			}

			in_reach =
				rook_range_mask[square] ^ (one << square);
			expected =
				1 << Util::bitCount(in_reach);

			AbortIf(occupancies.size() != expected,false);

			for ( auto iter = occupancies.begin(), end = occupancies.end();
				  iter != end; ++iter )
			{
				uint64 nAttacks, eAttacks;
				uint64 wAttacks, sAttacks;

				int lsb =
					Util::getLSB(northMask[square] & *iter);

				if (lsb != -1)
					nAttacks = northMask[square] ^ northMask[lsb];
				else
					nAttacks = northMask[square];

				lsb =
					Util::getLSB( westMask[square] & *iter );

				if (lsb != -1)
					wAttacks  =  westMask[square] ^ westMask[lsb];
				else
					wAttacks  =  westMask[square];

				int msb =
					Util::getMSB(southMask[square] & *iter);

				if (msb != -1)
					sAttacks = southMask[square] ^ southMask[msb];
				else
					sAttacks = southMask[square];

				msb =
					Util::getMSB( eastMask[square] & *iter );

				if (msb != -1)
					eAttacks  =  eastMask[square] ^ eastMask[msb];
				else
					eAttacks  =  eastMask[square];

				uint64 attacks = nAttacks | eAttacks |
				                 sAttacks | wAttacks;

				uint64 db_attacks =
					rook_attacks[rook_offsets[square] +
					(((*iter & rook_attacks_mask[square])
						* rook_magics[square]) >> rook_db_shifts[square])];

				AbortIf(attacks != db_attacks,
						false);
			}

			switch (FILE(square))
			{
			case 7:
				AbortIf(square < 56 && pawn_attacks[WHITE][square]
						!= set_mask[square+7], false);
				AbortIf(square >  7 && pawn_attacks[BLACK][square]
						!= set_mask[square-9], false);
				break;
			case 0:
				AbortIf(square < 56 && pawn_attacks[WHITE][square]
						!= set_mask[square+9], false);
				AbortIf(square >  7 && pawn_attacks[BLACK][square]
						!= set_mask[square-7], false);
				break;
			default:
				AbortIf(square < 56 && pawn_attacks[WHITE][square]
						!= (set_mask[square+7] | set_mask[square+9]),
						false);
				AbortIf(square >  7 && pawn_attacks[BLACK][square]
						!= (set_mask[square-9] | set_mask[square-7]),
						false);
			}

			switch (RANK(square))
			{
			case 1:
				AbortIf(pawn_advances[WHITE][square] !=
						(set_mask[square+8] | set_mask[square+16]),
						false);
				AbortIf(pawn_advances[BLACK][square] !=
						set_mask[square-8], false);
				break;
			case 6:
				AbortIf(pawn_advances[BLACK][square] !=
						(set_mask[square-8] | set_mask[square-16]),
						false);
				AbortIf(pawn_advances[WHITE][square] !=
						set_mask[square+8], false);
				break;
			default:
				AbortIf((square < 56 && pawn_advances[WHITE][square] !=
						 set_mask[square+8])
							||
						(square >  7 && pawn_advances[BLACK][square] !=
						 set_mask[square-8]),
						false);

			}

			uint64 kingAttacks = 0;
			if (FILE(square) < 7)
			{
				kingAttacks     |= set_mask[square+1];
				if (RANK(square) < 7)
				{
					kingAttacks |= set_mask[square+9];
					kingAttacks |= set_mask[square+8];
				}
				if (RANK(square) > 0)
				{
					kingAttacks |= set_mask[square-7];
					kingAttacks |= set_mask[square-8];
				}
			}

			if (FILE(square) > 0)
			{
				kingAttacks     |= set_mask[square-1];
				if (RANK(square) < 7)
				{
					kingAttacks |= set_mask[square+7];
					kingAttacks |= set_mask[square+8];
				}
				if (RANK(square) > 0)
				{
					kingAttacks |= set_mask[square-9];
					kingAttacks |= set_mask[square-8];
				}
			}

			AbortIf(kingAttacks != king_attacks[square],
					false);

			uint64 knightAttacks = 0;
			if (RANK(square) < 7)
			{
				if (FILE(square) > 1)
					knightAttacks |= set_mask[square+ 6];
				if (FILE(square) < 6)
					knightAttacks |= set_mask[square+10];
			}

			if (RANK(square) > 0)
			{
				if (FILE(square) > 1)
					knightAttacks |= set_mask[square-10];
				if (FILE(square) < 6)
					knightAttacks |= set_mask[square -6];
			}

			if (RANK(square) < 6)
			{
				if (FILE(square) > 0)
					knightAttacks |= set_mask[square+15];
				if (FILE(square) < 7)
					knightAttacks |= set_mask[square+17];
			}

			if (RANK(square) > 1)
			{
				if (FILE(square) > 0)
					knightAttacks |= set_mask[square-17];
				if (FILE(square) < 7)
					knightAttacks |= set_mask[square-15];
			}

			AbortIf(knightAttacks !=
					knight_attacks[square], false);

			AbortIf(~clear_mask[square] !=
						  set_mask[square], false);

			uint64 adj = 0;

			if (FILE(square) < 7) adj |= set_mask[square+1];
			if (FILE(square) > 0) adj |= set_mask[square-1];

			AbortIf(rankAdjacent[square] != adj,
					false);
		}

		return true;
	}

private:
	
	/**
	 *  Given an occupancy bitboard, computes the squares attacked by a
	 *  bishop on \a square
	 *
	 * @param [in] square   The square the bishop is on
	 * @param [in] occupied The squares occupied by all other pieces
	 *
	 * @return A bitboard with bits set for all the squares attacked by
	 *         this bishop
	 */
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
	 *  Given an occupancy bitboard, computes the squares attacked by a
	 *  rook on \a square
	 *
	 * @param [in] square   The square the rook is on
	 * @param [in] occupied The squares occupied by all other pieces
	 *
	 * @return A bitboard with bits set for all the squares attacked by
	 *         this rook
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

	/**
	 * Create the bishop attacks database. This database is initialized
	 * so that an attacks board can be looked up using the "magic
	 * bitboard" move generation algorithm. This routine also
	 * initializes the offset and bit shift arrays needed by the lookup
	 * algorithm
	 */
	void createDiagAttacksDatabase(void)
	{
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
			
			for (int j = 0; j < occ_set.size(); j++)
			{
				uint32 index = bishop_offsets[i] +
					((diag_magics[i] * occ_set[j])
							>> bishop_db_shifts[i]);

				uint64 attacks =
						   computeDiagAttacks(i, occ_set[j]);

				bishop_attacks[index]  = attacks;
				bishop_mobility[index] =
							 Util::bitCount<uint64>(attacks);
			}
		}
	}

	/**
	 * Create the rook attacks database. This database gets initialized
	 * so that an attacks board can be looked up using the "magic
	 * bitboard" move generation algorithm. This initializes the offset
	 * and bit shift arrays used by the lookup as well
	 */
	void createRookAttacksDatabase(void)
	{
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
			
			for (int j = 0; j < occ_set.size(); j++)
			{
				uint32 index = rook_offsets[i] +
					((rook_magics[i] * occ_set[j]) >> rook_db_shifts[i]);

				uint64 attacks =
					computeRookAttacks(i, occ_set[j]);

				rook_attacks[index]  = attacks;
				rook_mobility[index] =
					  Util::bitCount<uint64>(attacks);
			}
		}
	}

	/**
	 * Initialize the bishop "range" and bishop attack masks. The range
	 * mask is the set of all squares reachable by a bishop from a
	 * particular square, including the square itself, if the bishop is
	 * not obstructed in any direction. The attack mask is the one
	 * applied to the occupied squares board in the magic bitboard move
	 * generation algorithm
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
	}

	/**
	 * @brief
	 * Generate an occupancy set (collection of bitboards) for a bishop
	 * on the given square
	 *
	 * @details
	 * An "occupancy set" is the set of all the occupancy bitmasks that
	 * would affect the range of squares a bishop on \a square could
	 * attack. For example, the squares attacked by a bishop on E4 are
	 * determined by the occupancies of the H1-A8 and B1-H7 diagonals.
	 * Because as many as 9 squares may be occupied (excluding edges
	 * of the board and the square the bishop is on), there are a total
	 * of 2^9 occupancy masks in the set
	 *
	 * @param[in] square         The square to create the occupancy set
	 *                           for
	 * @param[out] occupancy_set The occupancy set
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
	}

	/**
	 * @brief
	 *  Generate the occupancy set (collection of bitboards) for a rook
	 *  on the given square
	 *
	 * @details
	 * An "occupancy set" is the set of all the occupancy bitmasks that
	 * would affect the range of squares a rook on \a square could
	 * attack. For example, the squares attacked by a rook on E4 are
	 * determined by the occupancies of the 4th rank and E-file.
	 * Because as many as 10 squares may be occupied (excluding edges
	 * of the board and the square the rook is on) there are a total of
	 * of 2^10 occupancy masks in the set
	 *
	 * @param[in] square         The square to create the occupancy set
	 *                           for
	 * @param[out] occupancy_set The occupancy set
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
	}

	/**
	 * Initialize the rook "range" and rook attacks bitmasks. The range
	 * bitmask is the set of all squares reachable by a rook from a
	 * particular square, including the square itself, if the rook is
	 * not obstructed in any direction. The attacks mask is the
	 * one bit-wise ANDed with the occupied squares board in the "magic
	 * bitboard" move generation algorithm
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
	}

	/**
	 * Get the diagonal that the given square is on in the A1-H8
	 * direction
	 *
	 * @param [in] square The square
	 *
	 * @return The diagonal containing this square
	 */
	uint64 getDiagA1H8(int square)
	{
		uint64 one = 1, diag = 0;
		int sq = square;

		for (int sq = square; sq < 64; sq += 7)
		{
			diag |= one << sq;
			if (FILE(sq) == 0) break;
		}

		for (int sq = square; sq >= 0; sq -= 7)
		{
			diag |= one << sq;
			if (FILE(sq) == 7) break;
		}

		return diag;
	}

	/**
	 * Get the diagonal that the given square is on in the H1-A8
	 * direction
	 *
	 * @param [in] square The square
	 *
	 * @return The diagonal containing this square
	 */
	uint64 getDiagH1A8(int square)
	{
		uint64 one = 1, diag = 0;
		int sq = square;

		for (int sq = square; sq < 64; sq += 9)
		{
			diag |= one << sq;
			if (FILE(sq) == 7) break;
		}

		for (int sq = square; sq >= 0; sq -= 9)
		{
			diag |= one << sq;
			if (FILE(sq) == 0) break;
		}

		return diag;
	}

	/**
	 * Get the bitmask representing the file the given square is on
	 *
	 * @param [in] square The square
	 *
	 * @return The file containing this square
	 */
	uint64 getFile(int square)
	{
		uint64 mask = FILE_H;

		return mask << FILE(square);
	}

	/**
	 * Get the bitmask representing the rank the given square is on
	 *
	 * @param [in] square The square
	 *
	 * @return The rank containing this square
	 */
	uint64 getRank(int square)
	{
		uint64 mask = 0xFF;

		return mask << (8*RANK(square));
	}

	/**
	 * Initialize en passant targets. The target for a 4th rank square
	 * is its adjacent square on the 3rd rank. The target for a 5th
	 * rank square is its adjacent square on the 6th rank. For all the
	 * other squares the target is invalid
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
	 * Initialize the king attack bitboards
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
	 * Initialize the knight attack bitboards
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
	 * Initialize the pawn advance boards, i.e. the set of squares that
	 * a pawn can advance to from each square
	 *
	 *  NOTE: Probably get rid of this, since we simultaneously advance
	 *        pawns with a single bit shift
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
	 * Initialize the pawn attack bitboards
	 */
	void initPawnAttacks(void)
	{
		uint64 one = 1;

		for (unsigned int i = 0; i < 64; i++)
		{
			/*
			 * Note: This is nonsensical for pawns on either the 1st or
			 * 8th rank, but useful in other routines:
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
	 * Initialize the least/most significant bit and population count
	 * These tables immediately return a value for 16-bit
	 * words, and are therefore used to produce the LSB, MSB, and pop
	 * count for larger types
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
	}

	/**
	 * Initialize miscellaneous lookup tables
	 */
	void init_misc_masks(void)
	{
		const uint64 one = 1;

		for (int i = 0; i < 64; i++)
		{
			ranks64[i] = RANK_1 << (8 * RANK(i));
			files64[i] = FILE_H << FILE(i);

			h1a8_64[i] = northWestMask[i] |
						 southEastMask[i] |
						 (one << i);

			a1h8_64[i] = northEastMask[i] |
						 southWestMask[i] |
						 (one << i);;
		}

		for (int sq1 = 0; sq1 < 64; sq1++)
		{
			for (int sq2 = 0; sq2 < 64; sq2++)
			{
				ray_segment[sq1][sq2] =

					(northEastMask[sq1] & southWestMask[sq2]) |
					(northEastMask[sq2] & southWestMask[sq1]) | 
					(northMask[sq1]     & southMask[sq2])     |
					(northMask[sq2]     & southMask[sq1])     |
					(northWestMask[sq1] & southEastMask[sq2]) | 
					(northWestMask[sq2] & southEastMask[sq1]) |
					(eastMask[sq1]      & westMask [sq2])     |
					(eastMask[sq2]      & westMask [sq1]);

				if (h1a8_64[sq1] == h1a8_64[sq2])
				{
					ray_extend[sq1][sq2] = h1a8_64[sq1];
					directions[sq1][sq2] = ALONG_H1A8;
				}
				else if (a1h8_64[sq1] == a1h8_64[sq2])
				{
					ray_extend[sq1][sq2] = a1h8_64[sq1];
					directions[sq1][sq2] = ALONG_A1H8;
				}
				else if (RANK(sq1) == RANK(sq2))
				{
					ray_extend[sq1][sq2] = ranks64[sq1];
					directions[sq1][sq2] = ALONG_RANK;
				}
				else if (FILE(sq1) == FILE(sq2))
				{
					ray_extend[sq1][sq2] = files64[sq1];
					directions[sq1][sq2] = ALONG_FILE;
				}
				else
				{
					directions[sq1][sq2] = NONE;
					ray_extend[sq1][sq2] = 0;
				}
			}
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

			if (FILE(i) != 0)
				rankAdjacent[i] |= set_mask[i-1];
			if (FILE(i) != 7)
				rankAdjacent[i] |= set_mask[i+1];
		}

		back_rank[WHITE] = RANK_1;
		back_rank[BLACK] = RANK_8;
	}

	/*
	 * A database containing the "attacks from" bitboards for a bishop
	 */
	uint64 bishop_attacks[ATTACKS_DIAG_DB_SIZE];

	/*
	 * A database that contains the mobility of bishops, as a function
	 * of square and occupancy. A higher mobility score indicates that
	 * the bishop can move to more squares
	 */
	int bishop_mobility[ATTACKS_DIAG_DB_SIZE];

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
	 * A database that contains the mobility of rooks as a function
	 * of square and occupancy. A higher mobility score indicates a
	 * rook can move to more squares
	 */
	int rook_mobility[ATTACKS_ROOK_DB_SIZE];

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

	/*
	 *  Bitmasks representing the back rank for each side, namely:
	 *
	 * back_rank[WHITE] = RANK_1
	 * back_rank[BLACK] = RANK_8
	 */
	uint64 back_rank[2];

	/*
	 * Represents all squares located between any two squares, but
	 * excluding those two squares
	 */
	uint64 ray_segment[64][64];

	/*
	 * Similar to ray_segment, but includes the entire "line" along
	 * that direction, e.g.
	 *
	 * ray_extend[B2][C3] = entire A1-H8 diagonal
	 */
	uint64 ray_extend[64][64];

	/*
	 * Describes how two ray segments are connected (along a file,
	 * diagonal, etc.)
	 */
	direction_t directions[64][64];

	/*
	 *  Bitboards representing the file, rank, or diagonal a given
	 *  square lies on:
	 */
	uint64 files64[64];
	uint64 ranks64[64];
	uint64 a1h8_64[64];
	uint64 h1a8_64[64];
};

#endif