#include "StateMachine.h"

/**
 * Default constructor
 */
State::State()
	: _name(""), _tasks()
{
}

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
 * Step through and invoke all tasks in this
 * state
 */
void State::run()
{
	for (size_t i = 0; i < _tasks.size(); i++)
		_tasks[i]->v_raise();
}

/**
 * Constructor
 */
StateMachine::StateMachine()
	: _cmd(),
	  _current_state(init),
	  _name_to_id(),
	  _exit(false),
	  _ready(false),
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

/**
 * Build the state machine.
 *
 * @param[in] fd The file descriptor to read for transition commands
 *
 * @return True on success
 */
bool StateMachine::build(int fd)
{
	_states.resize(num_states);

	/*
	 * Create the individual states but don't add tasks
	 * yet
	 */
	_states[search] = State("search");
	_states[init] = State("init");
	_states[idle] = State("idle");
	_states[ponder] = State("ponder");

	_transitions.resize(_states.size());

	for (size_t i = 0; i < _states.size(); i++)
	{
		state_t i_ = static_cast<state_t>(i);
		AbortIfNot(get_transitions(i_, _transitions[i]),
			false);

		_name_to_id[_states[i].get_name()] = i_;
	}

	AbortIfNot(_cmd.init(fd), false);

	AbortIfNot(_cmd.install<StateMachine>("goto", *this,
		&StateMachine::request_transition), false);

	AbortIfNot(_cmd.install<StateMachine>("quit", *this,
		&StateMachine::quit), false);

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
	state_v& transitions)
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
 * A handler for the "quit" command.
 *
 * @param[in] str Unused, but required to match the signature that
 *                CommandInterface expects
 *
 * @return True on success
 */
bool StateMachine::quit(const std::string& str)
{
	return _exit = true;
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
		/*
		 * Check if the desired state is reachable from our
		 * current state:
		 */
		if (reachables[i] == iter->second)
		{
			_current_state = reachables[i];
			return true;
		}
	}

	return false;
}

/**
 * Run the state machine. This calls run() on the current state to
 * perform a predetermined set of tasks while in that state. Once
 * this has started, the only way to exit is by issuing the "quit"
 * command
 *
 * @return True on success
 */
bool StateMachine::run()
{
	AbortIfNot(_ready, false);

	while (!_exit)
	{
		_states[_current_state].run();
	}
	
	return true;
}

/**
 * Once this has been called, no additional tasks can be added to
 * States
 */
void StateMachine::seal()
{
	_ready = true;
}
