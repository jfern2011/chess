#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "cmd.h"
#include "log.h"

class StateMachine
{

public:

	typedef enum
	{
		none      = 0,
		idle      = 1,
		searching = 2,
		n_states  = 3

	} state_t;

	typedef std::vector<state_t> state_v;

	StateMachine(CommandInterface& cmd, Logger& logger);

	~StateMachine();

	bool acknowledge_transition();

	void disable_logging();

	void enable_logging();

	state_t get_current_state() const;

	bool init();

	bool pending_request() const;

	bool poll();

	bool request_transition(state_t state);

private:

	CommandInterface&
		_cmd;

	state_t _current_state;

	bool _is_init;

	Logger& _logger;

	bool _logging_enabled;

	std::string _name;

	state_t _pending_state;

	std::vector<std::string>
		_state_names;

	/**
	 * Mapping from state to a corresponding set of reachable
	 * states
	 */
	std::vector<state_v>
		_transitions;
};

#endif
