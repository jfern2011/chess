#include "StateMachine.h"

/**
 * Constructor
 *
 * @param[in] name The name to give to this state
 */
State::State(const std::string& name)
	: _name(name), _tasks()
{
}

/**
 * Destructor
 */
State::~State()
{
}

/**
 * Retrieve the name of this state
 *
 * @return The state name
 */
std::string State::get_name() const
{
	return _name;
}

/**
 * Iterate through all tasks in this state until the next transition
 *
 * @return True on success
 */
bool State::run()
{
}

/**
 * Constructor
 */
StateMachine::StateMachine()
	: _cmd(),
	  _current_state(init),
	  _name_to_id(),
	  _states(),
	  _transitions()
{
}

/**
 * Destructor
 */
StateMachine::~StateMachine()
{
}

bool StateMachine::build(int fd)
{
	_states.resize(num_states);

	_states[search] = State("search");
	_states[init] = State("init");
	_states[idle] = State("idle");
	_states[ponder] = State("ponder");

	_transitions.resize(_states.size());

	AbortIfNot(get_transitions(search, _transitions[search]),
		false);
	AbortIfNot(get_transitions(init, _transitions[init]),
		false);
	AbortIfNot(get_transitions(idle, _transitions[idle]),
		false);
	AbortIfNot(get_transitions(ponder, _transitions[ponder]),
		false);

	for (size_t i = 0; i < _states.size(); i++)
	{
		_name_to_id[_states[i].get_name()] = i;
	}

	AbortIfNot(_cmd.init(fd), false);

	AbortIfNot(_cmd.install<StateMachine>("goto", *this,
		&StateMachine::request_transition), false);

	return true;
}

/**
 * Get a list of all possible transitions from a given
 * state
 *
 * @param[in]  state       The "from" state
 * @param[out] transitions The set of reachable states
 *
 * @return True on success
 */
bool StateMachine::get_transitions(state_t state,
	state_v& transitions) const
{
	transitions.clear();

	switch(state)
	{
	case search:
		transitions.push_back(ponder);
		transitions.push_back(idle);
		break;
	case ponder:
		transitions.push_back(search);
		transitions.push_back(idle);
		break;
	case init:
		transitions.push_back(idle);
		break;
	case idle:
		transitions.push_back(search);
		transitions.push_back(ponder);
		break;
	default:
		Abort(false);
	}

	return true;
}

/**
 * Request a state transition
 *
 * @param[in] state The name of the state to transition to
 *
 * @return True on success, or false if the requested state is not
 *         reachable from our current state
 */
bool StateMachine::request_transition(
	const std::string& state)
{
	auto iter = _name_to_id.find(state);
	AbortIf(iter == _name_to_id.end(),
		false);

	state_v reachables;
	AbortIfNot(get_transitions(_current_state, reachables),
		false);

	for (size_t i = 0; i < reachables.size(); i++)
	{
		if (reachables[i] == iter->second)
		{
			_current_state = reachables[i];
			return true;
		}
	}

	return false;
}
