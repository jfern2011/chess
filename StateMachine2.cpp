#include "StateMachine2.h"

StateMachine::StateMachine(CommandInterface& cmd, Logger& logger)
	: _cmd(cmd),
	  _current_state(none),
	  _is_init(false),
	  _logger(logger),
	  _logging_enabled(true),
	  _name("StateMachine"),
	  _pending_state(idle),
	  _transitions()
{
}

StateMachine::~StateMachine()
{
}

bool StateMachine::acknowledge_transition()
{
	AbortIf(_pending_state == _current_state,
		false);

	AbortIfNot(_is_init, false);

	const std::string old   = _state_names[_current_state];
	const std::string young = _state_names[_pending_state];

	const state_v& reachables =
		_transitions[_current_state];

	for (size_t i = 0; i < reachables.size(); i++)
	{
		if (reachables[i] == _pending_state)
		{
			_current_state = _pending_state;
			if (_logging_enabled)
			{
				_logger.write(_name,
					"changed states from '%s' to '%s'\n",
					old.c_str(), young.c_str());
			}

			return true;
		}
	}

	if (_logging_enabled)
	{
		_logger.write(_name,
			"unable to change states from '%s' to '%s'\n",
			old.c_str(), young.c_str());
	}

	/*
	 * The request failed, so reset the pending
	 * state:
	 */
	_pending_state =
			_current_state;

	return false;
}

void StateMachine::disable_logging()
{
	_logging_enabled = false;
}

void StateMachine::enable_logging()
{
	_logging_enabled = true;
}

StateMachine::state_t StateMachine::get_current_state() const
{
	return _current_state;
}

bool StateMachine::init()
{
	_transitions.resize(n_states);
	
	/*
	 * Set the state(s) we can transition to from
	 * the default state:
	 */
	_transitions[none].push_back(idle);

	/*
	 * Set the state(s) we can transition to from
	 * 'idle'
	 */
	_transitions[idle].push_back(searching);

	/*
	 * Set the state(s) we can transition to from
	 * 'searching'
	 */
	_transitions[searching].push_back(idle);

	AbortIfNot(_logger.register_source(_name),
		false);

	/*
	 * Assign the state names (informational only)
	 */
	_state_names.resize(n_states);

	_state_names[none]      = "none";
	_state_names[idle]      = "idle";
	_state_names[searching] = "searching";

	_is_init = true;

	/*
	 * We're done initializing; transition to the
	 * 'idle' state:
	 */
	AbortIfNot(acknowledge_transition(),
		false);

	return true;
}

bool StateMachine::pending_request() const
{
	return _pending_state != _current_state;
}

/**
 * Poll the commanding interface for user inputs, which
 * will enable command handlers to request a state
 * transition if needed
 *
 * @return True on success
 */
bool StateMachine::poll()
{
	AbortIfNot(_cmd.poll(), false);

	return true;
}

bool StateMachine::request_transition(state_t state)
{
	_pending_state = state;
	return true;
}
