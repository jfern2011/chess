#include "DataTables.h"

/**
 * Our global set of databases
 */
DataTables data_tables;

/**
 * Constructor
 */
DataTables::DataTables()
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

/**
 * Destructor
 */
DataTables::~DataTables()
{
}

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
uint64 DataTables::computeDiagAttacks(int square,
	uint64 occupied) const
{
	uint64 attacks =
		bishop_range_mask[square] ^ Util::getBit<uint64>(square);

	int blocker =
		Util::getLSB<uint64>(occupied & northeast_mask[square]);

	if (blocker != -1)
		attacks ^= northeast_mask[blocker];

	blocker =
		Util::getMSB<uint64>(occupied & southeast_mask[square]);

	if (blocker != -1)
		attacks ^= southeast_mask[blocker];

	blocker =
		Util::getLSB<uint64>(occupied & northwest_mask[square]);

	if (blocker != -1)
		attacks ^= northwest_mask[blocker];

	blocker =
		Util::getMSB<uint64>(occupied & southwest_mask[square]);

	if (blocker != -1)
		attacks ^= southwest_mask[blocker];

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
uint64 DataTables::computeRookAttacks(int square,
	uint64 occupied) const
{
	uint64 attacks =
		rook_range_mask[square] ^ Util::getBit<uint64>(square);

	int blocker = 
		Util::getLSB<uint64>(occupied & north_mask[square]);

	if (blocker != -1)
		attacks ^= north_mask[blocker];

	blocker = 
		Util::getLSB<uint64>(occupied & west_mask[square]);

	if (blocker != -1)
		attacks ^= west_mask[blocker];

	blocker = 
		Util::getMSB<uint64>(occupied & east_mask[square]);

	if (blocker != -1)
		attacks ^= east_mask[blocker];

	blocker = 
		Util::getMSB<uint64>(occupied & south_mask[square]);

	if (blocker != -1)
		attacks ^= south_mask[blocker];

	return attacks;
}

/**
 * Initialize the \ref bishop_attacks database. This database is
 * used to store "attacks from" bitboards that can be looked
 * up using the "magic bitboard" move generation algorithm. In
 * addition, this routine initializes the \ref
 * bishop_db_shifts and \ref bishop_offsets databases needed by
 * the lookup algorithm
 *
 * Note this also initializes the \ref bishop_mobility database,
 * which uses the same lookup process
 */
void DataTables::createDiagAttacksDatabase()
{
	genBishopMasks();

	/*
	 *  Initialize the offsets and bit shifts required to obtain
	 *  an index into the bishop_attacks database
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
			Util::bitCount( bishop_attacks_mask[sq] );

		bishop_offsets[sq] =
			 bishop_offsets[sq-1] + (one << variations);
	}

	/*
	 * Initialize the bishop_attacks database:
	 */
	for (int i = 0; i < 64; i++)
	{
		uint64_v occ_set; genOccupanciesDiag(i, occ_set);
		
		for (int j = 0; j < occ_set.size(); j++)
		{
			uint32 index = bishop_offsets[i] +
				((diag_magics[i] * occ_set[j]) >> bishop_db_shifts[i]);

			uint64 attacks =
				computeDiagAttacks(i, occ_set[j]);

			int n_set =
				Util::bitCount<uint64>( attacks );

			bishop_attacks[index]  = attacks;
			bishop_mobility[index] = n_set;
		}
	}
}

/**
 * Initializes the \ref rook_attacks database. This database is
 * used to store "attacks from" bitboards that can be looked
 * up using the "magic bitboard" move generation algorithm. In
 * addition, this routine initializes the \ref 
 * rook_db_shifts and the \ref rook_offsets databases needed by
 * the lookup algorithm
 *
 * Note this also initializes the \ref rook_mobility database,
 * which uses the same lookup process
 */
void DataTables::createRookAttacksDatabase()
{
	genRookMasks();

	/*
	 * Initialize the offsets and bit shifts required to obtain
	 * an index into the rook_attacks database
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
	 * Initialize the rook_attacks database:
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

			int n_set =
				Util::bitCount<uint64>( attacks );

			rook_attacks[index]  = attacks;
			rook_mobility[index] = n_set;
		}
	}
}

/**
 * Initializes the \ref bishop_range_mask and \ref bishop_attacks_mask
 * tables. The former is the set of all squares reachable by a bishop
 * from a particular square, including the square itself, if the
 * bishop is not obstructed in any direction. The latter is the one we
 * bitwise AND with the occupied squares board in the magic bitboard
 * generation algorithm
 *
 * This also initializes the following tables:
 *
 * \ref northeast_mask
 * \ref northwest_mask
 * \ref southeast_mask
 * \ref southwest_mask
 */
void DataTables::genBishopMasks()
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
	 * Initialize direction masks:
	 */
	uint64 one = 1;

	for (int i = 0; i < 64; i++)
	{
		Util::clearBits<uint64>(~0, northeast_mask[i]);
		Util::clearBits<uint64>(~0, northwest_mask[i]);
		Util::clearBits<uint64>(~0, southeast_mask[i]);
		Util::clearBits<uint64>(~0, southwest_mask[i]);

		uint64 diag = getDiagA1H8(i);

		for (int sq = i; diag & (one << sq); sq += 7)
		{
			if (sq > 63) break;
			if (sq != i)
				northeast_mask[i] |= one << sq;
		}

		for (int sq = i; diag & (one << sq); sq -= 7)
		{
			if (sq < 0) break;
			if (sq != i)
				southwest_mask[i] |= one << sq;
		}

		diag = getDiagH1A8(i);

		for (int sq = i; diag & (one << sq); sq += 9)
		{
			if (sq > 63) break;
			if (sq != i)
				northwest_mask[i] |= one << sq;
		}

		for (int sq = i; diag & (one << sq); sq -= 9)
		{
			if (sq < 0) break;
			if (sq != i)
				southeast_mask[i] |= one << sq;
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
void DataTables::genOccupanciesDiag(
	int square, uint64_v& occupancy_set) const
{
	occupancy_set.clear();

	uint64 diag = bishop_attacks_mask[ square ];

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
void DataTables::genOccupanciesRook(int square,
		uint64_v& occupancy_set) const
{
	occupancy_set.clear();

	uint64 range = rook_attacks_mask[ square ];

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
 * Initialize the \ref rook_range_mask and \ref rook_attacks_mask
 * tables. The former is the set of all squares reachable by a
 * rook from a particular square, including the square itself, if
 * the rook is not obstructed in any direction. The latter is
 * the one that we bitwise AND with the occupied squares board in
 * the magic bitboard move generation algorithm
 *
 * This also initializes the following tables:
 *
 * \ref south_mask
 * \ref east_mask
 * \ref north_mask
 * \ref west_mask
 */
void DataTables::genRookMasks()
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
	 * Initialize direction masks:
	 */
	uint64 one = 1;

	for (int i = 0; i < 64; i++)
	{
		Util::clearBits<uint64>(~0, south_mask[i]);
		Util::clearBits<uint64>(~0, north_mask[i]);
		Util::clearBits<uint64>(~0, east_mask [i]);
		Util::clearBits<uint64>(~0, west_mask [i]);

		for (int sq = i+8; sq < 64; sq += 8)
			north_mask[i] |= one << sq;

		for (int sq = i-8; sq >= 0; sq -= 8)
			south_mask[i] |= one << sq;

		for (int sq = i+1; sq < 64 && RANK(sq) == RANK(i); sq++)
			west_mask[i] |= (one << sq);

		for (int sq = i-1; sq >= 0 && RANK(sq) == RANK(i); sq--)
			east_mask[i] |= (one << sq);
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
uint64 DataTables::getDiagA1H8(int square) const
{
	uint64 one = 1, diag = 0;

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
uint64 DataTables::getDiagH1A8(int square) const
{
	uint64 one = 1, diag = 0;

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
uint64 DataTables::getFile(int square) const
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
uint64 DataTables::getRank(int square) const
{
	uint64 mask = 0xFF;

	return mask << (8*RANK(square));
}

/**
 * Initialize en passant targets (\ref ep_target). The target
 * for a 4th rank square is its adjacent square on the 3rd
 * rank, and the target for a 5th rank square is its adjacent
 * square on the 6th rank. For all the other squares, the
 * target is invalid
 */
void DataTables::initEpTargets()
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
 * Initialize the \ref king_attacks bitboards
 */
void DataTables::initKingAttacks()
{
	for (register int i = 0; i < 64; i++)
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
 * Initialize the \ref knight_attacks bitboards
 */
void DataTables::initKnightAttacks()
{
	for (register int i = 0; i < 64; i++)
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
 * Initialize the \ref pawn_advances boards, i.e. the set of squares
 * that a pawn can advance to from each square
 *
 * NOTE: Probably remove this, since we simultaneously advance pawns
 *       with a single bit shift
 */
void DataTables::initPawnAdvances()
{
	uint64 one = 1;

	for (unsigned int i = 0; i < BAD_SQUARE; i += 1)
	{
		pawn_advances[WHITE][i] = one << (i+8);
		pawn_advances[BLACK][i] = one << (i-8);

		if (RANK(i) == 1)
			pawn_advances[WHITE][i] |= one << (i+16);
		if (RANK(i) == 6)
			pawn_advances[BLACK][i] |= one << (i-16);
	}
}

/**
 *  Initialize the \ref pawn_attacks boards, i.e. the set of squares
 *  that a pawn attacks from each square
 */
void DataTables::initPawnAttacks()
{
	uint64 one = 1;

	for (unsigned int i = 0; i < 64; i++)
	{
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
 * tables, i.e. \ref lsb, \ref msb, and \ref pop. These tables
 * immediately return a value for 16-bit words, and are therefore
 * used to produce the LSB, MSB, and pop count of larger types
 */
void DataTables::initXSB()
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
 * Initialize miscellaneous lookup tables:
 *
 * \ref ranks64
 * \ref files64
 * \ref h1a8_64
 * \ref a1h8_64
 * \ref ray_segment
 * \ref ray_extend
 * \ref directions
 * \ref clear_mask
 * \ref set_mask
 * \ref rank_adjacent
 * \ref back_rank
 * \ref exchange
 * \ref kingside
 * \ref queenside
 */
void DataTables::init_misc_masks()
{
	const uint64 one = 1;

	for (int i = 0; i < 64; i++)
	{
		ranks64[i] = RANK_1 << (8 * RANK(i));
		files64[i] = FILE_H << FILE(i);

		h1a8_64[i] = northwest_mask[i] |
					 southeast_mask[i] |
					 (one << i);

		a1h8_64[i] = northeast_mask[i] |
					 southwest_mask[i] |
					 (one << i);;
	}

	for (int sq1 = 0; sq1 < 64; sq1++)
	{
		for (int sq2 = 0; sq2 < 64; sq2++)
		{
			ray_segment[sq1][sq2] =

				(northeast_mask[sq1] & southwest_mask[sq2]) |
				(northeast_mask[sq2] & southwest_mask[sq1]) | 
				(north_mask[sq1]     & south_mask[sq2])     |
				(north_mask[sq2]     & south_mask[sq1])     |
				(northwest_mask[sq1] & southeast_mask[sq2]) | 
				(northwest_mask[sq2] & southeast_mask[sq1]) |
				(east_mask[sq1]      & west_mask [sq2])     |
				(east_mask[sq2]      & west_mask [sq1]);

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
		rank_adjacent[i] = 0;

		if (FILE(i) != 0)
			rank_adjacent[i] |= set_mask[i-1];
		if (FILE(i) != 7)
			rank_adjacent[i] |= set_mask[i+1];
	}

	back_rank[WHITE] = RANK_1;
	back_rank[BLACK] = RANK_8;

	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 7; j++)
			exchange[i][j] =
				piece_value[i] - piece_value[j];

	kingside[WHITE] =
		set_mask[F1] | set_mask[G1];
	kingside[BLACK] =
		set_mask[F8] | set_mask[G8];

	queenside[WHITE] =
		set_mask[B1] | set_mask[C1] | set_mask[D1];
	queenside[BLACK] =
		set_mask[B8] | set_mask[C8] | set_mask[D8];
}

/**
 * The 64-bit "magic" numbers used to hash the \ref bishop_attacks
 * database
 */
const uint64 DataTables::diag_magics[64] =
{
	0x03044810010A08B0ULL, 0x2090010101220004ULL,
	0x4008128112080140ULL, 0x0049040309204160ULL, 
	0x2004046020020418ULL, 0x5043012010001020ULL, 
	0x0004044148080000ULL, 0x1000410828030402ULL,
	0x0000300A08080085ULL, 0x0030102200840290ULL, 
	0x0000041400820020ULL, 0x0008782049400000ULL,
	0x6009020210000060ULL, 0x4000010420050000ULL, 
	0x000102820510400CULL, 0x28200201441C4420ULL,
	0x0008181142484800ULL, 0x0850040204014408ULL, 
	0x0030020805202024ULL, 0x0022000403220120ULL,
	0x0312008401A21820ULL, 0x1002011409820820ULL, 
	0x0122040100822008ULL, 0x0841084140425008ULL,
	0x0020200008130C01ULL, 0x0042100020010210ULL, 
	0x0044021001080900ULL, 0x1004010006490100ULL,
	0x1040404004010043ULL, 0x10480A0040220100ULL, 
	0x400802C062024200ULL, 0x0200404001840400ULL,
	0x2401080841405180ULL, 0x2008010404904428ULL, 
	0x0010404800900220ULL, 0x0241010802010040ULL,
	0x00304C0400004100ULL, 0x0030084200044100ULL, 
	0x1102408A00011801ULL, 0x080801044281004AULL,
	0x0001080840004428ULL, 0x0211241024040210ULL, 
	0x0280420045003001ULL, 0x1000006011080800ULL,
	0x1014022039000200ULL, 0x4810013001881B00ULL, 
	0x20E8020808582210ULL, 0x0808010C28804828ULL,
	0x14841404040E2404ULL, 0x0081221910480400ULL, 
	0x2080010401040000ULL, 0x0020080242022102ULL,
	0x1000001012020200ULL, 0x4050223401120002ULL, 
	0x2024040448120400ULL, 0x4104413204090000ULL,
	0x0006004062103040ULL, 0x48000C420804220CULL, 
	0x0020005D08A80400ULL, 0x4020040150940404ULL,
	0x40C0000052160208ULL, 0x1800000408100108ULL, 
	0x002060202A0201C0ULL, 0x000C110204040081ULL
};

/**
 * The 64-bit "magic" numbers used to hash the \ref rook_attacks
 * database
 */
const uint64 DataTables::rook_magics[64] =
{
	0x1880003023804000ULL, 0x4D40002001100040ULL, 
	0x0180181000802000ULL, 0x01000A1001002014ULL,
	0x020028A200041020ULL, 0x060008010A001004ULL, 
	0x1080020000800500ULL, 0x0200008204002841ULL,
	0x0013002040800304ULL, 0x0008400120005000ULL, 
	0x0001004020001301ULL, 0x0089002408100100ULL,
	0x0041001100180004ULL, 0x0041002604010018ULL, 
	0x10040018210A0410ULL, 0x1021000100006092ULL,
	0x0010608001824000ULL, 0x00C0008040200080ULL, 
	0x1139010044200011ULL, 0x0400210008100100ULL,
	0x4181030010080084ULL, 0x408400800CC20080ULL, 
	0x0018040068102102ULL, 0x1004020004204095ULL,
	0x1002008200250040ULL, 0x20100C4140012000ULL, 
	0x4103014100302000ULL, 0x2422001A00102040ULL,
	0x4000049100080100ULL, 0x2012005200110804ULL, 
	0x0041120400013008ULL, 0x0821002100004082ULL,
	0x00800420004002C0ULL, 0x0000200041401004ULL, 
	0x0000600501004090ULL, 0x0410002800801085ULL,
	0x011801004900100CULL, 0x0002000802000490ULL, 
	0x2F20021014000801ULL, 0x0008018402000043ULL,
	0x0080002002444000ULL, 0x2010002002404016ULL, 
	0x2005012000410010ULL, 0x0890003100190022ULL,
	0x0600050008010010ULL, 0x0104001008020200ULL, 
	0x2002020108240010ULL, 0x00025051208A0004ULL,
	0x0242010040802200ULL, 0x0000201002400240ULL, 
	0x4008590040200100ULL, 0x00400A2100100100ULL,
	0x0084280005001100ULL, 0x4001004802040100ULL, 
	0x6001004402000700ULL, 0x22000C884D140200ULL,
	0x0A80008020485103ULL, 0x0015108420400101ULL, 
	0x5080102000090041ULL, 0x0204211000080501ULL,
	0x4102002518102022ULL, 0x2401008804000201ULL, 
	0x4000020110080484ULL, 0x0000109040210402ULL
};
