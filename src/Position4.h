#ifndef __POSITION_H__
#define __POSITION_H__

#include "chess_util4.h"
#include "DataTables4.h"
#include "OutputSource.h"

namespace Chess
{
	/**
	 * @class Position
	 *
	 * Represents a chess position
	 */
	class Position
	{
	public:
		/**
		 * Structure containing en passant information for
		 * a position
		 */
		struct EnPassant
		{
			/**
			 * The en passant target (i.e. "to") square
			 */
			square_t target;

			/**
			 * The origin square(s) from which a player may
			 * capture en passant
			 */
			BUFFER(square_t, src, 2);

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
			 * 1 integer for whose turn it is
			 */
			uint64 to_move;

			bool operator==(
				const HashInput& rhs) const;

			void clear();

			void print();
		};

		static constexpr char init_fen[] =
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	public:

		Position(Handle<std::ostream> stream,
				 const std::string& fen= init_fen);

		Position(const Position& other);

		~Position();

		Position& operator=( const Position& rhs );

		bool operator==(const Position& rhs) const;

		template <piece_t type>
		uint64 attacks_from(
			square_t square, player_t to_move= player_t::both) const;

		uint64 attacks_to  (square_t square, player_t to_move) const;

		bool can_castle_long( player_t to_move) const;

		bool can_castle_short(player_t to_move) const;

		bool equals(const Position& p, int ply) const;

		void generate_hash();

		template <piece_t type>
		uint64 get_bitboard( player_t to_move ) const;

		uint64 get_discover_ready(player_t to_move) const;

		std::string get_fen()         const;

		int get_fullmove_number()     const;

		uint64 get_hash_key(int  ply) const;

		uint64 get_hash_key()         const;

		square_t get_king_square (player_t to_move) const;

		int get_material() const;

		template <piece_t type>
		int get_mobility(square_t square);

		uint64 get_occupied( player_t to_move ) const;

		uint64 get_pinned_pieces (player_t to_move) const;

		player_t get_turn() const;

		bool in_check(player_t to_move) const;

		direction_t is_pinned(
			square_t square, player_t to_move ) const;

		bool make_move(int32 move);

		piece_t piece_on(square_t square) const;

		void print() const;

		bool reset(const std::string& fen = init_fen);

		void set_default();

		bool under_attack(
			square_t square, player_t to_move ) const;

		bool unmake_move (int32 move);

	private:

		/* Methods that compute sliding piece attacks  */

		uint64 _attacks_from_diag (square_t square,
			uint64 occupied) const;

		uint64 _attacks_from_queen(square_t square,
			uint64 occupied) const;

		uint64 _attacks_from_rook (square_t square,
			uint64 occupied) const;

		/* Methods that compute sliding piece mobility */

		int _get_diag_mobility (square_t square, uint64 occupied) const;

		int _get_queen_mobility(square_t square, uint64 occupied) const;

		int _get_rook_mobility (square_t square, uint64 occupied) const;

		void _update_hash(int32 move);

		bool _validate(
			const std::string& fen) const;

		/**
		 * Bitboards that give the locations of all bishops in the position
		 */
		BUFFER(uint64, _bishops, 2);

		/**
		 * Bitmasks describing the allowed castling abilities for each
		 * side
		 */
		BUFFER(char, _castle_rights, max_ply, 2);

		/**
		 * A record of en passant squares, indexed by ply
		 */
		BUFFER(EnPassant, _ep_info, max_ply);

		/**
		 * The full-move number (see FEN notation). Since this is really
		 * just informational, we don't bother incrementing it for
		 * null moves
		 */
		int _full_move;

		/**
		 * The half-move clock (see FEN notation). Note that we do not
		 * increment this for null moves
		 */
		int _half_move;

		/**
		 * A set of 64-bit integers used to generate a Zobrist hash key
		 * for this position
		 */
		HashInput _hash_input;

		/**
		 * Flag indicating this position is ready to use
		 */
		bool _is_init;

		/**
		 * Bitboards that give the locations of both kings
		 */
		BUFFER(uint64, _kings, 2);

		/**
		 * The square each king is on. This avoids having to \ref LSB the
		 * \ref _kings bitboard
		 */
		BUFFER(square_t, _king_sq, 2);

		/**
		 * Bitboards that give the locations of all knights in this
		 * position
		 */
		BUFFER(uint64, _knights, 2);

		/**
		 * The material balance. A value of zero indicates even material
		 */
		int _material;

		/**
		 *  Bitboards that give the locations of all squares occupied by
		 *  both sides
		 */
		BUFFER(uint64, _occupied, 2);

		/**
		 *  The output stream on which to write log messages
		 */
		Handle<OutputSource> _output;

		/**
		 * Bitboards that give the locations of all pawns in
		 * the position
		 */
		BUFFER(uint64, _pawns, 2);

		/**
		 * Tells us what piece is on each square
		 */
		BUFFER(piece_t, _pieces, 64);

		/**
		 * The current ply
		 */
		int _ply;

		/**
		 * Bitboards that give the locations of all queens in this
		 * position
		 */
		BUFFER(uint64, _queens, 2);

		/**
		 *  Bitboards that give the locations of all rooks in this
		 *  position
		 */
		BUFFER(uint64, _rooks, 2);

		/**
		 * A record of all hash signatures, indexed by ply
		 */
		BUFFER(uint64, _save_hash, max_ply);

		/**
		 * Whose turn it is to play
		 */
		player_t _to_move;
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
	inline bool Position::EnPassant::operator==(const EnPassant& rhs)
		const
	{
		return src[0] == rhs.src[0] && src[1] == rhs.src[1]
			&& target == rhs.target;
	}

	/**
	 * Set all members to their defaults
	 */
	inline void Position::EnPassant::clear()
	{
		target = src[0] = src[1] = square_t::BAD_SQUARE;
	}

	/**
	 * Clear all entries
	 */
	inline void Position::HashInput::clear()
	{
		for (int i = 0; i < 2; i++)
		{
			castle_rights[player_t::white][i] = 0;
			castle_rights[player_t::black][i] = 0;
		}

		for (int i = 0; i < 8; i++)
			en_passant[i] = 0;

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				piece[player_t::black][i][j] = 0;
				piece[player_t::white][i][j] = 0;
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
	inline bool Position::HashInput::operator==(
		const HashInput& rhs) const
	{
		bool same = true;

		for (int i = 0; i < 2; i++)
		{
			same = same &&
				castle_rights[player_t::white][i]
					== rhs.castle_rights[player_t::white][i];
			same = same && 
				castle_rights[player_t::black][i]
					== rhs.castle_rights[player_t::black][i];
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
				same = same && piece[player_t::black][i][j] ==
					rhs.piece[player_t::black][i][j];

				same = same && piece[player_t::white][i][j] ==
					rhs.piece[player_t::white][i][j];
			}
		}

		same = same &&
			to_move == rhs.to_move;

		return same;
	}

	/**
	 * Generates the squares attacked by the given piece located at the
	 * given square
	 *
	 * @tparam type  Which piece to generate attacked squares for
	 *
	 * @param[in] square   The square this piece is on
	 * @param[in] to_move  Generate an attacks board for this side (for
	 *                     pawn attacks)
	 *
	 * @return A bitboard specifying all squares attacked by this
	 *         piece from \a square
	 */
	template <piece_t type>
	inline uint64 Position::attacks_from(square_t square,
		player_t to_move) const
	{
		return 0;
	}

#ifndef DOXYGEN_SKIP
	template <>
	inline uint64 Position::attacks_from< piece_t::rook >(square_t square,
		player_t) const
	{
		return _attacks_from_rook(square, _occupied[player_t::white] |
										  _occupied[player_t::black]);
	}

	template <>
	inline uint64 Position::attacks_from<piece_t::knight>(square_t square,
		player_t) const
	{
		return DataTables::get().knight_attacks[square];
	}

	template <>
	inline uint64 Position::attacks_from<piece_t::bishop>(square_t square,
		player_t) const
	{
		return _attacks_from_diag(square, _occupied[player_t::white] |
										  _occupied[player_t::black]);
	}

	template <>
	inline uint64 Position::attacks_from< piece_t::pawn >(square_t square,
		player_t to_move) const
	{
		return DataTables::get().pawn_attacks[to_move][square];
	}

	template <>
	inline uint64 Position::attacks_from< piece_t::king >(square_t square,
		player_t) const
	{
		return DataTables::get().king_attacks[square];
	}

	template <>
	inline uint64 Position::attacks_from<piece_t::queen >(square_t square,
		player_t) const
	{
		const uint64 occ =
			_occupied[player_t::white] | _occupied[player_t::black];

		return _attacks_from_rook(square, occ) |
			   _attacks_from_diag(square, occ);
	}
#endif

	/**
	 * Get a bitboard containing 1-bits for each square occupied by \a
	 * to_move that has a piece attacking \a square
	 *
	 * @param[in] square  The square to examine
	 * @param[in] to_move The side whose attackers to the given square
	 *                    we want
	 *
	 * @return A bitboard of attackers for \a to_move
	 */
	inline uint64 Position::attacks_to(square_t square,
		player_t to_move) const
	{
		uint64 out = 0;

		const uint64 occupied = _occupied[0] | _occupied[1];

		auto& tables = DataTables::get();

		out |= tables.pawn_attacks[flip(to_move)][square]
				& _pawns[to_move];
		
		out |= tables.knight_attacks[square]
				& _knights[to_move];

		out |= _attacks_from_rook(square, occupied)
				& ( _rooks[ to_move ] | _queens[to_move] );

		out |= _attacks_from_diag(square, occupied)
				& ( _bishops[to_move] | _queens[to_move] );

		out |= tables.king_attacks[square]
				& _kings[to_move];

		return out;
	}

	/**
	 * Determine whether or not the given player has forfeited his ability
	 * to castle long
	 *
	 * @param[in] to_move The player of interest
	 *
	 * @return True if he may castle
	 */
	inline bool Position::can_castle_long(player_t to_move) const
	{
		return _castle_rights[_ply][to_move] & castle_Q;
	}

	/**
	 * Determine whether or not the given player has forfeited his ability
	 * to castle short
	 *
	 * @param[in] to_move The player of interest
	 *
	 * @return True if he may castle
	 */
	inline bool Position::can_castle_short(player_t to_move) const
	{
		return _castle_rights[_ply][to_move] & castle_K;
	}

	/**
	 * Get a bitboard representing all of a player's type of piece. For
	 * example, to get all of white's bishops:
	 *
	 * @code
	 * get_bitboard< piece_t::bishop >(player_t::white)
	 * @endcode
	 *
	 * @tparam type Specifies the piece type
	 *
	 * @param[in] to_move Specifies whose pieces to get
	 *
	 * @return A bitboard with 1 bit set for each square containing the
	 *         specified piece belonging to \a to_move
	 */
	template <piece_t type>
	inline uint64 Position::get_bitboard(player_t to_move) const
	{
		return 0;
	}

#ifndef DOXYGEN_SKIP
	template <>
	inline uint64 Position::get_bitboard< piece_t::rook >(
		player_t to_move) const
	{
		return _rooks[to_move];
	}

	template <>
	inline uint64 Position::get_bitboard<piece_t::knight>(
		player_t to_move) const
	{
		return _knights[to_move];
	}

	template <>
	inline uint64 Position::get_bitboard<piece_t::bishop>(
		player_t to_move) const
	{
		return _bishops[to_move];
	}

	template <>
	inline uint64 Position::get_bitboard< piece_t::pawn >(
		player_t to_move) const
	{
		return _pawns[to_move];
	}

	template <>
	inline uint64 Position::get_bitboard< piece_t::king >(
		player_t to_move) const
	{
		return _kings[to_move];
	}

	template <>
	inline uint64 Position::get_bitboard<piece_t::queen >(
		player_t to_move) const
	{
		return _queens[to_move];
	}
#endif

	/**
	 * Get a bitboard containing all pieces that, if moved, would uncover
	 * check on \a to_move
	 *
	 * @param[in] to_move The side whose king is vulnerable
	 *
	 * @return A bitboard with bits set for each square whose occupant is
	 *         ready to uncover check
	 */
	inline uint64 Position::get_discover_ready(player_t to_move) const
	{
		const uint64 occupied =
			_occupied[player_t::black] | _occupied[player_t::white];

		uint64 pinned =
			_attacks_from_queen(_king_sq[to_move], occupied)
				& _occupied[flip(to_move)];

		auto& tables = DataTables::get();

		for (uint64 temp = pinned; temp; )
		{
			const square_t sq = static_cast<square_t>( msb64(temp) );

			switch (tables.directions[sq][_king_sq[to_move]])
			{
			case direction_t::along_rank:
				if (!(_attacks_from_rook( sq, occupied)
						& tables.ranks64[sq]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_file:
				if (!(_attacks_from_rook( sq, occupied)
						& tables.files64[sq]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_a1h8:
				if (!(_attacks_from_diag(sq, occupied)
					  & tables.a1h8_64[sq]
					  & (_bishops[flip(to_move)] |
					  		 _queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_h1a8:
				if (!(_attacks_from_diag(sq, occupied)
					  & tables.h1a8_64[sq]
					  & (_bishops[flip(to_move)] |
					  		 _queens[flip(to_move)])))
					clear_bit64(sq, pinned);
			default:
				break;
			}

			clear_bit64(sq, temp);
		}

		return pinned;
	}

	/**
	 * Get the 64-bit Zobrist key associated with the position
	 * at the specified ply
	 *
	 * @param[in] ply Get the key at this ply
	 *
	 * @return The hash key
	 */
	inline uint64 Position::get_hash_key(int ply) const
	{
		return _save_hash[ply];
	}

	/**
	 * Get the 64-bit Zobrist key associated with the position
	 * at the current ply
	 *
	 * @return The hash key
	 */
	inline uint64 Position::get_hash_key() const
	{
		return _save_hash[_ply];
	}

	/**
	 * Get the square that the king belonging to \a to_move is
	 * currently on
	 *
	 * @param[in] to_move Get this guy's king
	 *
	 * @return  The king's location, which should be between 0
	 *          and 63 inclusive
	 */
	inline square_t Position::get_king_square(player_t to_move) const
	{
		return _king_sq[to_move];
	}

	/**
	 * Get the material balance. A positive value means white has
	 * more material
	 *
	 * @return The material balance
	 */
	inline int Position::get_material() const
	{
		return _material;
	}

	/**
	 * Get the mobility of a given piece on the given \a square. This
	 * is hashed to avoid computing it on the fly
	 *
	 * @param[in] square The piece's location
	 *
	 * @return The Hamming weight of \ref attacks_from(). For sliding
	 *         pieces only, this may be positive
	 */
	template <piece_t type>
	inline int Position::get_mobility(square_t square)
	{
		return 0;
	}

#ifndef DOXYGEN_SKIP
	template <>
	inline int Position::get_mobility< piece_t::rook >(square_t square)
	{
		return _get_rook_mobility(square, _occupied[player_t::white] |
										  _occupied[player_t::black]);
	}

	template <>
	inline int Position::get_mobility<piece_t::bishop>(square_t square)
	{
		return _get_diag_mobility(square, _occupied[player_t::white] |
										  _occupied[player_t::black]);
	}

	template <>
	inline int Position::get_mobility<piece_t::queen >(square_t square)
	{
		const uint64 occ =
			_occupied[player_t::white] | _occupied[player_t::black];

		return _get_diag_mobility(square, occ) |
			   _get_rook_mobility(square, occ);
	}
#endif

	/**
	 * Get a bitboard representing all squares occupied by \a to_move
	 *
	 * @param[in] to_move Get the squares occupied by this guy
	 *
	 * @return A bitboard with 1 bit set for each square containing a
	 *         piece belonging to \a to_move
	 */
	inline uint64 Position::get_occupied(player_t to_move) const
	{
		return _occupied[to_move];
	}

	/**
	 * Get a bitboard containing all pieces that are pinned on the king
	 * for the specified side
	 *
	 * @param[in] to_move Get pinned pieces for this side
	 *
	 * @return A bitboard with a bit set for each of the pinned squares
	 */
	inline uint64 Position::get_pinned_pieces(player_t to_move) const
	{
		const uint64 occupied =
			_occupied[player_t::black] | _occupied[player_t::white];

		uint64 pinned =
			_attacks_from_queen(_king_sq[to_move], occupied)
				& _occupied[to_move];

		auto& tables = DataTables::get();

		for (uint64 temp = pinned; temp; )
		{
			const square_t sq = static_cast<square_t>( msb64(temp) );

			switch (tables.directions[sq][_king_sq[to_move]])
			{
			case direction_t::along_rank:
				if (!(_attacks_from_rook(sq, occupied)
						& tables.ranks64[sq]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_file:
				if (!(_attacks_from_rook( sq, occupied)
						& tables.files64[sq]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_a1h8:
				if (!(_attacks_from_diag(sq, occupied)
					  & tables.a1h8_64[sq]
					  & (_bishops[flip(to_move)] |
					  		 _queens[flip(to_move)])))
					clear_bit64(sq, pinned);
				break;
			case direction_t::along_h1a8:
				if (!(_attacks_from_diag(sq, occupied)
					  & tables.h1a8_64[sq]
					  & (_bishops[flip(to_move)] |
					  		 _queens[flip(to_move)])))
					clear_bit64(sq, pinned);
			default:
				break;
			}

			clear_bit64(sq, temp);
		}

		return pinned;
	}

	/**
	 * Get the player whose turn it is to move in this position
	 *
	 * @return \ref player_t::white or \ref player_t::black
	 */
	inline player_t Position::get_turn() const
	{
		return _to_move;
	}

	/**
	 * Determine if the given side, \a to_move, is in check
	 *
	 * @param[in] to_move \ref player_t::white or \ref player_t::black
	 *
	 * @return True if \a to_move is in check
	 */
	inline bool Position::in_check(player_t to_move) const
	{
		return under_attack(_king_sq[to_move],
			flip(to_move));
	}

	/**
	 * Determine whether a piece on a particular square would be pinned
	 * on the king (if that piece were there)
	 *
	 * @param[in] square  The square of interest
	 * @param[in] to_move The player whose piece would be pinned
	 *
	 * @return The direction of the pin
	 */
	inline direction_t Position::is_pinned(square_t square,
		player_t to_move) const
	{
		const uint64 occupied =
			_occupied[player_t::black] | _occupied[player_t::white];

		auto& tables = DataTables::get();

		if (_attacks_from_queen(square,occupied) & _kings[to_move])
		{
			switch(tables.directions[ square ][_king_sq[ to_move ]])
			{
			case direction_t::along_rank:
				if (_attacks_from_rook( square, occupied)
						& tables.ranks64[square]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)]))

					return direction_t::along_rank;
				break;
			case direction_t::along_file:
				if (_attacks_from_rook( square, occupied)
						& tables.files64[square]
						& (_rooks[flip(to_move)] |
							_queens[flip(to_move)]))

					return direction_t::along_file;
				break;
			case direction_t::along_a1h8:
				if (_attacks_from_diag(square, occupied)
					  & tables.a1h8_64[square]
					  & (_bishops[flip(to_move)] |
					  			 _queens[flip(to_move)]))

					return direction_t::along_a1h8;
				break;
			case direction_t::along_h1a8:
				if (_attacks_from_diag(square, occupied)
					  & tables.h1a8_64[square]
					  & (_bishops[flip(to_move)] |
					  			 _queens[flip(to_move)]))

					return direction_t::along_h1a8;
			default:
				break;
			}
		}

		return direction_t::none;
	}

	/**
	 * Play the given move from the current position. Note that this does
	 * NOT check for legality
	 *
	 * @param[in] move The move data bits
	 *
	 * @return True on success
	 */
	inline bool Position::make_move(int32 move)
	{
		/*
		 * Update the position hash signature
		 */
		_update_hash(move);

		/*
		 * Before doing anything, carry over the castling
		 * rights to the next ply. Later, when we
		 * unMakeMove(), we'll have a record of what this
		 * was prior to making the move: 
		 */
		_castle_rights[_ply+1][player_t::black] =
				_castle_rights[_ply][player_t::black];
		_castle_rights[_ply+1][player_t::white] = 
				_castle_rights[_ply][player_t::white];

		_ply++;

		if (move == 0)
		{
			/*
			 * This is a null move. Switch sides and clear
			 * the en passant square:
			 */
			_to_move = flip(_to_move);
			_ep_info[_ply].clear();
			return true;
		}

		const piece_t captured = extract_captured(move);
		const int from         = extract_from(move);
		const piece_t moved    = extract_moved(move);
		const piece_t promote  = extract_promote(move);
		const int to           = extract_to(move);

		_pieces[from] = piece_t::empty;
		_pieces[to]   = moved;

		clear_set64(from, to, _occupied[_to_move] );

		/*
		 * Clear the en passant info as it is no longer
		 * valid:
		 */
		_ep_info[_ply].clear();

		int delta_material = 0;

		auto& tables = DataTables::get();

		switch (moved)
		{
			case piece_t::pawn:
				_pawns[_to_move] &= tables.clear_mask[from];

				if (promote != piece_t::empty)
				{
					_pieces[to] = promote;

					delta_material +=
						tables.piece_value[promote]-pawn_value;
				}

				switch (promote)
				{
					case piece_t::knight:
						_knights[_to_move]
							|= tables.set_mask[to];
						break;
					case piece_t::rook:
						_rooks[_to_move]
							|= tables.set_mask[to];
						break;
					case piece_t::queen:
						_queens[_to_move] 
							|= tables.set_mask[to];
						break;
					case piece_t::bishop:
						_bishops[_to_move]
							|= tables.set_mask[to];
						break;
					default:
						_pawns[_to_move]
							|= tables.set_mask[to];
				}

				/*
				 * Set the en passant target square:
				 */
				if (abs(from-to) == 16)
				{
					uint64 src =
						_pawns[flip(_to_move)] & tables.rank_adjacent[to];

					if (_to_move == player_t::white)
						_ep_info[_ply].target =
							static_cast<square_t>(to-8);
					else
						_ep_info[_ply].target =
							static_cast<square_t>(to+8);

					if (src & (tables.set_mask[to+1]))
						_ep_info[_ply].src[0] =
							static_cast<square_t>(to+1);
					if (src & (tables.set_mask[to-1]))
						_ep_info[_ply].src[1] =
							static_cast<square_t>(to-1);
				}

				break;

			case piece_t::knight:
				clear_set64(from, to, _knights[_to_move]);
				break;

			case piece_t::rook:
				clear_set64( from, to, _rooks[_to_move] );

				if ((_castle_rights[_ply][_to_move]))
				{
					switch (get_file(from))
					{
						/*
						 * Reduce castling rights since we moved
						 * this rook:
						 */
						case 0:
							_castle_rights[_ply][_to_move]
								&= castle_Q;
						break;
						case 7:
							_castle_rights[_ply][_to_move]
								&= castle_K;
					}
				}

				break;

			case piece_t::bishop:
				clear_set64(from, to, _bishops[_to_move]);
				break;

			case piece_t::queen:
				clear_set64(from, to, _queens[_to_move] );
				break;

			case piece_t::king:
				clear_set64( from, to, _kings[_to_move] );
				_king_sq[_to_move] =
					static_cast<square_t>(to);

				if (abs(from-to) == 2)
				{
					/*
				 	 * This was a castle move - update
				 	 * the rook data:
				 	 */
					if (_to_move == player_t::white)
					{
						if (to == square_t::G1)
						{
							_pieces[square_t::H1] = piece_t::empty;
							_pieces[square_t::F1] = piece_t::rook;

							clear_set64(square_t::H1, square_t::F1,
								_rooks[player_t::white]);
							clear_set64(square_t::H1, square_t::F1,
								_occupied[player_t::white]);
						}
						else // Queenside castle
						{
							_pieces[square_t::A1] = piece_t::empty;
							_pieces[square_t::D1] = piece_t::rook;

							clear_set64(square_t::A1, square_t::D1,
								_rooks[player_t::white]);
							clear_set64(square_t::A1, square_t::D1,
								_occupied[player_t::white]);
						}
					}
					else
					{
						if (to == square_t::G8)
						{
							_pieces[square_t::H8] = piece_t::empty;
							_pieces[square_t::F8] = piece_t::rook;

							clear_set64(square_t::H8, square_t::F8,
								_rooks[player_t::black]);
							clear_set64(square_t::H8, square_t::F8,
								_occupied[player_t::black]);
						}
						else // Queenside castle
						{
							_pieces[square_t::A8] = piece_t::empty;
							_pieces[square_t::D8] = piece_t::rook;

							clear_set64(square_t::A8, square_t::D8,
								_rooks[player_t::black]);
							clear_set64(square_t::A8, square_t::D8,
								_occupied[player_t::black]);
						}
					}
				}

				/*
				 * Clear all castling rights for this
				 * player:
				 */
				_castle_rights[_ply][_to_move] = 0;
				break;
			default:
				Abort(false);
		}

		if (captured != piece_t::empty)
		{
			delta_material += tables.piece_value[captured];

			switch (captured)
			{
				case piece_t::pawn:

					if (_occupied[flip(_to_move)] & tables.set_mask[to])
					{
						_pawns[flip(_to_move)] &= tables.clear_mask[to];
					}
					else
					{
						/*
						 * This was an en passant capture:
						 */
						if (_to_move == player_t::white)
						{
							_pieces[to-8] = piece_t::empty;

							_occupied[player_t::black] &= tables.clear_mask[to-8];
							_pawns[player_t::black] &=
								tables.clear_mask[to-8];
						}
						else
						{
							_pieces[to+8] = piece_t::empty;

							_occupied[player_t::white] &= tables.clear_mask[to+8];
							_pawns[player_t::white] &=
								tables.clear_mask[to+8];
						}
					}

					break;

				case piece_t::knight:

					_knights[flip(_to_move)] &= tables.clear_mask[to];
					break;

				case piece_t::bishop:
					_bishops[flip(_to_move)] &= tables.clear_mask[to];
					break;

				case piece_t::queen:
					_queens[flip(_to_move)]  &= tables.clear_mask[to];
					break;

				case piece_t::rook:
					_rooks[ flip(_to_move) ] &= tables.clear_mask[to];

					/*
					 * Update the opponent's castling rights if he could
					 * have castled with this rook:
					 */
					if ((_castle_rights[_ply][flip(_to_move)]))
					{
						/*
						 * Remove castling rights for the rook that
						 * was captured:
						 */
						switch (get_file(to))
						{
							case 0:
								_castle_rights[_ply][flip(_to_move)]
									&= castle_Q;
							break;
							case 7:
								_castle_rights[_ply][flip(_to_move)]
									&= castle_K;
						}
					}

					break;
				default:
					Abort(false);
			}

			// Update the enemy occupancy:
			_occupied[flip(_to_move)] &=
					tables.clear_mask[to];
		}

		else if (moved != piece_t::pawn)
			_half_move++;

		if ( _to_move == player_t::black )
			_full_move++;

		/*
		 * Update the material balance:
		 */
		if (_to_move == player_t::white)
			_material += delta_material;
		else
			_material -= delta_material;

		_to_move = flip(_to_move);
		return true;
	}

	/**
	 * Fetch the piece that is on a particular square, returning one of
	 * the piece types defined in chess.h
	 *
	 * @param[in] square The square of interest
	 *
	 * @return The piece on \a square
	 */
	inline piece_t Position::piece_on(square_t square) const
	{
		return _pieces[square];
	}

	/**
	 * Determine if the given square is being attacked by the specified
	 * side
	 *
	 * @param[in] square   The square to examine
	 * @param[in] to_move  Check if this side is attacking the square
	 *
	 * @return  True if the square is under attack by \a to_move, false
	 *          otherwise
	 */
	inline bool Position::under_attack(square_t square,
		player_t to_move) const
	{
		auto& tables = DataTables::get();

		if (tables.pawn_attacks[flip(to_move)][square]
				& _pawns[to_move])
			return true;

		if (tables.king_attacks[ square ] & _kings[ to_move ])
			return true;

		if (tables.knight_attacks[square] & _knights[to_move])
			return true;

		const uint64 rooks_queens   =
			_rooks[ to_move ] | _queens[to_move];
		const uint64 bishops_queens =
			_bishops[to_move] | _queens[to_move];

		uint64 rook_attackers =
			attacks_from< piece_t::rook >(square);

		if ( rook_attackers & rooks_queens )
			return true;

		uint64 diag_attackers = 
			attacks_from<piece_t::bishop>(square);

		if (diag_attackers & bishops_queens)
			return true;

		return false;
	}

	/**
	 * Undo the given move from the current position. This is essentially
	 * the inverse of \ref make_move()
	 *
	 * @param[in] move The move data bits
	 *
	 * @return True on success
	 */
	inline bool Position::unmake_move(int32 move)
	{
		/*
		 * Back up to the previous ply to restore castling,
		 * en passant info, and the hash signature
		 */
		_ply--;

		_to_move = flip(_to_move);

		/*
		 * If this is a null move, we are done
		 */
		if (move == 0)
			return true;

		const piece_t captured = extract_captured(move);
		const int from         = extract_from(move);
		const piece_t moved    = extract_moved(move);
		const piece_t promote  = extract_promote(move);
		const int to           = extract_to(move);

		/*
		 * Restore the piece locations
		 */
		_pieces[from] = moved;
		_pieces[to]   = captured;

		int delta_material = 0;

		/*
		 * Restore the occupancy bits for this player
		 */
		clear_set64(to, from, _occupied[_to_move]);

		auto& tables = DataTables::get();

		switch (moved)
		{
			case piece_t::pawn:
				_pawns[_to_move] |= tables.set_mask[from];

				if (promote != piece_t::empty)
				{
					delta_material +=
						tables.piece_value[promote] - pawn_value;
				}

				switch (promote)
				{
					case piece_t::knight:
						_knights[_to_move]
							&= tables.clear_mask[to];
						break;
					case piece_t::rook:
						_rooks[_to_move]
							&= tables.clear_mask[to];
						break;
					case piece_t::queen:
						_queens[_to_move]  
							&= tables.clear_mask[to];
						break;
					case piece_t::bishop:
						_bishops[_to_move] 
							&= tables.clear_mask[to];
						break;
					default:
						_pawns[ _to_move ]
							&= tables.clear_mask[to];
				}

				break;

			case piece_t::knight:
				clear_set64(to, from, _knights[_to_move]);
				break;

			case piece_t::rook:
				clear_set64( to, from, _rooks[_to_move] );
				break;

			case piece_t::bishop:
				clear_set64(to, from, _bishops[_to_move]);
				break;

			case piece_t::queen:
				clear_set64( to, from, _queens[_to_move]);
				break;

			case piece_t::king:
				clear_set64( to, from, _kings[_to_move] );
				_king_sq[_to_move] =
					static_cast<square_t>(from);

				/*
				 * Check if this was a castle move and update
				 * the rook bits accordingly:
				 */
				if (abs(from-to) == 2)
				{
					if (_to_move == player_t::white)
					{
						if (to == square_t::G1)
						{
							_pieces[square_t::F1] = piece_t::empty;
							_pieces[square_t::H1] = piece_t::rook;

							clear_set64(square_t::F1, square_t::H1,
								_rooks[player_t::white]);
							clear_set64(square_t::F1, square_t::H1,
								_occupied[player_t::white]);
						}
						else // Queenside castle
						{
							_pieces[square_t::D1] = piece_t::empty;
							_pieces[square_t::A1] = piece_t::rook;

							clear_set64(square_t::D1, square_t::A1,
								_rooks[player_t::white]);
							clear_set64(square_t::D1, square_t::A1,
								_occupied[player_t::white]);
						}
					}
					else
					{
						if (to == square_t::G8)
						{
							_pieces[square_t::F8] = piece_t::empty;
							_pieces[square_t::H8] = piece_t::rook;

							clear_set64(square_t::F8, square_t::H8,
								_rooks[player_t::black]);
							clear_set64(square_t::F8, square_t::H8,
								_occupied[player_t::black]);
						}
						else // Queenside castle
						{
							_pieces[square_t::D8] = piece_t::empty;
							_pieces[square_t::A8] = piece_t::rook;

							clear_set64(square_t::D8, square_t::A8,
								_rooks[player_t::black]);
							clear_set64(square_t::D8, square_t::A8,
								_occupied[player_t::black]);
						}
					}
				}

				break;
			default:
				Abort(false);
		}

		/*
		 * Restore the opponent's board info if this
		 * was a capture
		 */
		if (captured != piece_t::empty)
		{
			delta_material += tables.piece_value[captured];

			/*
			 * Restore the enemy occupancy:
			 */
			_occupied[flip(_to_move)] |=
					tables.set_mask[ to ];

			switch (captured)
			{
				case piece_t::pawn:

					if (to == _ep_info[_ply].target)
					{
						//  This was an en passant capture:

						_occupied[flip(_to_move)] &=
					  				tables.clear_mask[to];

						if (_to_move == player_t::white)
						{
							_pieces[to-8] = piece_t::pawn;

							_occupied[player_t::black]
								|= tables.set_mask[to-8];
							_pawns[player_t::black]
								|= tables.set_mask[to-8];
						}
						else
						{
							_pieces[to+8] = piece_t::pawn;

							_occupied[player_t::white]
								|= tables.set_mask[to+8];
							_pawns[player_t::white]
								|= tables.set_mask[to+8];
						}

						_pieces[to] = piece_t::empty;
					}
					else
					{
						_pawns[flip(_to_move)] |=
							tables.set_mask[to];
					}

					break;

				case piece_t::knight:
					_knights[flip(_to_move)] |= tables.set_mask[to];
					break;
				case piece_t::queen:
					_queens[flip(_to_move)]  |= tables.set_mask[to];
					break;
				case piece_t::rook:
					_rooks[ flip(_to_move) ] |= tables.set_mask[to];
					break;
				case piece_t::bishop:
					_bishops[flip(_to_move)] |= tables.set_mask[to];
					break;
				default:
					Abort(false);
			}
		}
		else if (moved != piece_t::pawn)
			_half_move--;

		if (_to_move == player_t::black)
			_full_move--;

		/*
		 * Restore the material balance:
		 */
		if (_to_move == player_t::white)
			_material -= delta_material;
		else
			_material += delta_material;

		return true;
	}

	/**
	 * Generates the squares attacked by a bishop located at the given
	 * square
	 *
	 * @param[in] square   The bishop's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return A bitboard specifying all squares attacked by this piece
	 */
	inline uint64 Position::_attacks_from_diag(square_t square,
		uint64 occupied ) const
	{
		auto& tables = DataTables::get();

		return tables.bishop_attacks[tables.bishop_offsets[square] +
				(((occupied & tables.bishop_attacks_mask[square])
					* tables.diag_magics[square]) >> tables.bishop_db_shifts[square])];
	}

	/**
	 * Generates the squares attacked by a queen located at the given
	 * square
	 *
	 * @param[in] square   The queen's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return  A bitboard showing all squares attacked by this piece
	 */
	inline uint64 Position::_attacks_from_queen(square_t square,
		uint64 occupied) const
	{
		return _attacks_from_rook(square, occupied)
				| _attacks_from_diag( square, occupied );
	}

	/**
	 * Generates the squares attacked by a rook located at the given
	 * square
	 *
	 * @param[in] square   The rook's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return A bitboard showing all squares attacked by this piece
	 */
	inline uint64 Position::_attacks_from_rook(square_t square,
		uint64 occupied) const
	{
		auto& tables = DataTables::get();

		return tables.rook_attacks[tables.rook_offsets[square] +
				(((occupied & tables.rook_attacks_mask[square])
					* tables.rook_magics[square]) >> tables.rook_db_shifts[square])];
	}

	/**
	 * Get the mobility of a bishop on \a square. This is hashed to avoid
	 * computing it on the fly
	 *
	 * @param[in] square   The bishop's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return The Hamming weight of \ref _attacks_from_diag()
	 */
	inline int Position::_get_diag_mobility(square_t square,
		uint64 occupied) const
	{
		auto& tables = DataTables::get();

		return tables.bishop_mobility[tables.bishop_offsets[square] +
				(((occupied & tables.bishop_attacks_mask[square])
					* tables.diag_magics[square]) >> tables.bishop_db_shifts[square])];
	}

	/**
	 * Get the mobility of a queen on \a square. This is hashed to avoid
	 * computing it on the fly
	 *
	 * @param[in] square   The queen's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return The Hamming weight of \ref attacks_from_queen()
	 */
	inline int Position::_get_queen_mobility(square_t square,
		uint64 occupied) const
	{
		return _get_rook_mobility(square, occupied)
				+ _get_diag_mobility(square, occupied);
	}

	/**
	 * Returns the mobility of a rook on \a square. This is hashed to avoid
	 * computing it on the fly
	 *
	 * @param[in] square   The rook's location
	 * @param[in] occupied The occupied squares bitboard
	 *
	 * @return The Hamming weight of \ref attacks_from_rook()
	 */
	inline int Position::_get_rook_mobility(square_t square,
		uint64 occupied) const
	{
		auto& tables = DataTables::get();

		return tables.rook_mobility[tables.rook_offsets[square] +
				(((occupied & tables.rook_attacks_mask[square])
					* tables.rook_magics[square]) >> tables.rook_db_shifts[square])];
	}

	/**
	 * Update the hash signature at the current ply. This should only
	 * be called when making a move
	 *
	 * @param[in] move The last move made
	 */
	inline void Position::_update_hash(int32 move)
	{
		uint64& hash = _save_hash[_ply+1];
		hash = _save_hash[_ply];

		const piece_t captured = extract_captured(move);
		const int from         = extract_from(move);
		const piece_t moved    = extract_moved(move);
		const piece_t promote  = extract_promote(move);
		const int to           = extract_to(move);

		/*
		 * If we had a valid en passant square for the current ply,
		 * remove it from the hash key:
		 */
		if (_ep_info[_ply].target != square_t::BAD_SQUARE)
		{
			hash ^=
			  _hash_input.en_passant[get_file(_ep_info[_ply].target)];
		}

		/*
		 * If this is a null move, update the hash to reflect
		 * the side on move and exit
		 */
		if (move == 0)
		{
			hash ^= _hash_input.to_move;
			return;
		}

		/*
		 * XOR in the en passant value if we advanced a pawn by
		 * two squares
		 */
		if (abs(from - to) == 16)
		{
			hash ^= _hash_input.en_passant[ get_file(to) ];
		}

		/*
		 * Update the hash entry to reflect the new location of
		 * the piece moved:
		 */
		hash ^=
			_hash_input.piece[_to_move][moved][from];

		if (promote == piece_t::empty)
			hash ^= _hash_input.piece[_to_move][ moved ][to];
		else
			hash ^= _hash_input.piece[_to_move][promote][to];

		/*
		 * Update the hash key if we moved a rook with which we
		 * could have castled
		 */
		if (moved == piece_t::rook)
		{
			if (get_file(from) == 0 &&
				(_castle_rights[_ply][_to_move] & castle_K))
			{
				hash ^=
					_hash_input.castle_rights[_to_move][castle_K_index];
			}
			else if (get_file(from) == 7 &&
				(_castle_rights[_ply][_to_move] & castle_Q))
			{
				hash ^=
					_hash_input.castle_rights[_to_move][castle_Q_index];
			}
		}

		/*
		 * Update the hash key to reflect new castling rights
		 * if the king was moved
		 */
		else if (moved == piece_t::king)
		{
			if (_castle_rights[_ply][_to_move] & castle_K)
				hash ^=
					_hash_input.castle_rights[_to_move][castle_K_index];
			if (_castle_rights[_ply][_to_move] & castle_Q)
				hash ^=
					_hash_input.castle_rights[_to_move][castle_Q_index];

			/*
			 * Update the hash key for the rook involved with
			 * the castle
			 */
			if (abs(from-to) == 2)
			{
				if (_to_move == player_t::white)
				{
					if (to == square_t::G1)
					{
						hash ^=
							_hash_input.piece[player_t::white][piece_t::rook][square_t::H1];
						hash ^=
							_hash_input.piece[player_t::white][piece_t::rook][square_t::F1];
					}
					else
					{
						hash ^=
							_hash_input.piece[player_t::white][piece_t::rook][square_t::A1];
						hash ^=
							_hash_input.piece[player_t::white][piece_t::rook][square_t::D1];
					}
				}
				else
				{
					if (to == square_t::G8)
					{
						hash ^=
							_hash_input.piece[player_t::black][piece_t::rook][square_t::H8];
						hash ^=
							_hash_input.piece[player_t::black][piece_t::rook][square_t::F8];
					}
					else
					{
						hash ^=
							_hash_input.piece[player_t::black][piece_t::rook][square_t::A8];
						hash ^=
							_hash_input.piece[player_t::black][piece_t::rook][square_t::D8];
					}
				}
			}
		}

		/*
		 * Next, Update the hash if an opponent's piece was captured
		 */
		if (captured != piece_t::empty)
		{
			const player_t x_side = flip(_to_move);

			switch (captured)
			{
			case piece_t::pawn:

				if (_pieces[to] != piece_t::empty)
					hash ^= _hash_input.piece[x_side][piece_t::pawn][to];
				else
				{
					if (_to_move == player_t::white)
						hash ^=
							_hash_input.piece[player_t::black][piece_t::pawn][to-8];
					else
						hash ^=
							_hash_input.piece[player_t::white][piece_t::pawn][to+8];
				}
				
				break;

			case piece_t::rook:

				/*
				 * Update the hash key if we captured a rook with which
				 * the opponent could have castled
				 */
				switch (get_file(to))
				{
				case 0:
					if (_castle_rights[_ply][x_side] & castle_K)
					{
						hash ^=
							_hash_input.castle_rights[x_side][castle_K_index];
					}
				break;

				case 7:
					if (_castle_rights[_ply][x_side] & castle_Q)
					{
						hash ^=
							_hash_input.castle_rights[x_side][castle_Q_index];
					}
				}

				break;

			default:

				hash ^= _hash_input.piece[
					flip(_to_move)][captured][to];
			}
		}

		/*
		 * Update the hash to reflect whose turn it is
		 */
		hash ^=
			_hash_input.to_move;
	}
}

#endif
