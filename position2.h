#ifndef __POSITION__
#define __POSITION__

#include <cctype>
#include <cstring>
#include <string>

#include "chess_util.h"

/**
 **********************************************************************
 *
 * @class Position
 *
 * Represents a chess position
 *
 **********************************************************************
 */
class Position
{
	friend class MoveGen;

	/**
	 * Structure containing en passant information for
	 * a position
	 */
	struct EnPassant
	{
		/**
		 * The en passant target (i.e. "to") square
		 */
		int32 target;

		/**
		 * The origin square(s) from which a player may
		 * capture en passant
		 */
		BUFFER(int32, src, 2);

		/**
		 * Set all members to their defaults
		 */
		inline void clear()
		{
			target = src[0] = src[1]
				= BAD_SQUARE;
		}

		/**
		 * Perform a byte-wise comparison between this object
		 * and another
		 *
		 * @param[in] rhs The object to compare against
		 *
		 * @return True if this EnPassant is the same as \a
		 *         rhs
		 */
		bool operator==(const EnPassant& rhs) const
		{
			bool same = ( target == rhs.target );

			for (int i = 0; i < 2; i++)
			{
				same =
					same && src[i] == rhs.src[i];
			}

			return same;
		}

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

		/**
		 * Clear all entries
		 */
		void clear()
		{
			for (int i = 0; i < 2; i++)
			{
				castle_rights[WHITE][i] = 0;
				castle_rights[BLACK][i] = 0;
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
					castle_rights[WHITE][i]
						== rhs.castle_rights[WHITE][i];
				same = same && 
					castle_rights[BLACK][i]
						== rhs.castle_rights[BLACK][i];
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
	};

public:

	Position(const DataTables& tables, bool xboard);

	Position(const DataTables& tables, const std::string & fen,
			 bool xboard);

	Position(const Position& other);

	~Position();

	Position& operator=( const Position& rhs );

	bool operator==(const Position& rhs) const;

	uint64 attacks_from(int square, piece_t piece,
		int to_move) const;

	uint64 attacks_from_bishop(int square, uint64 occupied) const;

	uint64 attacks_from_queen( int square, uint64 occupied) const;

	uint64 attacks_from_rook(  int square, uint64 occupied) const;

	uint64 attacks_to(int square, int to_move) const;

	bool can_castle_long( int to_move) const;

	bool can_castle_short(int to_move) const;

	void generate_hash();

	int get_bishop_mobility(int square, uint64 occupied) const;

	uint64 get_bishops(int to_move) const;

	uint64 get_bishops() const;

	uint64 get_discover_ready(int to_move) const;

	std::string get_fen() const;

	int get_fullmove_number() const;

	uint64 get_hash_key() const;

	uint64 get_hash_key(int ply) const;

	int get_king_square(int to_move) const;

	uint64 get_kings(int to_move) const;

	uint64 get_knights(int to_move) const;

	int get_material() const;

	uint64 get_occupied(int to_move) const;

	uint64 get_occupied() const;

	uint64 get_pawns(int to_move) const;

	uint64 get_pinned_pieces(int to_move) const;

	int get_queen_mobility( int square, uint64 occupied) const;

	uint64 get_queens(int to_move) const;

	uint64 get_queens() const;

	int get_rook_mobility(  int square, uint64 occupied) const;

	uint64 get_rooks(int to_move) const;

	uint64 get_rooks() const;

	int32 get_turn() const;

	bool in_check(int to_move) const;

	direction_t is_pinned(int square, int to_move) const;

	bool make_move(int move);

	piece_t piece_on(int square) const;

	void print() const;

	bool reset(const std::string& fen, bool xboard);

	bool reset(bool xboard);

	void set_default();

	bool under_attack(int square, int to_move) const;

	bool unmake_move(int move);

	void update_hash(int move);

	bool validate(const std::string& fen,
		bool xboard) const;

private:

	/**
	 * Bitboards that give the locations of all bishops in the position
	 */
	BUFFER(uint64, _bishops, 2);

	/**
	 * Bitmasks describing the allowed castling abilities for each
	 * side
	 */
	BUFFER(char, _castle_rights, MAX_PLY, 2);

	/**
	 * A record of en passant squares, indexed by ply
	 */
	BUFFER(EnPassant, _ep_info, MAX_PLY);

	/**
	 * The full-move number (see FEN notation). Since this is really
	 * just informational, we don't bother incrementing it for
	 * null moves
	 */
	int32 _full_move;

	/**
	 * The half-move clock (see FEN notation). Note that we do not
	 * increment this for null moves
	 */
	int32 _half_move;

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
	BUFFER(int32, _king_sq, 2);

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
	 * Bitboards that give the locations of all squares occupied by
	 * both sides
	 */
	BUFFER(uint64, _occupied, 2);

	/**
	 * Bitboards that give the locations of all pawns in the position
	 */
	BUFFER(uint64, _pawns, 2);

	/**
	 * Tells us what piece is on each square
	 */
	BUFFER(piece_t, _pieces, 64);

	/**
	 * The current ply
	 */
	int32 _ply;

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
	BUFFER(uint64, _save_hash, MAX_PLY);

	/**
	 * The global set of databases
	 */
	const DataTables& _tables;

	/**
	 * Whose turn it is to play
	 */
	int32 _to_move;
};

/*
 * The following functions are inlined for performance reasons
 */

/**
 * Generates the squares attacked by the given piece located at the
 * given square
 *
 * @param[in] square   The square this piece is on
 * @param[in] piece    What piece to generate attacked squares for
 * @param[in] to_move  Side to move
 *
 * @return A bitboard specifying all squares attacked by this piece
 *         or ~0 on error
 */
inline uint64 Position::attacks_from(int square, piece_t piece,
	int to_move) const
{
	uint64 occ = _occupied[WHITE] | _occupied[BLACK];
	switch (piece)
	{
		case ROOK:
			return attacks_from_rook(square, occ);
		case KNIGHT:
			return _tables.knight_attacks[square];
		case BISHOP:
			return
				attacks_from_bishop( square, occ);
		case PAWN:
			return _tables.pawn_attacks[to_move][square];
		case KING:
			return _tables.king_attacks[square];
		case QUEEN:
			return attacks_from_rook(square, occ) |
				   attacks_from_bishop(square, occ);
		default:
			AbortIf(true, ~0,
				"Position::attacks_from(): Invalid piece");
	}

	return 0;
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
inline uint64 Position::attacks_from_bishop( int square,
	uint64 occupied ) const
{
	return _tables.bishop_attacks[_tables.bishop_offsets[square] +
			(((occupied & _tables.bishop_attacks_mask[square])
				* _tables.diag_magics[square]) >> _tables.bishop_db_shifts[square])];
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
inline uint64 Position::attacks_from_queen(int square,
	uint64 occupied) const
{
	return attacks_from_rook(square, occupied)
			| attacks_from_bishop( square, occupied );
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
inline uint64 Position::attacks_from_rook(int square,
	uint64 occupied) const
{
	return _tables.rook_attacks[_tables.rook_offsets[square] +
			(((occupied & _tables.rook_attacks_mask[square])
				* _tables.rook_magics[square]) >> _tables.rook_db_shifts[square])];
}

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
inline uint64 Position::attacks_to(int square, int to_move) const
{
#if defined(DEBUG)
	AbortIfNot(_is_init, false);
#endif

	uint64 out = 0;

	const uint64 occupied = _occupied[0] | _occupied[1];

	out |= _tables.pawn_attacks[flip(to_move)][square]
			& _pawns[to_move];
	
	out |= _tables.knight_attacks[square]
			& _knights[to_move];

	out |= attacks_from_rook(square, occupied)
			& ( _rooks[ to_move ] | _queens[to_move] );

	out |= attacks_from_bishop(square, occupied)
			& ( _bishops[to_move] | _queens[to_move] );

	out |= _tables.king_attacks[square]
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
inline bool Position::can_castle_long(int to_move) const
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
inline bool Position::can_castle_short(int to_move) const
{
	return _castle_rights[_ply][to_move] & castle_K;
}

/**
 * Get the mobility of a bishop on \a square. This is hashed to avoid
 * computing it on the fly
 *
 * @param[in] square   The bishop's location
 * @param[in] occupied The occupied squares bitboard
 *
 * @return The Hamming weight of \ref attacks_from_bishop()
 */
inline int Position::get_bishop_mobility(int square,
	uint64 occupied) const
{
	return _tables.bishop_mobility[_tables.bishop_offsets[square] +
			(((occupied & _tables.bishop_attacks_mask[square])
				* _tables.diag_magics[square]) >> _tables.bishop_db_shifts[square])];
}

/**
 * Get a bitboard representing all bishops belonging to \a to_move
 *
 * @param[in] to_move Whose bishops to get
 *
 * @return  A bitboard with 1 bit set for each square containing a
 *          bishop belonging to \a to_move
 */
inline uint64 Position::get_bishops(int to_move) const
{
	return _bishops[to_move];
}

/**
 * Get a bit board representing all bishops on board
 *
 * @return A bitboard with 1 bit set for each square
 *         containing a bishop
 */
inline uint64 Position::get_bishops() const
{
	return _bishops[BLACK] | _bishops[WHITE];
}

/**
 * Get a bitboard containing all pieces that, if moved, would uncover
 * check on \a to_move
 *
 * @param[in] to_move The side whose king is vulnerable
 *
 * @return A bitboard with bits set for each square whose occupant is
 *         ready to uncover check
 */
inline uint64 Position::get_discover_ready(int to_move) const
{
	const uint64 occupied =
		_occupied[0] | _occupied[1];

	uint64 pinned =
		attacks_from_queen(_king_sq[to_move], occupied)
			& _occupied[flip(to_move)];

	for (uint64 temp = pinned; temp; )
	{
		const int sq = Util::msb64(temp);

		switch (data_tables.directions[sq][_king_sq[to_move]])
		{
		case ALONG_RANK:
			if (!(attacks_from_rook( sq, occupied)
					& data_tables.ranks64[sq]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_FILE:
			if (!(attacks_from_rook( sq, occupied)
					& data_tables.files64[sq]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_A1H8:
			if (!(attacks_from_bishop(sq, occupied)
				  & data_tables.a1h8_64[sq]
				  & (_bishops[flip(to_move)] |
				  		 _queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_H1A8:
			if (!(attacks_from_bishop(sq, occupied)
				  & data_tables.h1a8_64[sq]
				  & (_bishops[flip(to_move)] |
				  		 _queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
		default:
			break;
		}

		Util::clear_bit64(sq, temp);
	}

	return pinned;
}

/**
 * Get the 64-bit Zobrist key associated with this position
 *
 * @return The hash key
 */
inline uint64 Position::get_hash_key() const
{
#if defined(DEBUG)
	AbortIfNot(_is_init, ~0);
#endif

	return(_save_hash[_ply]);
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
#if defined(DEBUG)
	AbortIf(ply < 0 ||  MAX_PLY <= ply, ~0);
#endif

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
inline int Position::get_king_square(int to_move) const
{
	return _king_sq[to_move];
}

/**
 * Get a bitboard that represents the king belonging to \a to_move
 *
 * @param[in] to_move Whose king to get
 *
 * @return A bitboard with 1 bit set for the square containing the
 *         the king belonging to \a to_move
 */
inline uint64 Position::get_kings(int to_move) const
{
	return _kings[to_move];
}

/**
 * Get a bitboard representing all knights belonging to \a to_move
 *
 * @param[in] to_move Whose knights to get
 *
 * @return  A bitboard with 1 bit set for each square containing a
 *          knight belonging to \a to_move
 */
inline uint64 Position::get_knights(int to_move) const
{
	return _knights[to_move];
}

/**
 * Get the material balance. A positive value means white is better
 *
 * @return The material balance
 */
inline int Position::get_material() const
{
	return _material;
}

/**
 * Get a bitboard representing all squares occupied by \a to_move
 *
 * @param[in] to_move Get a bitboard for this guy
 *
 * @return A bitboard with 1 bit set for each square containing a
 *         piece belonging to \a to_move
 */
inline uint64 Position::get_occupied(int to_move) const
{
	return _occupied[to_move];
}

/**
 * Get a bitboard representing all squares occupied by both sides
 *
 * @return A bitboard with 1 bit set for each occupied square
 */
inline uint64 Position::get_occupied() const
{
	return _occupied[BLACK] | _occupied[WHITE];
}

/**
 * Get a bitboard representing all pawns belonging to \a to_move
 *
 * @param[in] to_move Whose pawns to get
 *
 * @return  A bitboard with 1 bit set for each square containing
 *          a pawn belonging to \a to_move
 */
inline uint64 Position::get_pawns(int to_move) const
{
	return _pawns[to_move];
}

/**
 * Get a bitboard containing all pieces that are pinned on the king
 * for the specified side
 *
 * @param[in] to_move Get pinned pieces for this side
 *
 * @return A bitboard with a bit set for each of the pinned squares
 */
inline uint64 Position::get_pinned_pieces( int to_move ) const
{
	const uint64 occupied =
		_occupied[0] | _occupied[1];

	uint64 pinned =
		attacks_from_queen(_king_sq[to_move], occupied)
			& _occupied[to_move];

	for (uint64 temp = pinned; temp; )
	{
		const int sq = Util::msb64(temp);

		switch (data_tables.directions[sq][_king_sq[to_move]])
		{
		case ALONG_RANK:
			if (!(attacks_from_rook(sq, occupied)
					& data_tables.ranks64[sq]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_FILE:
			if (!(attacks_from_rook( sq, occupied)
					& data_tables.files64[sq]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_A1H8:
			if (!(attacks_from_bishop(sq, occupied)
				  & data_tables.a1h8_64[sq]
				  & (_bishops[flip(to_move)] |
				  		 _queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
			break;
		case ALONG_H1A8:
			if (!(attacks_from_bishop(sq, occupied)
				  & data_tables.h1a8_64[sq]
				  & (_bishops[flip(to_move)] |
				  		 _queens[flip(to_move)])))
				Util::clear_bit64(sq, pinned);
		default:
			break;
		}

		Util::clear_bit64(sq, temp);
	}

	return pinned;
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
inline int Position::get_queen_mobility(int square,
	uint64 occupied) const
{
	return get_rook_mobility(square, occupied)
			+ get_bishop_mobility(square, occupied);
}

/**
 * Get a bitboard representing all queens belonging to \a to_move
 *
 * @param[in] to_move Whose queens to get
 *
 * @return A bitboard with 1 bit set for each square containing a
 *         queen belonging to \a to_move
 */
inline uint64 Position::get_queens(int to_move) const
{
	return _queens[to_move];
}

/**
 * Get a bitboard representing all queens on the board
 *
 * @return   A bitboard with 1 bit set for each square
 *           containing a queen
 */
inline uint64 Position::get_queens() const
{
	return _queens[BLACK] | _queens[WHITE];
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
inline int Position::get_rook_mobility(int square,
	uint64 occupied) const
{
	return _tables.rook_mobility[_tables.rook_offsets[square] +
			(((occupied & _tables.rook_attacks_mask[square])
				* _tables.rook_magics[square]) >> _tables.rook_db_shifts[square])];
}

/**
 * Get a bitboard representing all rooks belonging to \a to_move
 *
 * @param[in] to_move Whose rooks to get
 *
 * @return  A bitboard with 1 bit set for each square containing
 *          a rook belonging to \a to_move
 */
inline uint64 Position::get_rooks(int to_move) const
{
	return _rooks[to_move];
}

/**
 * Get a bitboard representing all rooks on the board
 *
 * @return  A bitboard with 1 bit set for each square
 *          containing a rook
 */
inline uint64 Position::get_rooks() const
{
	return _rooks[BLACK] | _rooks[WHITE];
}

/**
 * Get the player whose turn it is to move in this position
 *
 * @return \ref WHITE or \ref BLACK
 */
inline int32 Position::get_turn() const
{
#if defined(DEBUG)
	AbortIfNot( _is_init, ~0 );
#endif

	return _to_move;
}

/**
 * Determine if the given side, \a to_move, is in check
 *
 * @param[in] to_move \ref WHITE or \ref BLACK
 *
 * @return True if \a to_move is in check
 */
inline bool Position::in_check(int to_move) const
{
#if defined(DEBUG)
	AbortIf(to_move != WHITE && to_move != BLACK,
		false);
#endif

	return under_attack(_king_sq[to_move],
		flip(to_move));
}

/**
 * Determine whether a piece on a particular square would be pinned
 * on the king
 *
 * @param[in] square  The square of interest
 * @param[in] to_move The player whose piece would be pinned
 *
 * @return The direction of the pin
 */
inline direction_t Position::is_pinned(int square, int to_move) const
{
	const uint64 occupied =
		_occupied[0] | _occupied[1];

	if (attacks_from_queen(square,occupied) & _kings[to_move])
	{
		switch(data_tables.directions[ square ][_king_sq[ to_move ]])
		{
		case ALONG_RANK:
			if (attacks_from_rook( square, occupied)
					& data_tables.ranks64[square]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)]))
				return ALONG_RANK;
			break;
		case ALONG_FILE:
			if (attacks_from_rook( square, occupied)
					& data_tables.files64[square]
					& (_rooks[flip(to_move)] |
						_queens[flip(to_move)]))
				return ALONG_FILE;
			break;
		case ALONG_A1H8:
			if (attacks_from_bishop(square, occupied)
				  & data_tables.a1h8_64[square]
				  & (_bishops[flip(to_move)] |
				  			 _queens[flip(to_move)]))
				return ALONG_A1H8;
			break;
		case ALONG_H1A8:
			if (attacks_from_bishop(square, occupied)
				  & data_tables.h1a8_64[square]
				  & (_bishops[flip(to_move)] |
				  			 _queens[flip(to_move)]))
				return ALONG_H1A8;
		default:
			break;
		}
	}

	return NONE;
}

/**
 * Play the given move from the current position. Note that this does
 * NOT check for legality
 *
 * @param[in] move The move data bits
 *
 * @return True on success
 */
inline bool Position::make_move(int move)
{
#if defined(DEBUG)
	AbortIfNot(_is_init, false);
#endif

	/*
	 * Update the position hash signature
	 */
	update_hash(move);

	/*
	 * Before doing anything, carry over the castling
	 * rights to the next ply. Later, when we
	 * unMakeMove(), we'll have a record of what this
	 * was prior to making the move: 
	 */
	_castle_rights[_ply+1][BLACK] =
			_castle_rights[_ply][BLACK];
	_castle_rights[_ply+1][WHITE] = 
			_castle_rights[_ply][WHITE];

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

	const int captured = CAPTURED(move);
	const int from     = FROM(move);
	const int moved    = MOVED(move);
	const int promote  = PROMOTE(move);
	const int to       = TO(move);

	_pieces[from] = INVALID;
	_pieces[to]   = static_cast<piece_t>(moved);

	clear_set64(from, to, _occupied[_to_move] );

	/*
	 * Clear the en passant info as it is no longer
	 * valid:
	 */
	_ep_info[_ply].clear();

	int delta_material = 0;

	switch (moved)
	{
		case PAWN:
			_pawns[_to_move] &= _tables.clear_mask[from];

			if (promote != INVALID)
			{
				_pieces[to] =
					static_cast<piece_t>(promote);

				delta_material +=
						piece_value[promote ]-PAWN_VALUE;
			}

			switch (promote)
			{
				case KNIGHT:
					_knights[_to_move]
						|= _tables.set_mask[to];
					break;
				case ROOK:
					_rooks[_to_move]
						|= _tables.set_mask[to];
					break;
				case QUEEN:
					_queens[_to_move] 
						|= _tables.set_mask[to];
					break;
				case BISHOP:
					_bishops[_to_move]
						|= _tables.set_mask[to];
					break;
				default:
					_pawns[_to_move]
						|= _tables.set_mask[to];
			}

			/*
			 * Set the en passant target square:
			 */
			if (_abs(from-to) == 16)
			{
				uint64 src =
					_pawns[flip(_to_move)] & _tables.rank_adjacent[to];

				if (_to_move == WHITE)
					_ep_info[_ply].target = to-8;
				else
					_ep_info[_ply].target = to+8;

				if (src & (_tables.set_mask[to+1]))
					_ep_info[_ply].src[0] = to+1;
				if (src & (_tables.set_mask[to-1]))
					_ep_info[_ply].src[1] = to-1;
			}

			break;

		case KNIGHT:
			clear_set64(from, to, _knights[_to_move]);
			break;

		case ROOK:
			clear_set64( from, to, _rooks[_to_move] );

			if ((_castle_rights[_ply][_to_move]))
			{
				switch (FILE(from))
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

		case BISHOP:
			clear_set64(from, to, _bishops[_to_move]);
			break;

		case QUEEN:
			clear_set64(from, to, _queens[_to_move] );
			break;

		case KING:
			clear_set64( from, to, _kings[_to_move] );
			_king_sq[_to_move] = to;

			if (_abs(from-to) == 2)
			{
				/*
			 	 * This was a castle move - update
			 	 * the rook data:
			 	 */
				if (_to_move == WHITE)
				{
					if (to == G1)
					{
						_pieces[H1] = INVALID;
						_pieces[F1] = ROOK;

						clear_set64(H1, F1, _rooks[WHITE]);
						clear_set64(H1, F1,
							_occupied[WHITE]);
					}
					else // Queenside castle
					{
						_pieces[A1] = INVALID;
						_pieces[D1] = ROOK;

						clear_set64(A1, D1, _rooks[WHITE]);
						clear_set64(A1, D1,
							_occupied[WHITE]);
					}
				}
				else
				{
					if (to == G8)
					{
						_pieces[H8] = INVALID;
						_pieces[F8] = ROOK;

						clear_set64(H8, F8, _rooks[BLACK]);
						clear_set64(H8, F8,
							_occupied[BLACK]);
					}
					else // Queenside castle
					{
						_pieces[A8] = INVALID;
						_pieces[D8] = ROOK;

						clear_set64(A8, D8, _rooks[BLACK]);
						clear_set64(A8, D8,
							_occupied[BLACK]);
					}
				}
			}

			/*
			 * Clear all castling rights for this
			 * player:
			 */
			_castle_rights[_ply][_to_move] = 0;
			break;
	}

	if (captured != INVALID)
	{
		delta_material += piece_value[captured];

		switch (captured)
		{
			case PAWN:

				if (_occupied[flip(_to_move)] & _tables.set_mask[to])
				{
					_pawns[flip(_to_move)] &= _tables.clear_mask[to];
				}
				else
				{
					/*
					 * This was an en passant capture:
					 */
					if (_to_move == WHITE)
					{
						_pieces[to-8] = INVALID;

						_occupied[BLACK]
							&= _tables.clear_mask[to-8];
						_pawns[BLACK]
							&= _tables.clear_mask[to-8];
					}
					else
					{
						_pieces[to+8] = INVALID;

						_occupied[WHITE]
							&= _tables.clear_mask[to+8];
						_pawns[WHITE]
							&= _tables.clear_mask[to+8];
					}
				}

				break;

			case KNIGHT:

				_knights[flip(_to_move)] &= _tables.clear_mask[to];
				break;

			case BISHOP:
				_bishops[flip(_to_move)] &= _tables.clear_mask[to];
				break;

			case QUEEN:
				_queens[flip(_to_move)]  &= _tables.clear_mask[to];
				break;

			case ROOK:
				_rooks[ flip(_to_move) ] &= _tables.clear_mask[to];

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
					switch (FILE(to))
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
		}

		// Update the enemy occupancy:
		_occupied[flip(_to_move)] &=
				_tables.clear_mask[to];
	}

	else if (moved != PAWN)
		_half_move++;

	if ( _to_move == BLACK )
		_full_move++;

	/*
	 * Update the material balance:
	 */
	if (_to_move == WHITE)
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
inline piece_t Position::piece_on(int square) const
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
inline bool Position::under_attack(int square, int to_move) const
{
	if (_tables.pawn_attacks[flip(to_move)][square]
			& _pawns[to_move])
		return true;

	if (_tables.king_attacks[ square ] & _kings[ to_move ])
		return true;

	if (_tables.knight_attacks[square] & _knights[to_move])
		return true;

	const uint64 rooks_queens   =
		_rooks[ to_move ] | _queens[to_move];
	const uint64 bishops_queens =
		_bishops[to_move] | _queens[to_move];

	uint64 rook_attackers = attacks_from(square, ROOK, to_move);

	if ( rook_attackers & rooks_queens )
		return true;

	uint64 diag_attackers = 
		attacks_from(square, BISHOP, to_move);

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
inline bool Position::unmake_move(int move)
{
#if defined(DEBUG)
	AbortIfNot(_is_init, false);
#endif

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

	const int captured = CAPTURED(move);
	const int from     = FROM(move);
	const int moved    = MOVED(move);
	const int promote  = PROMOTE(move);
	const int to       = TO(move);

	/*
	 * Restore the piece locations
	 */
	_pieces[from] = static_cast<piece_t>(moved);
	_pieces[to]   =
				 static_cast<piece_t>(captured);

	int delta_material = 0;

	/*
	 * Restore the occupancy bits for this player
	 */
	clear_set64(to, from, _occupied[_to_move]);

	switch (moved)
	{
		case PAWN:
			_pawns[_to_move] |= _tables.set_mask[from];

			if (promote != INVALID)
			{
				delta_material +=
					piece_value[promote ] - PAWN_VALUE;
			}

			switch (promote)
			{
				case KNIGHT:
					_knights[_to_move]
						&= _tables.clear_mask[to];
					break;
				case ROOK:
					_rooks[_to_move]
						&= _tables.clear_mask[to];
					break;
				case QUEEN:
					_queens[_to_move]  
						&= _tables.clear_mask[to];
					break;
				case BISHOP:
					_bishops[_to_move] 
						&= _tables.clear_mask[to];
					break;
				default:
					_pawns[ _to_move ]
						&= _tables.clear_mask[to];
			}

			break;

		case KNIGHT:
			clear_set64(to, from, _knights[_to_move]);
			break;

		case ROOK:
			clear_set64( to, from, _rooks[_to_move] );
			break;

		case BISHOP:
			clear_set64(to, from, _bishops[_to_move]);
			break;

		case QUEEN:
			clear_set64( to, from, _queens[_to_move]);
			break;

		case KING:
			clear_set64( to, from, _kings[_to_move] );
			_king_sq[_to_move] = from;

			/*
			 * Check if this was a castle move and update
			 * the rook bits accordingly:
			 */
			if (_abs(from-to) == 2)
			{
				if (_to_move == WHITE)
				{
					if (to == G1)
					{
						_pieces[F1] = INVALID;
						_pieces[H1] = ROOK;

						clear_set64(F1, H1, _rooks[WHITE]);
						clear_set64(F1, H1,
							_occupied[WHITE]);
					}
					else // Queenside castle
					{
						_pieces[D1] = INVALID;
						_pieces[A1] = ROOK;

						clear_set64(D1, A1, _rooks[WHITE]);
						clear_set64(D1, A1,
							_occupied[WHITE]);
					}
				}
				else
				{
					if (to == G8)
					{
						_pieces[F8] = INVALID;
						_pieces[H8] = ROOK;

						clear_set64(F8, H8, _rooks[BLACK]);
						clear_set64(F8, H8,
							_occupied[BLACK]);
					}
					else // Queenside castle
					{
						_pieces[D8] = INVALID;
						_pieces[A8] = ROOK;

						clear_set64(D8, A8, _rooks[BLACK]);
						clear_set64(D8, A8,
							_occupied[BLACK]);
					}
				}
			}

			break;
	}

	/*
	 * Restore the opponent's board info if this
	 * was a capture
	 */
	if (captured != INVALID)
	{
		delta_material += piece_value[captured];

		/*
		 * Restore the enemy occupancy:
		 */
		_occupied[flip(_to_move)] |=
				_tables.set_mask[ to ];

		switch (captured)
		{
			case PAWN:

				if (to == _ep_info[_ply].target)
				{
					//  This was an en passant capture:

					_occupied[flip(_to_move)] &=
				  				_tables.clear_mask[to];

					if (_to_move == WHITE)
					{
						_pieces[to-8] = PAWN;

						_occupied[BLACK]
							|= _tables.set_mask[to-8];
						_pawns[BLACK]
							|= _tables.set_mask[to-8];
					}
					else
					{
						_pieces[to+8] = PAWN;

						_occupied[WHITE]
							|= _tables.set_mask[to+8];
						_pawns[WHITE]
							|= _tables.set_mask[to+8];
					}

					_pieces[to] = INVALID;
				}
				else
				{
					_pawns[flip(_to_move)] |=
						_tables.set_mask[to];
				}

				break;

			case KNIGHT:
				_knights[flip(_to_move)] |= _tables.set_mask[to];
				break;
			case QUEEN:
				_queens[flip(_to_move)]  |= _tables.set_mask[to];
				break;
			case ROOK:
				_rooks[ flip(_to_move) ] |= _tables.set_mask[to];
				break;
			case BISHOP:
				_bishops[flip(_to_move)] |= _tables.set_mask[to];
				break;
		}
	}
	else if (moved != PAWN)
		_half_move--;

	if (_to_move == BLACK)
		_full_move--;

	/*
	 * Restore the material balance:
	 */
	if (_to_move == WHITE)
		_material -= delta_material;
	else
		_material += delta_material;

	return true;
}

/**
 * Update the hash signature at the current ply. This should only
 * be called when making a move
 *
 * @param[in] move The last move made
 */
inline void Position::update_hash(int move)
{
	uint64& hash = _save_hash[_ply+1];
	hash = _save_hash[_ply];

	const int captured = CAPTURED(move);
	const int from     = FROM(move);
	const int moved    = MOVED(move);
	const int promote  = PROMOTE(move);
	const int to       = TO(move);

	/*
	 * If we had a valid en passant square for the current ply,
	 * remove it from the hash key:
	 */
	if (_ep_info[_ply].target != BAD_SQUARE)
	{
		hash ^=
		  _hash_input.en_passant[FILE(_ep_info[_ply].target) ];
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
	if (_abs(from-to) == 16)
	{
		hash ^= _hash_input.en_passant[ FILE(to) ];
	}

	/*
	 * Update the hash entry to reflect the new location of
	 * the piece moved:
	 */
	hash ^=
		_hash_input.piece[_to_move][moved][from];

	if (promote == INVALID)
		hash ^= _hash_input.piece[_to_move][ moved ][to];
	else
		hash ^= _hash_input.piece[_to_move][promote][to];

	/*
	 * Update the hash key if we moved a rook with which we
	 * could have castled
	 */
	if (moved == ROOK)
	{
		if (FILE(from) == 0 &&
			(_castle_rights[_ply][_to_move] & castle_K))
		{
			hash ^=
				_hash_input.castle_rights[_to_move][_OO_INDEX];
		}
		else if (FILE(from) == 7 &&
			(_castle_rights[_ply][_to_move] & castle_Q))
		{
			hash ^=
				_hash_input.castle_rights[_to_move][OOO_INDEX];
		}
	}

	/*
	 * Update the hash key to reflect new castling rights
	 * if the king was moved
	 */
	else if (moved == KING)
	{
		if (_castle_rights[_ply][_to_move] & castle_K)
			hash ^=
				_hash_input.castle_rights[_to_move][_OO_INDEX];
		if (_castle_rights[_ply][_to_move] & castle_Q)
			hash ^=
				_hash_input.castle_rights[_to_move][OOO_INDEX];

		/*
		 * Update the hash key for the rook involved with
		 * the castle
		 */
		if (_abs(from-to) == 2)
		{
			if (_to_move == WHITE)
			{
				if (to == G1)
				{
					hash ^=
						_hash_input.piece[WHITE][ROOK][H1];
					hash ^=
						_hash_input.piece[WHITE][ROOK][F1];
				}
				else
				{
					hash ^=
						_hash_input.piece[WHITE][ROOK][A1];
					hash ^=
						_hash_input.piece[WHITE][ROOK][D1];
				}
			}
			else
			{
				if (to == G8)
				{
					hash ^=
						_hash_input.piece[BLACK][ROOK][H8];
					hash ^=
						_hash_input.piece[BLACK][ROOK][F8];
				}
				else
				{
					hash ^=
						_hash_input.piece[BLACK][ROOK][A8];
					hash ^=
						_hash_input.piece[BLACK][ROOK][D8];
				}
			}
		}
	}

	/*
	 * Next, Update the hash if an opponent's piece was captured
	 */
	if (captured != INVALID)
	{
		const int x_side = flip(_to_move);

		/*
		 * Note that if the captured piece was a pawn, we need
		 * to treat en passant captures separately
		 */
		if (captured != PAWN)
			hash ^=
				_hash_input.piece[flip(_to_move)][captured][to];

		switch (captured)
		{
		case PAWN:

			if (_pieces[to] != INVALID)
				hash ^= _hash_input.piece[x_side][PAWN][to];
			else
			{
				if (_to_move == WHITE)
					hash ^=
						_hash_input.piece[BLACK][PAWN][ to-8 ];
				else
					hash ^=
						_hash_input.piece[WHITE][PAWN][ to+8 ];
			}
			
			break;

		case ROOK:

			/*
			 * Update the hash key if we captured a rook with which
			 * the opponent could have castled
			 */
			switch (FILE(to))
			{
			case 0:
				if (_castle_rights[_ply][x_side] & castle_K)
				{
					hash ^=
						_hash_input.castle_rights[x_side][_OO_INDEX];
				}
			break;

			case 7:
				if (_castle_rights[_ply][x_side] & castle_Q)
				{
					hash ^=
						_hash_input.castle_rights[x_side][OOO_INDEX];
				}
			}
		}
	}

	/*
	 * Update the hash to reflect whose turn it is
	 */
	hash ^=
		_hash_input.to_move;
}

#endif
