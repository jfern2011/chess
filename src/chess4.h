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
	using int32  = std::int32_t;
	using uint16 = std::uint16_t;
	using uint8 = std::uint8_t;

	const int max_ply      = 128;
	const int pawn_value   = 100;
	const int knight_value = 325;
	const int bishop_value = 325;
	const int rook_value   = 500;
	const int queen_value  = 975;

	const int castle_K = 0x1;
	const int castle_Q = 0x2;

	const int castle_K_index = 0;
	const int castle_Q_index = 1;

	const uint64 rank_1 = 0x00000000000000ff;

	const uint64 rank_2 = rank_1 <<  8;
	const uint64 rank_3 = rank_1 << 16;
	const uint64 rank_4 = rank_1 << 24;
	const uint64 rank_5 = rank_1 << 32;
	const uint64 rank_6 = rank_1 << 40;
	const uint64 rank_7 = rank_1 << 48;
	const uint64 rank_8 = rank_1 << 56;

	const uint64 file_h = 0x0101010101010101;

	const uint64 file_g = file_h << 1;
	const uint64 file_f = file_h << 2;
	const uint64 file_e = file_h << 3;
	const uint64 file_d = file_h << 4;
	const uint64 file_c = file_h << 5;
	const uint64 file_b = file_h << 6;
	const uint64 file_a = file_h << 7;

	extern const char* square_str[65];

	enum class direction_t
	{
		along_rank,
		along_file,
		along_a1h8,
		along_h1a8,
		none
	};

	struct player_e
	{
		enum player_t
		{
			black,
			white,
			both
		};
	};
	typedef player_e::player_t player_t;

	struct piece_e
	{
		enum piece_t : int32
		{
			pawn, rook, knight, bishop, queen, king, empty
		};
	};
	typedef piece_e::piece_t piece_t;

	struct square_e
	{
		enum square_t : int32
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
	};
	typedef square_e::square_t square_t;

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
