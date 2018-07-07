#include "Position4.h"
#include "util/bit_tools.h"
#include "util/str_util.h"
#include "Verbosity.h"

namespace Chess
{
	/**
	 * Constructor
	 *
	 * @param[in] stream The stream to write log messages to
	 * @param[in] fen    An initial FEN position
	 */
	Position::Position(Handle<std::ostream> stream,
					   const std::string& fen)
		: _output(stream, "Position")
	{
		_is_init = reset(fen);
	}

	/**
	 * Copy constructor
	 *
	 * @param[in] other The Position to copy
	 */
	Position::Position(const Position& other)
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
	 * @param [in] rhs The Position to assign *this to
	 *
	 * @return *this
	 */
	Position& Position::operator=(const Position& rhs)
	{
		if (this != &rhs)
		{
			_bishops[white] = rhs._bishops[white];
			_bishops[black] = rhs._bishops[black];

			for (int i = 0; i < max_ply; i++)
			{
				_castle_rights[i][white] = rhs._castle_rights[i][white];
				_castle_rights[i][black] = rhs._castle_rights[i][black];

				_ep_info[i]   = rhs._ep_info[i];
				_save_hash[i] =
					rhs._save_hash[i];
			}

			_full_move  = rhs._full_move;
			_half_move  = rhs._half_move;
			_hash_input = rhs._hash_input;
			_is_init    = rhs._is_init;

			_king_sq[white]  = rhs._king_sq[white];
			_king_sq[black]  = rhs._king_sq[black];

			_kings[white]    = rhs._kings[ white ];
			_kings[black]    = rhs._kings[ black ];

			_knights[white]  = rhs._knights[white];
			_knights[black]  = rhs._knights[black];

			_material = rhs._material;

			_occupied[white] =
				rhs._occupied[white];
			_occupied[black] =
				rhs._occupied[black];

			_output = rhs._output;

			_pawns[white]    = rhs._pawns[ white ];
			_pawns[black]    = rhs._pawns[ black ];

			for (int i = 0; i < 64; i++)
			{
				_pieces[i] = rhs._pieces[i];
			}

			_ply = rhs._ply;

			_queens[white] = rhs._queens[white];
			_queens[black] = rhs._queens[black];

			_rooks[white] = rhs._rooks[ white ];
			_rooks[black] = rhs._rooks[ black ];

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
			&& _to_move    == rhs._to_move
			&& _ply        == rhs._ply;

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

		for (int i = 0; i < max_ply; i++)
		{
			same = same
				&& _save_hash[i] == rhs._save_hash[i];

			same = same
				&& _castle_rights[i][black] ==
					rhs._castle_rights[i][black];

			same = same
				&& _castle_rights[i][white] ==
					rhs._castle_rights[i][white];

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
			_hash_input.castle_rights[black][i] = Util::rand64();
			_hash_input.castle_rights[white][i] = Util::rand64();
		}

		for (int i = 0; i < 8; i++)
			_hash_input.en_passant[i] = Util::rand64();

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				_hash_input.piece[black][i][j]  = Util::rand64();
				_hash_input.piece[white][i][j]  = Util::rand64();
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
				_hash_input.en_passant[get_file(_ep_info[_ply].target)];

		if (_to_move == white)
			signature ^= _hash_input.to_move;

		if (_castle_rights[_ply][white] & castle_K)
			signature ^=
				_hash_input.castle_rights[white][castle_K_index];
		if (_castle_rights[_ply][white] & castle_Q)
			signature ^=
				_hash_input.castle_rights[white][castle_Q_index];
		if (_castle_rights[_ply][black] & castle_K)
			signature ^=
				_hash_input.castle_rights[black][castle_K_index];
		if (_castle_rights[_ply][black] & castle_Q)
			signature ^=
				_hash_input.castle_rights[black][castle_Q_index];

		auto& tables = DataTables::get();

		for (int i = 0; i < 64; i++)
		{
			if (_pieces[i] != piece_t::empty)
			{
				if (_occupied[black] & tables.set_mask[i])
				{
					signature ^=
						_hash_input.piece[ black ][ _pieces[i] ][i];
				}
				else
				{
					signature ^=
						_hash_input.piece[ white ][ _pieces[i] ][i];
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

		auto& tables = DataTables::get();

		for (register int i = 63; i >= 0; i--)
		{
			std::sprintf(empty_s, "%d", empty);
			if (_pieces[i] != piece_t::empty)
			{
				bool whitePiece =
					static_cast<bool>(tables.set_mask[i] & _occupied[white]);

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

		if (_to_move == white)
			fen += " w ";
		else
			fen += " b ";

		if (_castle_rights[_ply][white] & castle_K)
			fen += "K";
		if (_castle_rights[_ply][white] & castle_Q)
			fen += "Q";
		if (_castle_rights[_ply][black] & castle_K)
			fen += "k";
		if (_castle_rights[_ply][black] & castle_Q)
			fen += "q";

		if (_castle_rights[_ply][white] == 0 &&
			_castle_rights[_ply][black] == 0)
			fen += "-";

		fen += " ";

		if (_ep_info[_ply].target != BAD_SQUARE)
			fen += square_str[_ep_info[_ply].target];
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
			if (get_rank(sq) != prev_rank)
			{
				std::printf("\n ---+---+---+---+---+---+---+--- \n");
				if (sq == -1) break;

				prev_rank = get_rank(sq);
			}

			if (_pieces[sq] != piece_t::empty)
			{
				char piece;
				switch (_pieces[sq])
				{
					case piece_t::pawn:
						piece = 'P'; break;
					case piece_t::rook:
						piece = 'R'; break;
					case piece_t::knight:
						piece = 'N'; break;
					case piece_t::bishop:
						piece = 'B'; break;
					case piece_t::queen:
						piece = 'Q'; break;
					case piece_t::king:
						piece = 'K'; break;
					default:
						break;
				}

				if (_occupied[black] & (one << sq))
					piece = Util::to_lower(piece);

				std::printf("| %c ", piece);
			}
			else
				std::printf("|   ");

			if (sq % 8 == 0)
				std::printf("|");
		}

		std::fflush(stdout);
	}

	/**
	 * Reset to the position encoded in Forsyth–Edwards Notation (FEN)
	 *
	 * @param [in] fen    A FEN position
	 *
	 * @return True if the new FEN position was successfully processed
	 */
	bool Position::reset(const std::string& fen)
	{
		Position backup(*this);
		int square = 63;

		// Clear member fields:
		set_default();

		std::vector<std::string> tokens; Util::split(fen, tokens, '/');

		if (tokens.size() != 8)
		{
			if (verbosity >= Verbosity::terse)
			{
				_output.write("Invalid FEN (wrong number of ranks): '%s'\n",
					fen.c_str());
			}

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
					uint64 mask = Util::get_bit<uint64>(square);
					_pieces[square] = piece2enum(c);

					if (Util::to_lower(c) == c)
						_occupied[black] |= mask;
					else
						_occupied[white] |= mask;
					square -= 1;

					switch (c)
					{
						case 'p':
							_pawns[black]   |= mask; break;
						case 'P':
							_pawns[white]   |= mask; break;
						case 'r':
							_rooks[black]   |= mask; break;
						case 'R':
							_rooks[white]   |= mask; break;
						case 'n':
							_knights[black] |= mask; break;
						case 'N':
							_knights[white] |= mask; break;
						case 'b':
							_bishops[black] |= mask; break;
						case 'B':
							_bishops[white] |= mask; break;
						case 'q':
							_queens[black]  |= mask; break;
						case 'Q':
							_queens[white]  |= mask; break;
						case 'k':
							_kings [black]  |= mask;
							_king_sq[black]  =
								Util::get_lSB<uint64>(mask);
							break;
						case 'K':
							_kings [white]  |= mask;
							_king_sq[white]  =
								Util::get_lSB<uint64>(mask);
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
						int val;
						if (!Util::from_string(std::string(&c,1),val))
						{
							*this = backup;
							return false;
						}
						else
							square -= val;
					}
					else
					{
						if (verbosity >= Verbosity::terse)
						{
							_output.write("Invalid FEN (unexpected character '%c'): %s\n",
								c, fen.c_str());
						}
						
						*this = backup;
						return false;
					}
				}

				if ((square < 0 && i != 7) || square < -1)
				{
					if (verbosity >= Verbosity::terse)
					{
						_output.write("Invalid FEN (more than 64 squares given): '%s'\n",
							fen.c_str());
					}
					
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

		std::vector<std::string> posn_info;
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
				if (!Util::from_string(posn_info[5], _full_move))
				{
					if (verbosity >= Verbosity::terse)
					{
						_output.write("Invalid FEN (fullmove number): '%s'\n",
							fen.c_str());
					}

					*this = backup;
					return false;
				}
			case 5:
				if (!Util::from_string(posn_info[4], _half_move))
				{
					if (verbosity >= Verbosity::terse)
					{
						_output.write("Invalid FEN (halfmove clock): '%s'\n",
							fen.c_str());
					}

					*this = backup;
					return false;
				}
			case 4:
				_ep_info[_ply].target = BAD_SQUARE;

				if (posn_info[3] != "-")
				{
					for (int i = 0; i < 64; i++)
					{
						if (Util::to_lower(posn_info[3]) == square_str[i])
						{
							_ep_info[_ply].target = i; break;
						}
					}

					if (_ep_info[_ply].target == BAD_SQUARE)
					{
						if (verbosity >= Verbosity::terse)
						{
							_output.write("Invalid FEN (en passant square): '%s'\n",
								fen.c_str());
						}
						
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
							_castle_rights[_ply][white] |= castle_K;
							break;
						case 'Q':
							_castle_rights[_ply][white] |= castle_Q;
							break;
						case 'k':
							_castle_rights[_ply][black] |= castle_K;
							break;
						case 'q':
							_castle_rights[_ply][black] |= castle_Q;
							break;
						case '-':
							if (posn_info[2].size() == 1)
								continue;
						default:
							if (verbosity >= Verbosity::terse)
							{
								_output.write("Invalid FEN (castling rights): '%s'\n",
									fen.c_str());
							}
							
							*this = backup;
							return (false);
					}
				}
			case 2:
				if (posn_info[1] != "w" && posn_info[1] != "b")
				{
					if (verbosity >= Verbosity::terse)
					{
						_output.write("Invalid FEN (invalid color): '%s'\n",
							fen.c_str());
					}
					
					*this = backup;
					return false;
				}
				_to_move = posn_info[1] == "w" ? white : black;
				break;
			case 1:
				if (verbosity >= Verbosity::terse)
				{
					_output.write("Invalid FEN (unspecified color): '%s'\n",
						fen.c_str());
				}
				
				*this = backup;
					return false;
		}

		/*
		 * Set the squares from which we can capture via en passant:
		 */
		uint64 src = 0; int victim;

		auto& tables = DataTables::get();
		
		if (_ep_info[_ply].target != BAD_SQUARE)
		{
			victim = (_to_move == white) ? 
				_ep_info[_ply].target-8 : _ep_info[_ply].target+8;

			src =
				_pawns[ _to_move ] & tables.rank_adjacent[victim];

			if (src & (tables.set_mask[victim+1]))
				_ep_info[_ply].src[0] = victim+1;

			if (src & (tables.set_mask[victim-1]))
				_ep_info[_ply].src[1] = victim-1;
		}

		/*
		 * Validate the new position. If it violates any of the rules
		 * of chess, reject it
		 */
		if (!validate(fen))
		{
			*this = backup;
				return(false);
		}

		/*
		 * Compute the material balance. This avoids
		 * having to do so during static eval
		 */
		_material =
			Util::bitCount(_pawns[white])   * pawn_value   +
			Util::bitCount(_knights[white]) * knight_value + 
			Util::bitCount(_bishops[white]) * bishop_value + 
			Util::bitCount(_rooks[white])   * rook_value   +
			Util::bitCount(_queens[white])  * queen_value;

		_material -=
			Util::bitCount(_pawns[black])   * pawn_value   + 
			Util::bitCount(_knights[black]) * knight_value + 
			Util::bitCount(_bishops[black]) * bishop_value + 
			Util::bitCount(_rooks[black])   * rook_value   +
			Util::bitCount(_queens[black])  * queen_value;

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
	 * @return True on success
	 */
	bool Position::reset()
	{
		return reset(init_fen);
	}

	/**
	 * Set default (uninitialized) values for this position
	 */
	void Position::set_default()
	{
		for (int i = 0; i < 64; i++ ) _pieces[i] = piece_t::empty;

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

		for (int i = 0; i < max_ply; i++)
		{
			_ep_info[i].clear();

			_castle_rights[i][black] = 0;
			_castle_rights[i][white] = 0;

			_save_hash[i] = 0;
		}
	}

	/**
	 * Validate the position according to the following rules:
	 *
	 * 1. No pawns on the 1st or 8th ranks
	 * 2. Only two kings on board
	 * 3. Side to move cannot capture a king
	 * 4. Castling rights make sense (e.g. king is not on its home
	 *    square => cannot castle)
	 * 5. En passant target makes sense (e.g. there must be a pawn
	 *    that has advanced two squares)
	 * 6. Maximum 8 pawns per side
	 * 7. At most 10 of any piece, per side
	 *
	 * @param[in] fen The FEN representation of the position to
	 *                validate
	 *
	 * @return True if this is a valid Position
	 */
	bool Position::validate(const std::string& fen) const
	{
		// Rule 1:
		if ((_pawns[black] | _pawns[white]) & (rank_1 | rank_8))
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (pawn(s) on back rank): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 2:
		if ((Util::bit_count(_kings[white]) != 1) ||
			(Util::bit_count(_kings[black]) != 1))
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (wrong number of kings): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 3:
		if (in_check(flip(_to_move)))
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (king can be captured): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 4:
		int castle_mask = castle_K | castle_Q;

		if (!(_kings[white] & Util::get_bit< uint64 >(E1)))
		{
			if (_castle_rights[_ply][white] & castle_mask)
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (white may not castle): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}
		else
		{
			if ((_castle_rights[_ply][white] & castle_K)
				 && !(_rooks[white] & Util::get_bit<uint64>(H1)))
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (white may not castle short): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}

			if ((_castle_rights[_ply][white] & castle_Q)
				 && !(_rooks[white] & Util::get_bit<uint64>(A1)))
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (white may not castle long): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}
		}

		if (!(_kings[black] & Util::get_bit< uint64 >(E8)))
		{
			if (_castle_rights[_ply][black] & castle_mask)
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (black may not castle): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}
		else
		{
			if ((_castle_rights[_ply][black] & castle_K)
				 && !(_rooks[black] & Util::get_bit<uint64>(H8)))
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (black may not castle short): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}

			if ((_castle_rights[_ply][black] & castle_Q)
				 && !(_rooks[black] & Util::get_bit<uint64>(A8)))
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (black may not castle long): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}
		}

		// Rule 5:
		if (_ep_info[_ply].target != BAD_SQUARE)
		{
			bool bad_ep = false;

			if (_to_move == white)
			{
				if (get_rank(_ep_info[_ply].target) != 5 ||
					!(_pawns[black] &
						Util::get_bit<uint64>(_ep_info[_ply].target-8)))
				{
					bad_ep = true;
				}
			}
			else
			{
				if (get_rank(_ep_info[_ply].target) != 2 ||
					!(_pawns[white] &
						Util::get_bit<uint64>(_ep_info[_ply].target+8)))
				{
					bad_ep = true;
				}
			}

			if (bad_ep)
			{
				if (verbosity >= terse)
				{
					_output.write("Invalid FEN (En passant square): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}

		// Rule 6:
		if (Util::bit_count<uint64>(_pawns[white]) > 8 || 
			Util::bit_count<uint64>(_pawns[black]) > 8)
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (Max 8 pawns allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 7:
		if (Util::bit_count<uint64>(_knights[white]) > 10 ||
			Util::bit_count<uint64>(_knights[black]) > 10)
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (Max 10 knights allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_rooks[white]) > 10 ||
			Util::bit_count<uint64>(_rooks[black]) > 10)
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (Max 10 rooks allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_queens[white]) > 10 ||
			Util::bit_count<uint64>(_queens[black]) > 10)
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (Max 10 queens allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_bishops[white]) > 10 ||
			Util::bit_count<uint64>(_bishops[black]) > 10)
		{
			if (verbosity >= terse)
			{
				_output.write("Invalid FEN (Max 10 bishops allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}
		
		return true;
	}
}
