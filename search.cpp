#include "search2.h"

Search::Search(const MoveGen& movegen)
	: _data(),
	  _movegen(movegen)
{
}

Search::~Search()
{
}

const Search::SearchData& Search::get_search_data() const
{
	return _data;
}

PvSearch::PvSearch(const MoveGen& movegen,
				   StateMachine& state_machine,
				   Logger& logger)
	: Search(movegen),
	  _abort_requested(false),
	  _input_check_delay(100000),
	  _interrupt_handler(state_machine),
	  _logger(logger),
	  _name("PvSearch"),
	  _next_input_check(0),
	  _node_count(0)
{
}

PvSearch::~PvSearch()
{
}

bool PvSearch::init()
{
	AbortIfNot(_logger.register_source(_name),
		false);

	return true;
}

bool PvSearch::search(Position& pos)
{
	_abort_requested  = false;
	_next_input_check = _input_check_delay;
	_node_count       = 0;

	Position orig(pos); // do this every iteration

	return true;
}
