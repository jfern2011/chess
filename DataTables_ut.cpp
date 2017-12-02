#include "DataTables.h"

#if 0 // TODO

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
bool DataTables::run_test() const
{
	uint64 one = 1;

	for (int square = 0; square < 64; square++)
	{
		uint64 temp = northeast_mask[square] |
					  northwest_mask[square] |
					  southeast_mask[square] |
					  southwest_mask[square] |
					  (one << square);

		AbortIfNot(bishop_range_mask  [square] == temp,
				   false);

		int xsb;

		xsb = Util::getMSB(northeast_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getMSB(northwest_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getLSB(southeast_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getLSB(southwest_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;

		temp ^= one << square;

		AbortIfNot(bishop_attacks_mask[square] == temp,
				   false);

		temp = north_mask[square] | east_mask[square] |
			   south_mask[square] | west_mask[square] |
			   (one << square);

		AbortIfNot(rook_range_mask  [square]   == temp,
				   false);

		xsb = Util::getMSB(north_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getMSB(west_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getLSB(east_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;
		xsb = Util::getLSB(south_mask[square]);
		if (xsb != -1)
			temp ^= one << xsb;

		temp ^= one << square;

		AbortIfNot(rook_attacks_mask[square]   == temp,
				   false);

		int dir1_count =
			Util::bitCount(northwest_mask[square]);
		int dir2_count =
			Util::bitCount(northeast_mask[square]);
		int dir3_count =
			Util::bitCount(southwest_mask[square]);
		int dir4_count =
			Util::bitCount(southeast_mask[square]);

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
				Util::getLSB(northwest_mask[square] & *iter);

			if (lsb != -1)
				nwAttacks = northwest_mask[square] ^ northwest_mask[lsb];
			else
				nwAttacks = northwest_mask[square];

			lsb =
				Util::getLSB(northeast_mask[square] & *iter);

			if (lsb != -1)
				neAttacks = northeast_mask[square] ^ northeast_mask[lsb];
			else
				neAttacks = northeast_mask[square];

			int msb =
				Util::getMSB(southeast_mask[square] & *iter);

			if (msb != -1)
				seAttacks = southeast_mask[square] ^ southeast_mask[msb];
			else
				seAttacks = southeast_mask[square];

			msb =
				Util::getMSB(southwest_mask[square] & *iter);

			if (msb != -1)
				swAttacks = southwest_mask[square] ^ southwest_mask[msb];
			else
				swAttacks = southwest_mask[square];

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
			Util::bitCount(north_mask[square]);
		dir2_count =
			Util::bitCount(east_mask[square] );
		dir3_count =
			Util::bitCount(west_mask[square] );
		dir4_count =
			Util::bitCount(south_mask[square]);

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
				Util::getLSB(north_mask[square] & *iter);

			if (lsb != -1)
				nAttacks = north_mask[square] ^ north_mask[lsb];
			else
				nAttacks = north_mask[square];

			lsb =
				Util::getLSB( west_mask[square] & *iter );

			if (lsb != -1)
				wAttacks  =  west_mask[square] ^ west_mask[lsb];
			else
				wAttacks  =  west_mask[square];

			int msb =
				Util::getMSB(south_mask[square] & *iter);

			if (msb != -1)
				sAttacks = south_mask[square] ^ south_mask[msb];
			else
				sAttacks = south_mask[square];

			msb =
				Util::getMSB( east_mask[square] & *iter );

			if (msb != -1)
				eAttacks  =  east_mask[square] ^ east_mask[msb];
			else
				eAttacks  =  east_mask[square];

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

		AbortIf(rank_adjacent[square] != adj,
				false);
	}

	/*
	 * Check the genOccupanciesDiag() and genOccupanciesRook()
	 * methods
	 */
	uint64_v occupancies_b, occupancies_r;

	for (int i = 0; i < 64; i++)
	{
		genOccupanciesDiag(i, occupancies_b);
		genOccupanciesRook(i, occupancies_r);

		int nbits_b =
			Util::bitCount<uint64>(bishop_attacks_mask[i]);

		int nbits_r =
			Util::bitCount<uint64>( rook_attacks_mask[i] );

		AbortIf(occupancies_b.size() !=
			(1 << nbits_b), false);

		AbortIf(occupancies_r.size() !=
			(1 << nbits_r), false);

		for (int i = 0; i < occupancies_b.size(); i++)
		{
			for (int j = 0; j < occupancies_b.size(); j++)
			{
				AbortIf(i != j && occupancies_b[i] == occupancies_b[j],
					false);
			}
		}

		for (int i = 0; i < occupancies_r.size(); i++)
		{
			for (int j = 0; j < occupancies_r.size(); j++)
			{
				AbortIf(i != j && occupancies_r[i] == occupancies_r[j],
					false);
			}
		}
	}

	return true;
}

#endif

DataTables tables;

int main()
{
	std::cout << "Running test...";
	std::fflush(stdout);

	if (tables.run_test())
		std::cout << "passed." << std::endl;
	else
		std::cout << "failed." << std::endl;
	
	return 0;
}
