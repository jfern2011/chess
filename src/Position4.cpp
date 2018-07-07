#include "Position4.h"

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
				_hash_input.en_passant[get_file(_ep_info[_ply].target)];

		if (_to_move == white)
			signature ^= _hash_input.to_move;

		if (_castle_rights[_ply][white] & castle_K)
			signature ^=
				_hash_input.castle_rights[white][_OO_INDEX];
		if (_castle_rights[_ply][white] & castle_Q)
			signature ^=
				_hash_input.castle_rights[white][OOO_INDEX];
		if (_castle_rights[_ply][black] & castle_K)
			signature ^=
				_hash_input.castle_rights[black][_OO_INDEX];
		if (_castle_rights[_ply][black] & castle_Q)
			signature ^=
				_hash_input.castle_rights[black][OOO_INDEX];

		for (int i = 0; i < 64; i++)
		{
			if (_pieces[i] != INVALID)
			{
				if (_occupied[black] & _tables.set_mask[i])
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
}
