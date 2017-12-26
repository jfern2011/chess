#ifndef __INPUTS_H__
#define __INPUTS_H__

#include "log.h"
#include "movegen2.h"

class EngineInputs
{

public:

	EngineInputs(const DataTables& tables, Logger& logger);

	~EngineInputs();

	bool get_debug() const;

	int get_depth() const;

	int get_hash_size() const;

	int get_increment(int side) const;

	int get_mate_depth() const;

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

	void set_hash_size(int size);

	bool set_increment(int ms, int side);

	bool set_mate_depth(int moves);

	bool set_movestogo(int moves);

	bool set_movetime(int ms);

	bool set_node_limit(int64 max);

	void set_ponder(bool on);

	bool set_position(const Position& pos);

	bool set_time(int ms, int side);

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
	 * Size of the hash table, in MB
	 */
	int _hash_size;

	/**
	 * True if \ref init() was called
	 */
	bool _is_init;

	/**
	 *  The number of moves to search for a mate in
	 */
	int _mate;

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
	 * Log activity to this object
	 */
	Logger& _logger;

	/**
	 * The name of this module for logging
	 * purposes
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
	 * The current chess position (passed into Search::search())
	 */
	Position* _position;

	/**
	 * The moves to consider on the next
	 * search
	 */
	std::vector<int> _search_moves;

	/**
	 * White's increment per move, in milliseconds
	 */
	int _winc;

	/**
	 * The number of milliseconds on white's clock
	 */
	int _wtime;

	/**
	 * The set of pre-initialized databases
	 */
	const DataTables&
		_tables;
};

#endif
