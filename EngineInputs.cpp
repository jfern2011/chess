#include "EngineInputs.h"

/**
 * Constructor
 *
 * @param[in] tables The pre-initialized set of databases to use for
 *                   \ref searchmoves()
 * @param[in] logger The Logger that this component will write
 *                   diagnostics to
 */
EngineInputs::EngineInputs(const DataTables& tables, Logger& logger)
	: _binc(-1), _btime(-1),
	  _debug(false),
	  _depth(-1),
	  _hash_size(0),
	  _is_init(false),
	  _logger(logger),
	  _mate(-1),
	  _movestogo(MAX_MOVES),
	  _movetime(-1),
	  _name("EngineInputs"),
	  _nodes(-1),
	  _ponder(false),
	  _position(nullptr),
	  _search_moves(),
	  _tables(tables),
	  _winc(-1),
	  _wtime(-1)
{
}

/**
 * Destructor
 */
EngineInputs::~EngineInputs()
{
	if (_position) delete _position;
}

/**
 * Get the current value of the UCI debug option
 *
 * @return True if debugging is enabled
 */
bool EngineInputs::get_debug() const
{
	return _debug;
}

/**
 * Get the number of plies to be searched
 *
 * @return The search depth
 */
int EngineInputs::get_depth() const
{
	return _depth;
}

/**
 * Get the combined size of all hash tables
 *
 * @return The size, in MB
 */
int EngineInputs::get_hash_size() const
{
	return _hash_size;
}

/**
 * Get the increment per move for the given player
 *
 * @param[in] side Get the increment for this side
 *
 * @return The increment per move, in milliseconds
 */
int EngineInputs::get_increment(int side) const
{
	return side == WHITE ? _winc : _binc;
}

/**
 * Get the number of moves to search for a checkmate
 *
 * @return The number of moves (not plies) to search
 *         for mate
 */
int EngineInputs::get_mate_depth() const
{
	return _mate;
}

/**
 * Get the number of moves to the next time control
 *
 * @return The number of moves remaining under this
 *         time control
 */
int EngineInputs::get_movestogo() const
{
	return _movestogo;
}

/**
 * Get the amount of time to search for, in milliseconds
 *
 * @return The exact search time
 */
int EngineInputs::get_movetime() const
{
	return _movetime;
}

/**
 * Get the current limit on the number of nodes to search
 *
 * @return The number of nodes to search
 */
int64 EngineInputs::get_node_limit() const
{
	return _nodes;
}

/**
 * Get the pondering flag
 *
 * @return True if pondering is enabled
 */
bool EngineInputs::get_ponder() const
{
	return _ponder;
}

/**
 * Get the current position
 *
 * @return The internal position
 */
const Position* EngineInputs::get_position() const
{
	return _position;
}

/**
 * Get the amount of time left on a player's clock
 *
 * @param[in] side Get this player's clock
 *
 * @return The amount of time left in milliseconds
 */
int EngineInputs::get_time(int side) const
{
	return side == WHITE ? _wtime : _btime;
}

/**
 * Initialize.
 *
 * @param[in] pos Initialize with this position
 *
 * @return True on success
 */
bool EngineInputs::init(const Position& pos)
{
	AbortIf(_is_init, false);
	AbortIfNot(_logger.register_source(_name),
		false);

	_position = new Position(pos);

	_is_init = true;
	return true;
}

/**
 * Restrict searches to the given list of moves. This should be called
 * with every "go" UCI command
 *
 * @param[in] moves List of moves to search. If empty, all legal moves
 *                  will be searched
 *
 * @return True on success
 */
bool EngineInputs::searchmoves(const std::string& moves)
{
	if (moves.empty())
	{
		BUFFER(int, moves, MAX_MOVES);

		MoveGen movegen(_tables);
		size_t n_moves =
		movegen.generate_legal_moves(*_position, _position->get_turn(),
			moves);

		for (size_t i = 0; i < n_moves; i++)
			_search_moves.assign(moves, moves + n_moves);

		return true;
	}

	Util::str_v list; Util::split( moves, list );

	_search_moves.clear();

	for (size_t i = 0; i < list.size(); i++)
	{
		const int move = Util::parse_coordinate(list[i]);

		if (!_is_legal(move))
		{
			BUFFER(char, msg, 128);
			std::snprintf(
				msg, 128, "Illegal move: '%s' \n", list[ i ].c_str() );
			Abort(false, msg);
		}

		_search_moves.push_back(move);
	}

	if (!moves.empty())
		_logger.write(_name, 
			"restricting search to %s\n", moves.c_str());

	return true;
}

/**
 * Set the value of the UCI debug option
 *
 * @param[in] debug True or false
 */
void EngineInputs::set_debug(bool debug)
{
	std::string val;
	Util::to_string<bool>(debug, val);

	_logger.write(_name, "setting debug to %s.\n",
		val.c_str());

	_debug = debug;
}

/**
 * Set the number of plies to be searched
 *
 * @param [in] depth Search for this many plies of depth
 *
 * @return True on success
 */
bool EngineInputs::set_depth(int depth)
{
	AbortIf(depth < 0, false);

	_depth = depth;

	_logger.write(_name, "new search depth = %d plies\n",
		_depth);

	return true;
}

/**
 * Set the total size allocated to hash tables
 *
 * @param[in] bytes The size, in MB
 */
void EngineInputs::set_hash_size(int size)
{
	std::string val;
	Util::to_string<int>(size, val);

	_logger.write(_name, "setting hash tables to %s MB.\n",
		val.c_str());

	_hash_size = size;
}

/**
 * Set the time increment to give to the specified player
 *
 * @param[in] ms   The increment, in milliseconds
 * @param[in] side Assign the time increment to this side
 *
 * @return True on success
 */
bool EngineInputs::set_increment(int ms, int side)
{
	AbortIf(side != WHITE && side != BLACK, false);

	AbortIf(ms < 0, false);

	if (side == WHITE)
	{
		_winc = ms;
		_logger.write(_name, "setting white increment to %d ms.\n",
			_winc);
	}
	else
	{
		_binc = ms;
		_logger.write(_name, "setting black increment to %d ms.\n",
			_binc);
	}

	return true;
}

/**
 * Set the depth (in full moves) to search for a mate
 *
 * @param[in] moves The search depth
 *
 * @return True on success
 */
bool EngineInputs::set_mate_depth(int moves)
{
	AbortIf(moves < 0, false);

	_mate = moves;

	_logger.write(_name,
		"searching for a mate in %d...\n", _mate);

	return true;
}

/**
 * Set the number of moves left in the current time control
 *
 * @param[in] moves The number of moves left
 *
 * @return True on success
 */
bool EngineInputs::set_movestogo(int moves)
{
	AbortIf(moves < 0, false);

	_movestogo = moves;

	_logger.write(_name,
		"%d moves left in current time control", _movestogo);

	return true;
}

/**
 * Set the amount of time to run a search, in milliseconds
 *
 * @param[in] ms Search for this long
 *
 * @return True on success
 */
bool EngineInputs::set_movetime(int ms)
{
	AbortIf(ms < 0, false);

	_movetime = ms;

	_logger.write(_name, "setting search time to %d ms.\n",
		_movetime);

	return true;
}

/**
 * Limit the search to \a max nodes
 *
 * @param[in] max Search until we've hit this many nodes
 *
 * @return True on success
 */
bool EngineInputs::set_node_limit(int64 max)
{
	AbortIf(max < 0, false);
	_nodes = max;

	_logger.write(_name, "limiting search to %d nodes.\n",
		_nodes);

	return true;
}

/**
 * Enable or disable engine pondering
 *
 * @param[in] on True to enable
 */
void EngineInputs::set_ponder(bool on)
{
	if (on)
		_logger.write(_name, "pondering enabled. \n");
	else
		_logger.write(_name, "pondering disabled.\n");

	_ponder = on;
}

/**
 * Set the internal copy of the position to \a pos
 *
 * @param[in] pos The new position
 *
 * @return True on success
 */
bool EngineInputs::set_position( const Position& pos )
{
	AbortIfNot(_is_init, false);

	const std::string fen = pos.get_fen();
	_logger.write(_name, "setting position to [%s]\n",
		fen.c_str());

	*_position = pos;
	return true;
}

/**
 * Set the number of milliseconds left on a player's clock
 *
 * @param[in] ms   The number of milliseconds to apply
 * @param[in] side Set this guy's clock to \ms millseconds
 *
 * @return True on success
 */
bool EngineInputs::set_time(int ms, int side)
{
	AbortIf(side != WHITE && side != BLACK,
		false);

	AbortIf(ms < 0, false);

	if (side == WHITE)
	{
		_wtime = ms;
		_logger.write(_name, "setting white's clock to %d ms.\n",
			_wtime);
	}
	else
	{
		_btime = ms;
		_logger.write(_name, "setting black's clock to %d ms.\n",
			_btime);
	}

	return true;
}

/**
 * Determines if the given user move is legal. This checks the input
 * against all legal moves, and a match is found if the origin
 * square, the destination square, and the promotion piece are equal 
 *
 * @param[in] move The move to test for legality
 *
 * @return True if \a move is legal
 */
bool EngineInputs::_is_legal(int move) const
{
	MoveGen movegen(_tables);

	int moves[MAX_MOVES];
	size_t n_moves =
		movegen.generate_legal_moves(*_position, _position->get_turn(),
			moves);

	for (size_t i = 0; i < n_moves; i++)
	{
		if (FROM(move) == FROM(moves[i ]) && TO(move) == TO(moves[ i ])
			&& PROMOTE(move) == PROMOTE(moves[i]))
			return true;
	}

	return false;
}
