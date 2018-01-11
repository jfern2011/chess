#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "log.h"
#include "movegen2.h"

/**
 * @class EngineInputs
 *
 * Stores inputs from the GUI for retrieval by the search
 * algorithm. Most of these are sent as part of the UCI
 * "go" command, but a few (e.g. hash table size) are UCI
 * UCI engine options
 *
 * @todo Update for xBoard-specific inputs
 */
class EngineInputs
{

public:

	EngineInputs(const DataTables& tables, Logger& logger);

	~EngineInputs();

	bool get_debug() const;

	int get_depth() const;

	int get_hash_size() const;

	int get_increment(int side) const;

	int get_mate_depth()   const;

	bool get_mate_search() const;

	int get_movestogo() const;

	int get_movetime() const;

	int64 get_node_limit() const;

	bool get_ponder() const;

	const Position* get_position()  const;

	int get_time(int side) const;

	bool init(const Position& pos);

	bool searchmoves( const std::string& moves );
	
	void set_debug(bool debug);

	bool set_depth(int depth);

	void set_fixed_searchdepth(bool val);

	void set_fixed_searchnodes(bool val);

	void set_fixed_searchtime(bool val);

	void set_hash_size(int size);

	bool set_increment(int ms, int side);

	bool set_mate_depth(int moves);

	void set_mate_search(bool val);

	bool set_movestogo(int moves);

	bool set_movetime(int ms);

	void set_multipv(int lines);

	bool set_node_limit(int64 max);

	void set_ponder(bool on);

	bool set_position(const Position& pos);

	bool set_time(int ms, int side);

	bool use_fixed_searchdepth() const;

	bool use_fixed_searchnodes() const;

	bool use_fixed_searchtime()  const;

private:

	bool _is_legal(int move) const;

	/**
	 * Black's increment per move, in milliseconds
	 */
	int _binc;

	/**
	 * The number of milliseconds on black's clock
	 */
	int _btime;

	/**
	 * The value of the UCI "debug" option
	 */
	bool _debug;

	/**
	 * Limit the search to this many plies
	 */
	int _depth;

	/**
	 * If true, search for \ref _depth plies
	 */
	bool _fixed_searchdepth;

	/**
	 * If true, search \ref _nodes nodes
	 */
	bool _fixed_searchnodes;

	/**
	 *  If true, search for exactly \ref _movetime
	 *  milliseconds
	 */
	bool _fixed_searchtime;

	/**
	 * Size of the hash tables, in MB
	 */
	int _hash_size;

	/**
	 * True if \ref init() was called
	 */
	bool _is_init;

	/**
	 * Used for logging activity
	 */
	Logger& _logger;

	/**
	 *  The number of moves to search for a mate in
	 */
	int _mate;

	/**
	 * If true, the engine will run a mate search
	 */
	bool _mate_search;

	/**
	 * The number of moves left in the current time
	 * control
	 */
	int _movestogo;

	/**
	 * Search for exactly this many milliseconds
	 */
	int _movetime;

	/**
	 * Display this many best lines
	 */
	int _multipv;

	/**
	 * The name of this module (for logging purposes)
	 */
	const std::string _name;

	/**
	 * Search until we've hit this many nodes
	 */
	int64 _nodes;

	/**
	 * True if pondering is enabled
	 */
	bool _ponder;

	/**
	 * The chess position to search
	 */
	Position* _position;

	/**
	 * The moves to consider on the next
	 * search
	 */
	std::vector<int> _search_moves;

	/**
	 * The set of pre-initialized tables
	 */
	const DataTables&
		_tables;

	/**
	 * White's increment per move, in milliseconds
	 */
	int _winc;

	/**
	 * The number of milliseconds on white's clock
	 */
	int _wtime;
};

#endif
