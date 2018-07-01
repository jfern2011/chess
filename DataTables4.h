#ifndef __DATA_TABLES_H__
#define __DATA_TABLES_H__

#include "chess4.h"
#include "types.h"

namespace Chess
{
	/**
	 * @class DataTables
	 *
	 * Manages databases used throughout the engine which are accessed
	 * through a singleton instance
	 */
	class DataTables
	{
		/**
		 * The number of slots in the attacks_from database for
		 * rooks
		 */
		static const size_t ATTACKS_ROOK_DB_SIZE = 0x19000;

		/**
		 * The number of slots in the attacks_from database for
		 * bishops
		 */
		static const size_t ATTACKS_DIAG_DB_SIZE = 0x01480;

		DataTables();

		static Handle<DataTables> _tables;

	public:

		static const DataTables& get();

		/**
		 * The "magic" numbers used to look up attacks_from boards for
		 * bishops in the magic bitboard hashing algorithm
		 */
		BUFFER(uint64, diag_magics, 64);

		/**
		 * The "magic" numbers used to look up attacks_from boards for
		 * rooks in the magic bitboard hashing algorithm
		 */
		BUFFER(uint64, rook_magics, 64);

		/**
		 * Bitboard representing an a1-h8 diagonal that a given square
		 * lies on:
		 */
		BUFFER(uint64, a1h8_64, 64);

		/**
		 *  Bitmasks representing the back rank for each side, namely:
		 *
		 * back_rank[white] = Rank::_1
		 * back_rank[black] = Rank::_8
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
		BUFFER(int, bishop_db_shifts, 64);

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
		 * Bitboard representing an h1-a8 diagonal that a given square
		 * lies on:
		 */
		BUFFER(uint64, h1a8_64, 64);

		/**
		 * Database of "attacks from" bitboards for a king
		 */
		BUFFER(uint64, king_attacks, 64);

		/**
		 * Bits representing the kingside, i.e. for white this would
		 * be F1 and G1
		 */
		BUFFER(uint64, kingside, 2);

		/**
		 * Database of "attacks from" bitboards for a knight
		 */
		BUFFER(uint64, knight_attacks, 64);

		/**
		 * Returns the LSB for every possible unsigned 16-bit value
		 */
		BUFFER(int, lsb, 65536);

		/**
		 * Returns the MSB for every possible unsigned 16-bit value
		 */
		BUFFER(int, msb, 65536);

		/**
		 * All squares "north" of a particular square, from white's
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
		 * The values of each type of piece. See \ref value_t
		 */
		BUFFER(int, piece_value, 6);

		/**
		 * Returns the population count for every possible unsigned
		 * 16-bit value
		 */
		BUFFER(int, pop, 65536);

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
		BUFFER(int, rook_db_shifts, 64);

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
		 * Bits representing the queenside, i.e. for white this would be
		 * B1, C1, and D1
		 */
		BUFFER(uint64, queenside, 2);

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

	private:

		uint64 _compute_diag_attacks(int square, uint64 occupied) const;

		uint64 _compute_rook_attacks(int square, uint64 occupied) const;

		void _create_diag_attacks_database();

		void _create_rook_attacks_database();

		void _gen_bishop_masks();

		void _gen_occupancies_diag(
			int square, types::uint64_v& occupancy_set) const;

		void _gen_occupancies_rook(
			int square, types::uint64_v& occupancy_set) const;

		void _gen_rook_masks();

		uint64 _get_diag_a1h8(int square) const;

		uint64 _get_diag_h1a8(int square) const;

		uint64 _get_file(int square) const;

		uint64 _get_rank(int square) const;

		void _init_ep_targets();

		void _init_king_attacks();

		void _init_knight_attacks();

		void _init_magics();

		void _init_misc_masks();

		void _init_pawn_advances();

		void _init_pawn_attacks();

		void _init_piece_values();

		void _init_xsb();
	};
}

#endif
