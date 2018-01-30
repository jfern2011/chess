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
	: _binc(-1),
	  _btime(1000),
	  _debug(false),
	  _depth(-1),
	  _fixed_searchdepth(false),
	  _fixed_searchnodes(false),
	  _fixed_searchtime(false),
	  _hash_size(0),
	  _infinite_search( false ),
	  _is_init(false),
	  _logger(logger),
	  _mate(-1),
	  _mate_search(false),
	  _movestogo(MAX_MOVES),
	  _movetime(-1),
	  _multipv(1),
	  _name("EngineInputs"),
	  _nodes(-1),
	  _ponder(false),
	  _position(nullptr),
	  _search_moves(),
	  _tables(tables),
	  _winc(-1),
	  _wtime(1000)
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
 * Get the increment per move for the given player,
 * in milliseconds
 *
 * @param[in] side Get the increment for this side
 *
 * @return The increment per move
 */
int EngineInputs::get_increment(int side) const
{
	return side == WHITE ? _winc : _binc;
}

/**
 * Get the number of moves to search for a checkmate
 *
 * @return The number of moves (not plies) to search
 *         for mate, i.e. mate in 5
 */
int EngineInputs::get_mate_depth() const
{
	return _mate;
}

/**
 * Check whether or not to run a mate search
 *
 * @return True to search for a mate
 */
bool EngineInputs::get_mate_search() const
{
	return _mate_search;
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
 *  Get the amount of time to search for, in milliseconds
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
 * Get the position that will be searched
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
 * Get the infinite search flag. If true, the search
 * will continue until the user commands a state
 * change
 *
 * @return True to run an infinite search
 */
bool EngineInputs::run_infinite_search() const
{
	return _infinite_search;
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
	AbortIfNot(_is_init, false);

	if (moves.empty())
	{
		Buffer<int, MAX_MOVES> moves;

		MoveGen movegen(_tables);
		size_t n_moves =
		movegen.generate_legal_moves(*_position, _position->get_turn(),
			moves);

		_search_moves.resize(n_moves);

		for (size_t i = 0; i < n_moves; i++)
			_search_moves[i] = moves[i];

		_logger.write( _name, "searching all moves.\n" );

		return true;
	}

	Util::str_v list; Util::split( moves, list );

	_search_moves.clear();

	for (size_t i = 0; i < list.size(); i++)
	{
		const int move = Util::parse_coordinate(list[i]);

		if (!_is_legal(move))
		{
			_logger.write(_name, "Illegal move: '%s' \n",
				list[i].c_str());
			
			return false;
		}

		AbortIfNot(_position->make_move(move),
			false);
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
	if (depth < 0)
	{
		_logger.write(
			_name, "invalid search depth = %d\n", depth);

		return false;
	}

	_depth = depth;

	_logger.write(_name, "new search depth = %d plies\n",
		_depth);

	_fixed_searchdepth = true;
	return true;
}

/**
 * Tell the engine if it should search until depth \ref
 * _depth is reached
 *
 * @param[in] val If true, search for \ref _depth plies
 */
void EngineInputs::set_fixed_searchdepth(bool val)
{
	_fixed_searchdepth = val;
}

/**
 * Tell the engine if it should search only \ref _nodes
 * nodes before stopping
 *
 * @param[in] val If true, search for \ref _nodes nodes
 */
void EngineInputs::set_fixed_searchnodes(bool val)
{
	_fixed_searchnodes = val;
}

/**
 * Tell the engine if it should search for a fixed amount
 * of time
 *
 * @param[in] val If true, search for \ref _movetime ms
 */
void EngineInputs::set_fixed_searchtime(bool val)
{
	_fixed_searchtime = val;
}

/**
 * Set the total size allocated to hash tables
 *
 * @param[in] bytes The size, in MB
 */
void EngineInputs::set_hash_size(int size)
{
	if (size < 0)
	{
		_logger.write(
			_name, "invalid hash table size = %d\n", size);

		return;
	}

	_logger.write(_name, "setting hash tables to %d MB.\n",
		size);

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
	if (ms < 0)
	{
		_logger.write(_name, "[%s] invalid increment = %d\n",
			__FUNCTION__, ms  );
		return false;
	}

	if (side != WHITE && side != BLACK)
	{
		_logger.write(_name, "[%s] invalid player    = %d\n",
			__FUNCTION__, side);
		return false;
	}

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
 * Set the infinite search flag. This will cause the engine to ignore
 * time and depth constraints
 */
void EngineInputs::set_infinite_search(bool value)
{
	_infinite_search = value;
}

/**
 * Set the depth (in full moves) to search for a mate
 *
 * @param[in] moves Find a mate in this many moves
 *
 * @return True on success
 */
bool EngineInputs::set_mate_depth(int moves)
{
	if (moves < 0)
	{
		_logger.write(_name, "cannot set mate depth to %d\n",
			moves);
		return false;
	}

	_mate = moves;

	_logger.write(_name,
		"searching for a mate in %d...\n", _mate);

	_mate_search = true;
	return true;
}

/**
 * Tell the engine whether or not to run a mate search
 *
 * @param[in] val True to search for a mate
 */
void EngineInputs::set_mate_search(bool val)
{
	_mate_search = val;
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
	if (moves < 0)
	{
		_logger.write(_name, "[%s] invalid # of moves = %d. \n",
			__FUNCTION__, moves);
		return false;
	}

	_movestogo = moves;

	_logger.write(_name,
		"%d moves left in current time control.\n", _movestogo);

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
	if (ms < 0)
	{
		_logger.write(_name,
			"[%s] invalid search time = %d\n",__FUNCTION__,
			ms);
		return false;
	}

	_movetime = ms;

	_logger.write(_name, "setting search time to %d ms.\n",
		_movetime);

	_fixed_searchtime = true;
	return true;
}

/**
 * Set the number of best lines (principal variations) to
 * display
 *
 * @param[in] lines The number of lines to print
 */
void EngineInputs::set_multipv(int lines)
{
	if (lines < 0)
	{
		_logger.write(_name, "invalid number of PVs = %d\n",
			lines);
		return;
	}

	_logger.write( _name , "displaying %d best line(s). \n",
		lines);

	_multipv = lines;
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
	if (max < 0)
	{
		_logger.write(_name,
			"[%s] invalid node limit = %lld\n",__FUNCTION__,
			max);
		return false;
	}

	_nodes = max;

	_logger.write( _name, "limiting search to %d nodes. \n",
		_nodes);

	_fixed_searchnodes = true;
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
 * Set the internal copy of the position to \a pos, which will
 * be searched by \ref search()
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
	if (ms < 0)
	{
		_logger.write(_name, "[%s] invalid clock  = %d\n",
			__FUNCTION__, ms  );
		return false;
	}

	if (side != WHITE && side != BLACK)
	{
		_logger.write(_name, "[%s] invalid player = %d\n",
			__FUNCTION__, side);
		return false;
	}

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
 * Check whether the engine needs to search until \ref _depth
 * plies rather than maxing out
 *
 * @return True if the engine should search \ref _depth plies
 */
bool EngineInputs::use_fixed_searchdepth() const
{
	return _fixed_searchdepth;
}

/**
 * Check whether the engine needs to search until \ref _nodes
 * nodes before stopping
 *
 * @return True if the engine should search \ref _nodes nodes
 */
bool EngineInputs::use_fixed_searchnodes() const
{
	return _fixed_searchnodes;
}

/**
 * Check whether the engine needs to search for \ref _movetime
 * ms rather than figure out how to budget its time
 *
 * @return True if the engine should use \ref _movetime
 */
bool EngineInputs::use_fixed_searchtime() const
{
	return _fixed_searchtime;
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
