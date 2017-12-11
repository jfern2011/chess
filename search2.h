#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "movegen2.h"
#include "StateMachine2.h"

class Search
{

public:

	/**
	 * Stores the results of the previous search
	 */
	struct SearchData
	{
		SearchData()
			: best_move(0), score(0)
		{
		}

		int best_move;
		int score;
	};

	Search(const MoveGen& movegen);

	virtual ~Search();

	const SearchData& get_search_data() const;

	virtual bool search(Position& pos) = 0;

protected:

	SearchData _data;

	const MoveGen& _movegen;
};

class PvSearch : public Search
{
	class InterruptHandler
	{

	public:

		InterruptHandler(StateMachine& state_machine)
			: _state_machine(state_machine)
		{
		}

		~InterruptHandler()
		{
		}

		bool abort()
		{
			/*
			 * Poll the command interface for user inputs, which will
			 * cause transition requests to flow into the state
			 * machine. Don't print abort messages on error; doing so 
			 * may just send high-rate spam to standard output
			 */
			if (!_state_machine.poll())
				return false;

			return _state_machine.pending_request()
					&& _state_machine.acknowledge_transition();
		}

	private:

		StateMachine&
			_state_machine;

	};

public:

	PvSearch(const MoveGen& movegen,
		     StateMachine& state_machine,
		     Logger& logger);

	~PvSearch();

	bool init();

	bool search(Position& pos);

private:

	int _search(Position& pos, int depth, int alpha, int beta,
				bool do_null);

	bool _abort_requested;

	int64 _input_check_delay;

	InterruptHandler
		_interrupt_handler;

	Logger& _logger;

	const std::string _name;

	int64 _next_input_check;
	int64 _node_count;
};

inline int PvSearch::_search(Position& pos, int depth, int alpha,
							 int beta, bool do_null)
{
	if (_next_input_check <= _node_count)
	{
		/*
		 * Check if this search was interrupted, e.g. by a user
		 * commmand:
		 */
		if (_interrupt_handler.abort())
		{
			_abort_requested = true;
			return beta;
		}

		_next_input_check = _node_count
			+ _input_check_delay;
	}

	uint32 moves[MAX_MOVES];

	return 0;
}

#endif
