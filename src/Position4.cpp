#include "util/bit_tools.h"
#include "Position4.h"
#include "util/str_util.h"
#include "Verbosity.h"

namespace Chess
{
	constexpr char Position::init_fen[];

	/**
	 * Dump the set of random numbers to standard output
	 */
	void Position::HashInput::print()
	{
		for (int i = 0; i < 2; i++)
		{
			std::printf("castle_rights[white][%d]  = 0x%" PRIx64 "\n",
				i, castle_rights[player_t::white][i]);
			std::printf("castle_rights[black][%d]  = 0x%" PRIx64 "\n",
				i, castle_rights[player_t::black][i]);
		}

		for (int i = 0; i < 8; i++)
		{
			std::printf("en_passant[%d]            = 0x%" PRIx64 "\n",
				i, en_passant[i]);
		}

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				std::printf("piece[black][%d][%2d]      = 0x%" PRIx64 "\n",
					i, j, piece[player_t::black][i][j]);
				std::printf("piece[white][%d][%2d]      = 0x%" PRIx64 "\n",
					i, j, piece[player_t::white][i][j]);
			}
		}

		std::printf("to_move                  = 0x%" PRIx64 "\n",
			to_move);

		std::fflush(stdout);
	}

	/**
	 * Constructor
	 *
	 * @param[in] stream The stream to write log messages to
	 * @param[in] fen    An initial FEN position
	 */
	Position::Position(Handle<std::ostream> stream,
					   const std::string& fen)
		: _output(new OutputSource("Position", stream))
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
			_bishops[player_t::white] = rhs._bishops[player_t::white];
			_bishops[player_t::black] = rhs._bishops[player_t::black];

			for (int i = 0; i < max_ply; i++)
			{
				_castle_rights[i][player_t::white] =
					rhs._castle_rights[i][player_t::white];
				_castle_rights[i][player_t::black] =
					rhs._castle_rights[i][player_t::black];

				_ep_info[i]   = rhs._ep_info[i];
				_save_hash[i] =
					rhs._save_hash[i];
			}

			_full_move  = rhs._full_move;
			_half_move  = rhs._half_move;
			_hash_input = rhs._hash_input;
			_is_init    = rhs._is_init;

			_king_sq[player_t::white]  = rhs._king_sq[player_t::white];
			_king_sq[player_t::black]  = rhs._king_sq[player_t::black];

			_kings[player_t::white]    = rhs._kings[ player_t::white ];
			_kings[player_t::black]    = rhs._kings[ player_t::black ];

			_knights[player_t::white]  = rhs._knights[player_t::white];
			_knights[player_t::black]  = rhs._knights[player_t::black];

			_material = rhs._material;

			_occupied[player_t::white] =
				rhs._occupied[player_t::white];
			_occupied[player_t::black] =
				rhs._occupied[player_t::black];

			_output = rhs._output;

			_pawns[player_t::white]    = rhs._pawns[ player_t::white ];
			_pawns[player_t::black]    = rhs._pawns[ player_t::black ];

			for (int i = 0; i < 64; i++)
			{
				_pieces[i] = rhs._pieces[i];
			}

			_ply = rhs._ply;

			_queens[player_t::white]   =
				rhs._queens[player_t::white];
			_queens[player_t::black]   =
				rhs._queens[player_t::black];

			_rooks[player_t::white]    =
				rhs._rooks[ player_t::white ];
			_rooks[player_t::black]    =
				rhs._rooks[ player_t::black ];

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
				&& _castle_rights[i][player_t::black] ==
					rhs._castle_rights[i][player_t::black];

			same = same
				&& _castle_rights[i][player_t::white] ==
					rhs._castle_rights[i][player_t::white];

			same = same && _ep_info[i]
				== rhs._ep_info[i];
		}

		return same;
	}

	/**
	 * Compare this Position with another at a given ply
	 *
	 * @note A Position stores internal data indexed by ply; if Position
	 *       P makes/unmakes a move, then its data at the *next* ply
	 *       will be different, and the equality (==) operator will note
	 *       that P has changed. Here, we are interested in comparing
	 *       the data located at index \a ply
	 *
	 * @param[in] rhs The Position to compare against
	 * @param[in] ply The ply at which to perform the comparison
	 *
	 * @return True if they are the same
	 */
	bool Position::equals(const Position& rhs, int ply) const
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

		same = same
			&& _save_hash[ply] == rhs._save_hash[ply];

		same = same
			&& _castle_rights[ply][player_t::black] ==
				rhs._castle_rights[ply][player_t::black];

		same = same
			&& _castle_rights[ply][player_t::white] ==
				rhs._castle_rights[ply][player_t::white];

		same = same && _ep_info[ply]
			== rhs._ep_info[ply];

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

		for (int i = 0; i < 2; i++)
		{
			_hash_input.castle_rights[player_t::black][i] = rand64();
			_hash_input.castle_rights[player_t::white][i] = rand64();
		}

		for (int i = 0; i < 8; i++)
			_hash_input.en_passant[i] = rand64();

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				_hash_input.piece[player_t::black][i][j]  = rand64();
				_hash_input.piece[player_t::white][i][j]  = rand64();
			}
		}

		_hash_input.to_move = rand64();

		/*
		 * Compute the hash signature for this position
		 */

		uint64& signature  = _save_hash[_ply];
		signature = 0;

		if (_ep_info[_ply].target != square_t::BAD_SQUARE)
			signature ^=
				_hash_input.en_passant[ get_file( _ep_info[_ply].target) ];

		if (_to_move == player_t::white)
			signature ^= _hash_input.to_move;

		if (_castle_rights[_ply][player_t::white] & castle_K)
			signature ^=
				_hash_input.castle_rights[player_t::white][castle_K_index];
		if (_castle_rights[_ply][player_t::white] & castle_Q)
			signature ^=
				_hash_input.castle_rights[player_t::white][castle_Q_index];
		if (_castle_rights[_ply][player_t::black] & castle_K)
			signature ^=
				_hash_input.castle_rights[player_t::black][castle_K_index];
		if (_castle_rights[_ply][player_t::black] & castle_Q)
			signature ^=
				_hash_input.castle_rights[player_t::black][castle_Q_index];

		auto& tables = DataTables::get();

		for (int i = 0; i < 64; i++)
		{
			if (_pieces[i] != piece_t::empty)
			{
				if (_occupied[player_t::black] & tables.set_mask[i])
				{
					signature ^=
						_hash_input.piece[ player_t::black ][ _pieces[i] ][i];
				}
				else
				{
					signature ^=
						_hash_input.piece[ player_t::white ][ _pieces[i] ][i];
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
					static_cast<bool>(tables.set_mask[i] & _occupied[player_t::white]);

				if (empty != 0)
				{
					fen += empty_s;  empty = 0;
				}

				switch (_pieces[i])
				{
				case piece_t::pawn:
					fen += (whitePiece ? "P" : "p");
					break;
				case piece_t::knight:
					fen += (whitePiece ? "N" : "n");
					break;
				case piece_t::bishop:
					fen += (whitePiece ? "B" : "b");
					break;
				case piece_t::rook:
					fen += (whitePiece ? "R" : "r");
					break;
				case piece_t::queen:
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

		if (_to_move == player_t::white)
			fen += " w ";
		else
			fen += " b ";

		if (_castle_rights[_ply][player_t::white] & castle_K)
			fen += "K";
		if (_castle_rights[_ply][player_t::white] & castle_Q)
			fen += "Q";
		if (_castle_rights[_ply][player_t::black] & castle_K)
			fen += "k";
		if (_castle_rights[_ply][player_t::black] & castle_Q)
			fen += "q";

		if (_castle_rights[_ply][player_t::white] == 0 &&
			_castle_rights[_ply][player_t::black] == 0)
			fen += "-";

		fen += " ";

		if (_ep_info[_ply].target != square_t::BAD_SQUARE)
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

				if (_occupied[player_t::black] & (one << sq))
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

		std::vector<std::string> tokens; Util::split(fen, tokens, "/");

		if (tokens.size() != 8)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (wrong number of ranks): '%s'\n",
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
				if (is_piece(c))
				{
					uint64 mask = Util::get_bit<uint64>(square);
					_pieces[square] = piece2enum(c);

					if (Util::to_lower(c) == c)
						_occupied[player_t::black] |= mask;
					else
						_occupied[player_t::white] |= mask;
					square -= 1;

					switch (c)
					{
						case 'p':
							_pawns[player_t::black]   |= mask; break;
						case 'P':
							_pawns[player_t::white]   |= mask; break;
						case 'r':
							_rooks[player_t::black]   |= mask; break;
						case 'R':
							_rooks[player_t::white]   |= mask; break;
						case 'n':
							_knights[player_t::black] |= mask; break;
						case 'N':
							_knights[player_t::white] |= mask; break;
						case 'b':
							_bishops[player_t::black] |= mask; break;
						case 'B':
							_bishops[player_t::white] |= mask; break;
						case 'q':
							_queens[player_t::black]  |= mask; break;
						case 'Q':
							_queens[player_t::white]  |= mask; break;
						case 'k':
							_kings [player_t::black]  |= mask;
							_king_sq[player_t::black]  =
								static_cast<square_t>(Util::get_lsb(mask));
							break;
						case 'K':
							_kings [player_t::white]  |= mask;
							_king_sq[player_t::white]  =
								static_cast<square_t>(Util::get_lsb(mask));
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
						if (verbosity >= Verbosity::terse && _output)
						{
							_output->write("Invalid FEN (unexpected character '%c'): %s\n",
								c, fen.c_str());
						}
						
						*this = backup;
						return false;
					}
				}

				if ((square < 0 && i != 7) || square < -1)
				{
					if (verbosity >= Verbosity::terse && _output)
					{
						_output->write("Invalid FEN (more than 64 squares given): '%s'\n",
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
		Util::split(tokens.back(), posn_info, " ");

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
					if (verbosity >= Verbosity::terse && _output)
					{
						_output->write("Invalid FEN (fullmove number): '%s'\n",
							fen.c_str());
					}

					*this = backup;
					return false;
				}
			case 5:
				if (!Util::from_string(posn_info[4], _half_move))
				{
					if (verbosity >= Verbosity::terse && _output)
					{
						_output->write("Invalid FEN (halfmove clock): '%s'\n",
							fen.c_str());
					}

					*this = backup;
					return false;
				}
			case 4:
				_ep_info[_ply].target = square_t::BAD_SQUARE;

				if (posn_info[3] != "-")
				{
					for (int i = 0; i < 64; i++)
					{
						if (Util::to_lower(posn_info[3]) == square_str[i])
						{
							_ep_info[_ply].target =
								static_cast<square_t>(i);
							break;
						}
					}

					if (_ep_info[_ply].target == square_t::BAD_SQUARE)
					{
						if (verbosity >= Verbosity::terse && _output)
						{
							_output->write("Invalid FEN (en passant square): '%s'\n",
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
							_castle_rights[_ply][player_t::white] |= castle_K;
							break;
						case 'Q':
							_castle_rights[_ply][player_t::white] |= castle_Q;
							break;
						case 'k':
							_castle_rights[_ply][player_t::black] |= castle_K;
							break;
						case 'q':
							_castle_rights[_ply][player_t::black] |= castle_Q;
							break;
						case '-':
							if (posn_info[2].size() == 1)
								continue;
						default:
							if (verbosity >= Verbosity::terse && _output)
							{
								_output->write("Invalid FEN (castling rights): '%s'\n",
									fen.c_str());
							}
							
							*this = backup;
							return (false);
					}
				}
			case 2:
				if (posn_info[1] != "w" && posn_info[1] != "b")
				{
					if (verbosity >= Verbosity::terse && _output)
					{
						_output->write("Invalid FEN (invalid color): '%s'\n",
							fen.c_str());
					}
					
					*this = backup;
					return false;
				}
				_to_move = posn_info[1] == "w" ? player_t::white : player_t::black;
				break;
			case 1:
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (unspecified color): '%s'\n",
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
		
		if (_ep_info[_ply].target != square_t::BAD_SQUARE)
		{
			victim = (_to_move == player_t::white) ? 
				_ep_info[_ply].target-8 : _ep_info[_ply].target+8;

			src =
				_pawns[ _to_move ] & tables.rank_adjacent[victim];

			if (src & (tables.set_mask[victim+1]))
				_ep_info[_ply].src[0] =
					static_cast<square_t>(victim+1);

			if (src & (tables.set_mask[victim-1]))
				_ep_info[_ply].src[1] =
					static_cast<square_t>(victim-1);
		}

		/*
		 * Validate the new position. If it violates any of the rules
		 * of chess, reject it
		 */
		if (!_validate(fen))
		{
			*this = backup;
				return(false);
		}

		/*
		 * Compute the material balance. This avoids
		 * having to do so during static eval
		 */
		_material =
			Util::bit_count(_pawns[player_t::white])   * pawn_value   +
			Util::bit_count(_knights[player_t::white]) * knight_value + 
			Util::bit_count(_bishops[player_t::white]) * bishop_value + 
			Util::bit_count(_rooks[player_t::white])   * rook_value   +
			Util::bit_count(_queens[player_t::white])  * queen_value;

		_material -=
			Util::bit_count(_pawns[player_t::black])   * pawn_value   + 
			Util::bit_count(_knights[player_t::black]) * knight_value + 
			Util::bit_count(_bishops[player_t::black]) * bishop_value + 
			Util::bit_count(_rooks[player_t::black])   * rook_value   +
			Util::bit_count(_queens[player_t::black])  * queen_value;

		/*
		 * Generate a hash signature for this position
		 */
		generate_hash();

		_is_init = true;
			return _is_init;
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
			_king_sq[i]  = square_t::BAD_SQUARE;
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
		_to_move   = player_t::white;

		for (int i = 0; i < max_ply; i++)
		{
			_ep_info[i].clear();

			_castle_rights[i][player_t::black] = 0;
			_castle_rights[i][player_t::white] = 0;

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
	bool Position::_validate(const std::string& fen) const
	{
		// Rule 1:
		if ((_pawns[player_t::black] | _pawns[player_t::white]) & (rank_1 | rank_8))
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (pawn(s) on back rank): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 2:
		if ((Util::bit_count(_kings[player_t::white]) != 1) ||
			(Util::bit_count(_kings[player_t::black]) != 1))
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (wrong number of kings): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 3:
		if (in_check(flip(_to_move)))
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (king can be captured): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 4:
		int castle_mask = castle_K | castle_Q;

		if (!(_kings[player_t::white] & Util::get_bit< uint64 >(square_t::E1)))
		{
			if (_castle_rights[_ply][player_t::white] & castle_mask)
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::white may not castle): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}
		else
		{
			if ((_castle_rights[_ply][player_t::white] & castle_K)
				 && !(_rooks[player_t::white] & Util::get_bit<uint64>(square_t::H1)))
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::white may not castle short): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}

			if ((_castle_rights[_ply][player_t::white] & castle_Q)
				 && !(_rooks[player_t::white] & Util::get_bit<uint64>(square_t::A1)))
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::white may not castle long): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}
		}

		if (!(_kings[player_t::black] & Util::get_bit< uint64 >(square_t::E8)))
		{
			if (_castle_rights[_ply][player_t::black] & castle_mask)
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::black may not castle): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}
		else
		{
			if ((_castle_rights[_ply][player_t::black] & castle_K)
				 && !(_rooks[player_t::black] & Util::get_bit<uint64>(square_t::H8)))
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::black may not castle short): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}

			if ((_castle_rights[_ply][player_t::black] & castle_Q)
				 && !(_rooks[player_t::black] & Util::get_bit<uint64>(square_t::A8)))
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (player_t::black may not castle long): '%s'\n",
						fen.c_str());
				}
				
				return false; 
			}
		}

		// Rule 5:
		if (_ep_info[_ply].target != square_t::BAD_SQUARE)
		{
			bool bad_ep = false;

			if (_to_move == player_t::white)
			{
				if (get_rank(_ep_info[_ply].target) != 5 ||
					!(_pawns[player_t::black] &
						Util::get_bit<uint64>(_ep_info[_ply].target-8)))
				{
					bad_ep = true;
				}
			}
			else
			{
				if (get_rank(_ep_info[_ply].target) != 2 ||
					!(_pawns[player_t::white] &
						Util::get_bit<uint64>(_ep_info[_ply].target+8)))
				{
					bad_ep = true;
				}
			}

			if (bad_ep)
			{
				if (verbosity >= Verbosity::terse && _output)
				{
					_output->write("Invalid FEN (En passant square): '%s'\n",
						fen.c_str());
				}
				
				return false;
			}
		}

		// Rule 6:
		if (Util::bit_count<uint64>(_pawns[player_t::white]) > 8 || 
			Util::bit_count<uint64>(_pawns[player_t::black]) > 8)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (Max 8 pawns allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		// Rule 7:
		if (Util::bit_count<uint64>(_knights[player_t::white]) > 10 ||
			Util::bit_count<uint64>(_knights[player_t::black]) > 10)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (Max 10 knights allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_rooks[player_t::white]) > 10 ||
			Util::bit_count<uint64>(_rooks[player_t::black]) > 10)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (Max 10 rooks allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_queens[player_t::white]) > 10 ||
			Util::bit_count<uint64>(_queens[player_t::black]) > 10)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (Max 10 queens allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}

		if (Util::bit_count<uint64>(_bishops[player_t::white]) > 10 ||
			Util::bit_count<uint64>(_bishops[player_t::black]) > 10)
		{
			if (verbosity >= Verbosity::terse && _output)
			{
				_output->write("Invalid FEN (Max 10 bishops allowed per side): '%s'\n",
					fen.c_str());
			}
			
			return false;
		}
		
		return true;
	}
}
