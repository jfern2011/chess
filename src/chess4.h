#ifndef __CHESS_H__
#define __CHESS_H__

#include <memory>
#include <cstdint>

#include "util/Buffer.h"

namespace Chess
{
	template <typename T> using Handle = std::shared_ptr<T>;

	using uint64 = std::uint64_t;
	using uint32 = std::uint32_t;
	using uint16 = std::uint16_t;
	using uint8 = std::uint8_t;

	const int max_ply = 128;

	extern const char* SQUARE_STR[65];

	enum : uint64
	{
		rank_1 = 0xFF,
		rank_2 = rank_1 <<  8,
		rank_3 = rank_1 << 16,
		rank_4 = rank_1 << 24,
		rank_5 = rank_1 << 32,
		rank_6 = rank_1 << 40,
		rank_7 = rank_1 << 48,
		rank_8 = rank_1 << 56
	};

	enum : uint64
	{
		file_h = 0x0101010101010101,
		file_g = file_h << 1,
		file_f = file_h << 2,
		file_e = file_h << 3,
		file_d = file_h << 4,
		file_c = file_h << 5,
		file_b = file_h << 6,
		file_a = file_h << 7
	};

	enum
	{
		black = 0,
		white = 1
	};

	enum class direction_t
	{
		along_rank,
		along_file,
		along_a1h8,
		along_h1a8,
		none
	};

	enum class piece_t
	{
		pawn   = 0,
		rook   = 1,
		knight = 2,
		bishop = 3,
		queen  = 4,
		king   = 5,
		empty  = 6
	};

	enum
	{
		pawn_value   = 100,
		knight_value = 325,
		bishop_value = 325,
		rook_value   = 500,
		queen_value  = 975
	};

	enum
	{
		H1, G1, F1, E1, D1, C1, B1, A1,
		H2, G2, F2, E2, D2, C2, B2, A2,
		H3, G3, F3, E3, D3, C3, B3, A3,
		H4, G4, F4, E4, D4, C4, B4, A4,
		H5, G5, F5, E5, D5, C5, B5, A5,
		H6, G6, F6, E6, D6, C6, B6, A6,
		H7, G7, F7, E7, D7, C7, B7, A7,
		H8, G8, F8, E8, D8, C8, B8, A8,
		BAD_SQUARE
	};

	enum
	{
		castle_K = 0x1,
		castle_Q = 0x2
	};

	enum
	{
		castle_K_index = 0,
		castle_Q_index = 1
	};

	int abs(int value)
	{
		return value > 0 ? value : -value;
	}

	inline int flip(int to_move) { return to_move ^ 1; }

	inline int get_file(int a) { return a &  7; }

	inline int get_rank(int a) { return a >> 3; }

	/**
	 * @defgroup move_bits Move bit packing
	 *
	 * Moves are packed in 21 bits:
	 *
	 * 20...18: promotion piece
	 * 17...15: captured piece
	 * 14...12: piece moved
	 * 11... 6: destination square
	 *  5... 0: origin square
	 *
	 * @{
	 */

	/**
	 ******************************************************************
	 *
	 * Extract the piece promoted to from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The piece promoted to
	 *
	 ******************************************************************
	 */
	inline piece_t extract_promote(uint32 move)
	{
		return move >> 18;
	}

	/**
	 ******************************************************************
	 *
	 * Extract the piece captured from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The captured piece
	 *
	 ******************************************************************
	 */
	inline piece_t extract_captured(uint32 move)
	{
		return (move >> 15) & 0x07;
	}

	/**
	 ******************************************************************
	 *
	 * Extract the piece moved from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The moved piece
	 *
	 ******************************************************************
	 */
	inline piece_t extract_moved(uint32 move)
	{
		return (move >> 12) & 0x07;
	}

	/**
	 ******************************************************************
	 *
	 * Extract the destination square from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The destination square
	 *
	 ******************************************************************
	 */
	inline int extract_to(uint32 move)
	{
		return (move >> 6)  & 0x3f;
	}

	/**
	 ******************************************************************
	 *
	 * Extract the origin square from the given move bits
	 *
	 * @param[in] move The move to decode
	 *
	 * @return The origin square
	 *
	 ******************************************************************
	 */
	inline int extract_from(uint32 move)
	{
		return move & 0x3f;
	}

	/** @} */

	/**
	 * @defgroup SAFE_BUFFER Safe Buffering
	 *
	 * If compiling with SAFE_BUFFER, buffers declared with these macros
	 * will use the \ref Buffer class, which helps to debug buffer
	 * overflow errors. Otherwise, buffers will be declared in the usual
	 * way, e.g. int buf[10]. Note that you do not need to explicitly
	 * use these; see \ref Buffer
	 *
	 * @{
	 */
	#ifdef SAFE_BUFFER
		#define BUFFER_1(type, name, size1)               \
		 	Buffer<type,size1> name
		#define BUFFER_2(type, name, size1, size2)        \
		 	Buffer<type,size1,size2> name
		#define BUFFER_3(type, name, size1, size2, size3) \
		 	Buffer<type,size1,size2,size3> name
	#else
		#define BUFFER_1(type, name, size1)               \
		 	type name[size1]
		#define BUFFER_2(type, name, size1, size2)        \
		 	type name[size1][size2]
		#define BUFFER_3(type, name, size1, size2, size3) \
		 	type name[size1][size2][size3]
	#endif
	/** @} */

#ifndef DOXYGEN_SKIP
	#define num_args(_0, _1, _2, nargin, ...) nargin
	#define get_buf_dim(...) \
					  num_args(__VA_ARGS__, 3, 2, 1)
#endif

	/**
	 * Generic buffer selection. Use this to declare a buffer of 1, 2, or
	 * 3 dimensions
	 */
	#define BUFFER(type, name, ...) \
		_select(BUFFER, \
			get_buf_dim(__VA_ARGS__))(type, name, __VA_ARGS__)
}

#endif
