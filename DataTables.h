#ifndef __DATA_H__
#define __DATA_H__

#include "chess.h"

/**
 **********************************************************************
 *
 * @class DataTables
 *
 * Manages databases used throughout the engine
 *
 **********************************************************************
 */
class DataTables
{
	static const size_t ATTACKS_ROOK_DB_SIZE = 0x19000;
	static const size_t ATTACKS_DIAG_DB_SIZE = 0x01480;
	
public:

	DataTables();

	~DataTables();

	bool run_test() const;

private:
	
	uint64 computeDiagAttacks(int square, uint64 occupied) const;

	uint64 computeRookAttacks(int square, uint64 occupied) const;

	void createDiagAttacksDatabase();

	void createRookAttacksDatabase();

	void genBishopMasks();

	void genOccupanciesDiag(int square, uint64_v& occupancy_set) const;

	void genOccupanciesRook(int square, uint64_v& occupancy_set) const;

	void genRookMasks();

	uint64 getDiagA1H8(int square) const;

	uint64 getDiagH1A8(int square) const;

	uint64 getFile(int square) const;

	uint64 getRank(int square) const;

	void initEpTargets();

	void initKingAttacks();

	void initKnightAttacks();

	void initPawnAdvances();

	void initPawnAttacks();

	void initXSB();

	void init_misc_masks();

	static const uint64 diag_magics[64];
	static const uint64 rook_magics[64];

	/**
	 * Bitboard representing the a1-h8 diagonal that a given square
	 * lies on:
	 */
	BUFFER(uint64, a1h8_64, 64);

	/**
	 *  Bitmasks representing the back rank for each side, namely:
	 *
	 * back_rank[WHITE] = RANK_1
	 * back_rank[BLACK] = RANK_8
	 */
	BUFFER(uint64, back_rank, 2);

	/**
	 * A database containing the "attacks from" bitboards for a bishop
	 */
	BUFFER(uint64, bishop_attacks, ATTACKS_DIAG_DB_SIZE);

	/**
	 * The occupancy squares we mask the occupied squares bitboard
	 * with to obtain a key into the \ref bishop_attacks database
	 */
	BUFFER(uint64, bishop_attacks_mask, 64);

	/**
	 * The amount we need to bit shift to obtain an index into the
	 * \ref bishop_attacks database
	 */
	BUFFER(uint32, bishop_db_shifts, 64);

	/**
	 * A database that contains the mobility of bishops, as a function
	 * of square and occupancy. A higher mobility score indicates that
	 * the bishop can move to more squares
	 */
	BUFFER(int, bishop_mobility, ATTACKS_DIAG_DB_SIZE);

	/**
	 * Offset into the \ref bishop_attacks database that marks the
	 * start of the "attacks from" boards for a particular square
	 */
	BUFFER(uint32, bishop_offsets, 64);

	/**
	 * All squares reachable by a bishop from a given square including
	 * the square itself
	 */
	BUFFER(uint64, bishop_range_mask, 64);

	/**
	 * Bitmasks used to clear single bits. All bits are set except
	 * at the corresponding index
	 */
	BUFFER(uint64, clear_mask, 64);

	/**
	 * Describes how two squares are connected (along a file, along
	 * a diagonal, etc.)
	 */
	BUFFER(direction_t, directions, 64, 64);

	/**
	 * All squares "east" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, east_mask, 64);

	/**
	 * En-passant target squares. These are invalid except for the 4th
	 * and 5th ranks
	 */
	BUFFER(uint64, ep_target, 64);

	/**
	 *  Material exchange[piece captured][piece moved]. A positive
	 *  value indicates material was gained
	 */
	BUFFER(int, exchange, 7, 7);

	/**
	 * Bitboard representing the file that a given square lies on:
	 */
	BUFFER(uint64, files64, 64);

	/**
	 * Bitboard representing the h1-a8 diagonal that a given square
	 * lies on:
	 */
	BUFFER(uint64, h1a8_64, 64);

	/**
	 * Database of "attacks from" bitboards for a king
	 */
	BUFFER(uint64, king_attacks, 64);

	/**
	 * Database of "attacks from" bitboards for a knight
	 */
	BUFFER(uint64, knight_attacks, 64);

	/**
	 * Returns the LSB for every possible unsigned 16-bit value
	 */
	BUFFER(short, lsb, 65536);

	/**
	 * Returns the MSB for every possible unsigned 16-bit value
	 */
	BUFFER(short, msb, 65536);

	/**
	 * All squares "north" of a particular square from white's
	 * perspective
	 */
	BUFFER(uint64, north_mask, 64);

	/**
	 * All squares "northeast" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, northeast_mask, 64);

	/**
	 * All squares "northwest" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, northwest_mask, 64);

	/**
	 * Database of the squares that a pawn can advance to (both sides)
	 */
	BUFFER(uint64, pawn_advances, 2, 64);

	/**
	 * Database of squares attacked by a pawn from a particular square
	 * (for both sides)
	 */
	BUFFER(uint64, pawn_attacks, 2, 64);

	/**
	 * Returns the population count for every possible unsigned
	 * 16-bit value
	 */
	BUFFER(short, pop, 65536);

	/**
	 * Bitmasks indicating the squares adjacent to each square and
	 * on the same rank
	 *
	 * E.g. rank_adjacent[E4] = D4 | F4
	 */
	BUFFER(uint64, rank_adjacent, 64);

	/**
	 * Bitboard representing the rank that a given square lies on:
	 */
	BUFFER(uint64, ranks64, 64);

	/**
	 * Similar to \ref ray_segment, but includes the entire "line"
	 * along that direction, e.g.
	 *
	 * ray_extend[B2][C3] = entire A1-H8 diagonal
	 */
	BUFFER(uint64, ray_extend, 64, 64);

	/**
	 * Represents all squares located between any two squares, but
	 * excluding those two squares
	 */
	BUFFER(uint64, ray_segment, 64, 64);

	/**
	 * Database containing "attacks from" bitboards for a rook
	 */
	BUFFER(uint64, rook_attacks, ATTACKS_ROOK_DB_SIZE);

	/**
	 * The occupancy squares we mask the occupied squares bitboard with
	 * to obtain a key into the \ref rook_attacks database
	 */
	BUFFER(uint64, rook_attacks_mask, 64);

	/**
	 * The amount we need to bit shift to obtain an index into the \ref
	 * rook_attacks database
	 */
	BUFFER(uint32, rook_db_shifts, 64);

	/**
	 * A database that contains the mobility of rooks as a function
	 * of square and occupancy. A higher mobility score indicates a
	 * rook can move to more squares
	 */
	BUFFER(int, rook_mobility, ATTACKS_ROOK_DB_SIZE);

	/**
	 * Offset into the \ref rook_attacks database that marks the start
	 * of the "attacks from" boards for a particular square
	 */
	BUFFER(uint32, rook_offsets, 64);

	/**
	 * All squares reachable by a rook from a given square, including
	 * the square itself
	 */
	BUFFER(uint64, rook_range_mask, 64);

	/**
	 * Bitmasks used to set single bits. All bits are clear except at
	 * the corresponding index
	 */
	BUFFER(uint64, set_mask, 64);

	/**
	 * All squares "south" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, south_mask, 64);

	/**
	 * All squares "southeast" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, southeast_mask, 64);

	/**
	 * All squares "southwest" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, southwest_mask, 64);

	/**
	 * All squares "west" of a particular square, from white's
	 * perspective
	 */
	BUFFER(uint64, west_mask, 64);
};

#endif
