#include <iostream>

#include "position2.h"

/**
 * Constructor (1). Create the initial position
 *
 * @param [in] tables Databases to use
 * @param [in] xboard Flag that indicates we are interfacing with xBoard
 */
Position::Position(const DataTables& tables, bool xboard)
	: _is_init(false), _tables(tables)
{
	_is_init =
		reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			  xboard);
}

/**
 * Construct the position given in Forsyth–Edwards Notation
 *
 * @param[in] tables  Databases to use
 * @param[in] fen     A FEN position
 * @param[in] xboard  True when interfacing with xBoard
 */
Position::Position(const DataTables& tables, const std::string& fen,
				   bool xboard)
	: _is_init(false), _tables(tables)
{
	_is_init =
		reset(fen, xboard);
}

/**
 * Construct a copy of the given Position
 *
 * @param [in] other The Position to copy
 */
Position::Position(const Position& other)
	: _tables(other._tables)
{
	*this = other;
}

/**
 * Destructor
 */
Position::~Position()
{
}

/*
 * Assignment operator
 *
 * @param [in] rhs The Position to assign this to
 *
 * @return *this
 */
Position& Position::operator=(const Position& rhs)
{
	
	if (this != &rhs)
	{
		_bishops[WHITE] = rhs._bishops[WHITE];
		_bishops[BLACK] = rhs._bishops[BLACK];

		for (int i = 0; i < MAX_PLY; i++)
		{
			_castle_rights[i][WHITE] = rhs._castle_rights[i][WHITE];
			_castle_rights[i][BLACK] = rhs._castle_rights[i][BLACK];
			_ep_info[i]   = rhs._ep_info[i];
			_save_hash[i] =
				rhs._save_hash[i];
		}

		_full_move  = rhs._full_move;
		_half_move  = rhs._half_move;
		_hash_input = rhs._hash_input;
		_is_init    = rhs._is_init;

		_kings[WHITE]    = rhs._kings[ WHITE ];
		_kings[BLACK]    = rhs._kings[ BLACK ];

		_king_sq[WHITE]  = rhs._king_sq[WHITE];
		_king_sq[BLACK]  = rhs._king_sq[BLACK];

		_knights[WHITE]  = rhs._knights[WHITE];
		_knights[BLACK]  = rhs._knights[BLACK];

		_material = rhs._material;

		_occupied[WHITE] =
			rhs._occupied[WHITE];
		_occupied[BLACK] =
			rhs._occupied[BLACK];

		_pawns[WHITE]    = rhs._pawns[ WHITE ];
		_pawns[BLACK]    = rhs._pawns[ BLACK ];

		for (int i = 0; i < 64; i++)
		{
			_pieces[i] = rhs._pieces[i];
		}

		_ply = rhs._ply;

		_queens[WHITE] = rhs._queens[WHITE];
		_queens[BLACK] = rhs._queens[BLACK];

		_rooks[WHITE] = rhs._rooks[ WHITE ];
		_rooks[BLACK] = rhs._rooks[ BLACK ];

		_to_move = rhs._to_move;
	}

	return *this;
}

/**
 * Compare this Position with another in a byte-wise sense
 *
 * @param[in] rhs The Position to compare against
 *
 * @return True if they are identical, false otherwise
 */
bool Position::operator==(const Position& rhs) const
{
	bool same = true;

	for (int i = 0; i < 64; i++)
	{
		same = same && ( _pieces[i] == rhs._pieces[i] );
	}

	same = same
		&& _full_move  == rhs._full_move
		&& _half_move  == rhs._half_move
		&& _hash_input == rhs._hash_input
		&& _is_init    == rhs._is_init
		&& _material   == rhs._material
		&& _ply        == rhs._ply
		&& _to_move    == rhs._to_move;

	for (int i = 0; i < 2; i++)
	{
		same = same
			&& _bishops[i]  == rhs._bishops[i]
			&& _kings[i]    == rhs._kings[i]
		 	&& _king_sq[i]  == rhs._king_sq[i]
		 	&& _knights[i]  == rhs._knights[i]
		 	&& _occupied[i] == rhs._occupied[i]
		 	&& _pawns[i]    == rhs._pawns[i]
		 	&& _queens[i]   == rhs._queens[i]
		 	&& _rooks[i]    == rhs._rooks[i];
	}

	for (int i = 0; i < MAX_PLY; i++)
	{
		same = same
			&& _save_hash[i] == rhs._save_hash[i];

		same = same
			&& _castle_rights[i][BLACK] ==
				rhs._castle_rights[i][BLACK];

		same = same
			&& _castle_rights[i][WHITE] ==
				rhs._castle_rights[i][WHITE];

		same = same && _ep_info[i]
			== rhs._ep_info[i];
	}

	return same;
}

/**
 * Generates a new hash signature for this position. This should
 * be called for every reset()
 */
void Position::generate_hash()
{
	/*
	 * Generate pseudo-random numbers used for updating the hash
	 * keys
	 */

	std::srand(101687);

	for (int i = 0; i < 2; i++)
	{
		_hash_input.castle_rights[0][i] = Util::rand64();
		_hash_input.castle_rights[1][i] = Util::rand64();
	}

	for (int i = 0; i < 8; i++)
		_hash_input.en_passant[i] = Util::rand64();

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			_hash_input.piece[0][i][j]  = Util::rand64();
			_hash_input.piece[1][i][j]  = Util::rand64();
		}
	}

	_hash_input.to_move =
		Util::rand64();

	/*
	 * Compute the hash signature for this position
	 */

	uint64& signature  = _save_hash[_ply];
	signature = 0;

	if (_ep_info[_ply].target != BAD_SQUARE)
		signature ^=
			_hash_input.en_passant[FILE(_ep_info[_ply].target)];

	if (_to_move == WHITE)
		signature ^= _hash_input.to_move;

	if (_castle_rights[_ply][WHITE] & castle_K)
		signature ^=
			_hash_input.castle_rights[WHITE][_OO_INDEX];
	if (_castle_rights[_ply][WHITE] & castle_Q)
		signature ^=
			_hash_input.castle_rights[WHITE][OOO_INDEX];
	if (_castle_rights[_ply][BLACK] & castle_K)
		signature ^=
			_hash_input.castle_rights[BLACK][_OO_INDEX];
	if (_castle_rights[_ply][BLACK] & castle_Q)
		signature ^=
			_hash_input.castle_rights[BLACK][OOO_INDEX];

	for (int i = 0; i < 64; i++)
	{
		if (_pieces[i] != INVALID)
		{
			if (_occupied[BLACK] & _tables.set_mask[i])
			{
				signature ^=
					_hash_input.piece[ BLACK ][ _pieces[i] ][i];
			}
			else
			{
				signature ^=
					_hash_input.piece[ WHITE ][ _pieces[i] ][i];
			}
		}
	}
}

/**
 * Get the FEN representation of this position
 *
 * @return The FEN position, or an empty string if this position was
 *         not initialized
 */
std::string Position::get_fen() const
{
	int empty = 0; char empty_s[8];

	std::string fen = "";
	AbortIfNot( _is_init, fen );

	for (register int i = 63; i >= 0; i--)
	{
		std::sprintf(empty_s, "%d", empty);
		if (_pieces[i] != INVALID)
		{
			bool whitePiece =
				static_cast<bool>(_tables.set_mask[i] & _occupied[WHITE]);

			if (empty != 0)
			{
				fen += empty_s;  empty = 0;
			}

			switch (_pieces[i])
			{
			case PAWN:
				fen += (whitePiece ? "P" : "p");
				break;
			case KNIGHT:
				fen += (whitePiece ? "N" : "n");
				break;
			case BISHOP:
				fen += (whitePiece ? "B" : "b");
				break;
			case ROOK:
				fen += (whitePiece ? "R" : "r");
				break;
			case QUEEN:
				fen += (whitePiece ? "Q" : "q");
				break;
			default:
				fen += (whitePiece ? "K" : "k");
			}
		}
		else
			empty++;

		// Start the next rank:
		if (i % 8 == 0)
		{
			if (empty != 0)
			{
				std::sprintf(empty_s, "%d", empty);
				empty = 0;
				fen += empty_s; 
			}
			if (i != 0)
				fen += "/";
		}
	}

	if (_to_move == WHITE)
		fen += " w ";
	else
		fen += " b ";

	if (_castle_rights[_ply][WHITE] & castle_K)
		fen += "K";
	if (_castle_rights[_ply][WHITE] & castle_Q)
		fen += "Q";
	if (_castle_rights[_ply][BLACK] & castle_K)
		fen += "k";
	if (_castle_rights[_ply][BLACK] & castle_Q)
		fen += "q";

	if (_castle_rights[_ply][WHITE] == 0 &&
		_castle_rights[_ply][BLACK] == 0)
		fen += "-";

	fen += " ";

	if (_ep_info[_ply].target != BAD_SQUARE)
		fen += SQUARE_STR[_ep_info[_ply].target];
	else
		fen += "-";

	char halfMove_s[8];
	char fullMove_s[8];

	std::sprintf(halfMove_s, "%d", _half_move);
	std::sprintf(fullMove_s, "%d", _full_move);

	std::string space = " ";

	fen +=  space + std::string(halfMove_s) +
		    space + fullMove_s;

	return fen;
}

/**
 * Get the full move number, i.e. from the FEN representation
 * of this position
 *
 * @return The move number
 */
int Position::get_fullmove_number() const
{
	return _full_move;
}

/**
 * Display the current position
 */
void Position::print() const
{
	int prev_rank = 8;
	uint64 one = 1;

	for (int sq = 63; sq >= -1; sq--)
	{
		if (RANK(sq) != prev_rank)
		{
			std::cout << "\n ---+---+---+---+---+---+---+--- \n";
			if (sq == -1) break;

			prev_rank = RANK(sq);
		}

		if (_pieces[sq] != INVALID)
		{
			char piece;
			switch (_pieces[sq])
			{
				case PAWN:
					piece = 'P'; break;
				case ROOK:
					piece = 'R'; break;
				case KNIGHT:
					piece = 'N'; break;
				case BISHOP:
					piece = 'B'; break;
				case QUEEN:
					piece = 'Q'; break;
				case KING:
					piece = 'K'; break;
			}

			if (_occupied[BLACK] & (one << sq))
				piece = Util::to_lower(piece);

			std::cout << "| " << piece << " ";
		}
		else
			std::cout << "|   ";

		if (sq % 8 == 0) std::cout << "|";
	}

	std::cout << std::endl;
}

/**
 * Reset to the position encoded in Forsyth–Edwards Notation (FEN)
 *
 * @param [in] fen    A FEN position
 * @param [in] xboard Indicates we're interfacing with xBoard
 *
 * @return True if the new FEN position was successfully processed
 */
bool Position::reset(const std::string& fen, bool xboard)
{
	Position backup(*this);
	int square = 63;

	// Clear member fields:
	set_default();

	Util::str_v tokens; Util::split(fen, tokens, '/');

	if (tokens.size() != 8)
	{
		if (!xboard)
			std::cout << "Invalid FEN (wrong number of ranks): "
				<< fen << std::endl;
		*this = backup;
		return false;
	}

	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < tokens[i].size(); j++)
		{
			char c = tokens[i][j];
			if (Util::isPiece(c))
			{
				uint64 mask = Util::getBit<uint64>(square);
				_pieces[square] =
					Util::piece2enum(c);

				if (Util::to_lower(c) == c)
					_occupied[BLACK] |= mask;
				else
					_occupied[WHITE] |= mask;
				square -= 1;

				switch (c)
				{
					case 'p':
						_pawns[BLACK]   |= mask; break;
					case 'P':
						_pawns[WHITE]   |= mask; break;
					case 'r':
						_rooks[BLACK]   |= mask; break;
					case 'R':
						_rooks[WHITE]   |= mask; break;
					case 'n':
						_knights[BLACK] |= mask; break;
					case 'N':
						_knights[WHITE] |= mask; break;
					case 'b':
						_bishops[BLACK] |= mask; break;
					case 'B':
						_bishops[WHITE] |= mask; break;
					case 'q':
						_queens[BLACK]  |= mask; break;
					case 'Q':
						_queens[WHITE]  |= mask; break;
					case 'k':
						_kings [BLACK]  |= mask;
						_king_sq[BLACK]  =
							Util::getLSB<uint64>(mask);
						break;
					case 'K':
						_kings [WHITE]  |= mask;
						_king_sq[WHITE]  =
							Util::getLSB<uint64>(mask);
						break;
					default:
						*this = backup;
						return false;
				}
			}
			else
			{
				if (std::isdigit(c))
				{
					int32 val;
					if (!Util::str_to_i32(std::string(&c,1),10,val))
					{
						*this = backup;
						return false;
					}
					else
						square -= val;
				}
				else
				{
					if (!xboard)
						std::cout
							<< "Invalid FEN (unexpected character \""
							<< c << "\"):"
								<< fen << std::endl;
					*this = backup;
					return false;
				}
			}

			if ((square < 0 && i != 7) || square < -1)
			{
				if (!xboard)
					std::cout
						<< "Invalid FEN (more than 64 squares given): "
						<< fen << std::endl;
				*this = backup;
				return false;
			}
			else if (square < 0)
			{
				// Move on to whose turn:
				break;
			}
		}
	}

	Util::str_v posn_info;
	Util::split(tokens.back(), posn_info, ' ');

	_half_move = 0;
	_full_move = 1;

	/*
	 * Reset the ply count, which removes all database-driven history
	 */
	_ply = 0;

	switch (posn_info.size())
	{
		default:
			// Ignore anything beyond the 6th token instead of
			// returning an error
		case 6:
			if (!Util::str_to_i32(posn_info[5],10,_full_move))
			{
				if (!xboard)
					std::cout << "Invalid FEN (fullmove number): "
						<< fen << std::endl;
				*this = backup;
				return false;
			}
		case 5:
			if (!Util::str_to_i32(posn_info[4],10,_half_move))
			{
				if (!xboard)
					std::cout << "Invalid FEN (halfmove clock): "
						<< fen << std::endl;
				*this = backup;
				return false;
			}
		case 4:
			_ep_info[_ply].target = BAD_SQUARE;

			if (posn_info[3] != "-")
			{
				for (int i = 0; i < 64; i++)
				{
					if (Util::to_lower(posn_info[3]) == SQUARE_STR[i])
					{
						_ep_info[_ply].target = i; break;
					}
				}

				if (_ep_info[_ply].target == BAD_SQUARE)
				{
					if (!xboard)
						std::cout << "Invalid FEN (en passant square): "
							<< fen << std::endl;
					*this = backup;
					return false;
				}
			}
		case 3:
			for (size_t i = 0; i < posn_info[2].size(); i++)
			{
				switch (posn_info[2][i])
				{
					case 'K':
						_castle_rights[_ply][WHITE] |= castle_K;
						break;
					case 'Q':
						_castle_rights[_ply][WHITE] |= castle_Q;
						break;
					case 'k':
						_castle_rights[_ply][BLACK] |= castle_K;
						break;
					case 'q':
						_castle_rights[_ply][BLACK] |= castle_Q;
						break;
					case '-':
						if (posn_info[2].size() == 1)
							continue;
					default:
						if (!xboard)
							std::cout << "Invalid FEN (castling rights): "
								<< fen << std::endl;
						*this = backup;
						return (false);
				}
			}
		case 2:
			if (posn_info[1] != "w" && posn_info[1] != "b")
			{
				if (!xboard)
					std::cout << "Invalid FEN (invalid color): "
						<< fen << std::endl;
				*this = backup;
				return false;
			}
			_to_move = posn_info[1] == "w" ? WHITE : BLACK;
			break;
		case 1:
			if (!xboard)
				std::cout << "Invalid FEN (unspecified color): "
					<< fen << std::endl;
			*this = backup;
				return false;
	}

	/*
	 * Set the squares from which we can capture via en passant:
	 */
	uint64 src = 0; int victim;
	
	if (_ep_info[_ply].target != BAD_SQUARE)
	{
		victim = (_to_move == WHITE) ? 
			_ep_info[_ply].target-8 : _ep_info[_ply].target+8;

		src =
			_pawns[ _to_move] & _tables.rank_adjacent[victim];

		if (src & (_tables.set_mask[victim+1]))
			_ep_info[_ply].src[0] = victim+1;

		if (src & (_tables.set_mask[victim-1]))
			_ep_info[_ply].src[1] = victim-1;
	}

	/*
	 * Validate the new position. If it violates any of the rules
	 * of chess, reject it
	 */
	if (!validate(fen, xboard))
	{
		*this = backup;
			return(false);
	}

	/*
	 * Compute the material balance. This avoids
	 * having to do so during static eval
	 */
	_material =
		Util::bitCount(_pawns[WHITE])   * PAWN_VALUE   +
		Util::bitCount(_knights[WHITE]) * KNIGHT_VALUE + 
		Util::bitCount(_bishops[WHITE]) * BISHOP_VALUE + 
		Util::bitCount(_rooks[WHITE])   * ROOK_VALUE   +
		Util::bitCount(_queens[WHITE])  * QUEEN_VALUE;

	_material -=
		Util::bitCount(_pawns[BLACK])   * PAWN_VALUE   + 
		Util::bitCount(_knights[BLACK]) * KNIGHT_VALUE + 
		Util::bitCount(_bishops[BLACK]) * BISHOP_VALUE + 
		Util::bitCount(_rooks[BLACK])   * ROOK_VALUE   +
		Util::bitCount(_queens[BLACK])  * QUEEN_VALUE;

	/*
	 * Generate a hash signature for this position
	 */
	generate_hash();

	_is_init = true;
		return _is_init;
}

/**
 * Reset to the initial (starting) position
 *
 * @param [in] xboard If true, then suppress error messages since we
 *                    are interfacing with xBoard
 *
 * @return True on success
 */
bool Position::reset(bool xboard)
{
	return
	reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		xboard);
}

/**
 * Set default (uninitialized) values for this position
 */
void Position::set_default()
{
	for (int i = 0; i < 64; i++ ) _pieces[i] = INVALID;

	for (int i = 0; i < 2; i++)
	{
		_bishops[i]  = 0;
		_kings[i]    = 0;
		_king_sq[i]  = BAD_SQUARE;
		_knights[i]  = 0;
		_occupied[i] = 0;
		_pawns[i]    = 0;
		_queens[i]   = 0;
		_rooks[i]    = 0;
	}


	_full_move = -1;
	_half_move = -1;

	_hash_input.clear();

	_is_init   = false;
	_material  = 0;
	_ply       = 0;
	_to_move   = 0;

	for (int i = 0; i < MAX_PLY; i++)
	{
		_ep_info[i].clear();

		_castle_rights[i][BLACK] = 0;
		_castle_rights[i][WHITE] = 0;

		_save_hash[i] = 0;
	}
}

/**
 * Validate the position
 *
 * @param[in] fen     The FEN representation of the current
 *                    position
 * @param[in] xboard  Is true we're interfacing with xBoard
 *
 * @return True if this is a valid Position
 */
bool Position::validate(const std::string& fen, bool xboard) const
{
	// Validate against the following rules:
	//
	// 1. No pawns on the 1st or 8th ranks
	// 2. Only two kings on board
	// 3. Side to move cannot capture a king
	// 4. Castling rights make sense (e.g. king is not on its home
	//    square => cannot castle)
	// 5. En passant target makes sense (e.g. there must be a pawn
	//    that has advanced two squares)
	// 6. Maximum 8 pawns per side
	// 7. At most 10 of any piece, per side

	// Rule 1:
	if ((_pawns[BLACK] | _pawns[WHITE]) & (RANK_1 | RANK_8))
	{
		if (!xboard)
			std::cout << "Invalid FEN (pawn(s) on back rank): "
				<< fen << std::endl;
		return false;
	}

	// Rule 2:
	if ((Util::bitCount(_kings[WHITE]) != 1) ||
		(Util::bitCount(_kings[BLACK]) != 1))
	{
		if (!xboard)
			std::cout<< "Invalid FEN (wrong number of kings): "
				<< fen << std::endl;
		return false;
	}

	// Rule 3:
	if (in_check(flip(_to_move)))
	{
		if (!xboard)
			std::cout << "Invalid FEN (king can be captured): "
				<< fen << std::endl;
		return false;
	}

	// Rule 4:
	int32 castle_mask = castle_K | castle_Q;

	if (!(_kings[WHITE] & Util::getBit< uint64 >(E1)))
	{
		if (_castle_rights[_ply][WHITE] & castle_mask)
		{
			if (!xboard)
				std::cout <<"Invalid FEN (white may not castle): "
					<< fen << std::endl;
			return false;
		}
	}
	else
	{
		if ((_castle_rights[_ply][WHITE] & castle_K)
			 && !(_rooks[WHITE] & Util::getBit<uint64>(H1)))
		{
			if (!xboard)
				std::cout << "Invalid FEN (white may not castle "
					"kingside): "
					<< fen << std::endl;
			return false; 
		}

		if ((_castle_rights[_ply][WHITE] & castle_Q)
			 && !(_rooks[WHITE] & Util::getBit<uint64>(A1)))
		{
			if (!xboard)
				std::cout << "Invalid FEN (white may not castle "
					"queenside): "
					<< fen << std::endl;
			return false; 
		}
	}

	if (!(_kings[BLACK] & Util::getBit< uint64 >(E8)))
	{
		if (_castle_rights[_ply][BLACK] & castle_mask)
		{
			if (!xboard)
				std::cout <<"Invalid FEN (black may not castle): "
					<< fen << std::endl;
			return false;
		}
	}
	else
	{
		if ((_castle_rights[_ply][BLACK] & castle_K)
			 && !(_rooks[BLACK] & Util::getBit<uint64>(H8)))
		{
			if (!xboard)
				std::cout << "Invalid FEN (black may not castle "
					"kingside): "
					<< fen << std::endl;
			return false; 
		}

		if ((_castle_rights[_ply][BLACK] & castle_Q)
			 && !(_rooks[BLACK] & Util::getBit<uint64>(A8)))
		{
			if (!xboard)
				std::cout << "Invalid FEN (black may not castle "
					"queenside): "
					<< fen << std::endl;
			return false; 
		}
	}

	// Rule 5:
	if (_ep_info[_ply].target != BAD_SQUARE)
	{
		bool bad_ep = false;

		if (_to_move == WHITE)
		{
			if (RANK(_ep_info[_ply].target) != 5 ||
				!(_pawns[BLACK] &
					Util::getBit<uint64>(_ep_info[_ply].target-8)))
			{
				bad_ep = true;
			}
		}
		else
		{
			if (RANK(_ep_info[_ply].target) != 2 ||
				!(_pawns[WHITE] &
					Util::getBit<uint64>(_ep_info[_ply].target+8)))
			{
				bad_ep = true;
			}
		}

		if (bad_ep)
		{
			if (!xboard)
				std::cout << "Invalid FEN (En passant square): "
					<< fen << std::endl;
			return false;
		}
	}

	// Rule 6:
	if (Util::bitCount<uint64>(_pawns[WHITE]) > 8 || 
		Util::bitCount<uint64>(_pawns[BLACK]) > 8)
	{
		if (!xboard)
			std::cout << "Invalid FEN (Max 8 pawns allowed per side): "
				<< fen << std::endl;
		return false;
	}

	// Rule 7:
	if (Util::bitCount<uint64>(_knights[WHITE]) > 10 ||
		Util::bitCount<uint64>(_knights[BLACK]) > 10)
	{
		if (!xboard)
			std::cout << "Invalid FEN (Max 10 knights allowed per side): "
				<< fen << std::endl;
		return false;
	}

	if (Util::bitCount<uint64>(_rooks[WHITE]) > 10 ||
		Util::bitCount<uint64>(_rooks[BLACK]) > 10)
	{
		if (!xboard)
			std::cout << "Invalid FEN (Max 10 rooks allowed per side): "
				<< fen << std::endl;
		return false;
	}

	if (Util::bitCount<uint64>(_queens[WHITE]) > 10 ||
		Util::bitCount<uint64>(_queens[BLACK]) > 10)
	{
		if (!xboard)
			std::cout << "Invalid FEN (Max 10 queens allowed per side): "
				<< fen << std::endl;
		return false;
	}

	if (Util::bitCount<uint64>(_bishops[WHITE]) > 10 ||
		Util::bitCount<uint64>(_bishops[BLACK]) > 10)
	{
		if (!xboard)
			std::cout << "Invalid FEN (Max 10 bishops allowed per side): "
				<< fen << std::endl;
		return false;
	}
	
	return true;
}
