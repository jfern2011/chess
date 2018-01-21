#include "StateMachine3.h"

/**
 * Constructor
 *
 * @param[in] name The name of this state
 */
State::State(const std::string& name)
	: _name(name), _tasks()
{
}

/**
 * Copy constructor
 *
 * @param[in] other Create a copy of this
 */
State::State(const State& other)
{
	*this = other;
}

/**
 * Destructor
 */
State::~State()
{
}

/**
 * Copy assignment operator
 *
 * @param[in] rhs Assign ourselves to this
 */
State& State::operator=(const State& rhs)
{
	if (this != &rhs)
	{
		_tasks.resize(rhs._tasks.size());

		for (size_t i = 0; i < _tasks.size(); i++)
			_tasks[i].reset(
				rhs._tasks[i]->clone() );

		_name = rhs._name;
	}

	return *this;
}

/**
 * Add a new task to perform while in this state
 *
 * @param[in] task The task to run
 *
 * @note  Ownership of \a task is transferred to
 *        this instance
 *
 * @return True on success
 */
bool State::add_task(Signal::generic* task)
{
	AbortIfNot(task, false);

	_tasks.push_back(std::unique_ptr<Signal::generic>(task));
	return true;
}

/**
 * Run each task, making a single pass through
 * the task list
 */
void State::execute()
{
	for (size_t i = 0; i < _tasks.size(); i++)
		_tasks[i]->v_raise();
}

/**
 * Get the name of this State
 *
 * @return The state name
 */
std::string State::get_name() const
{
	return _name;
}

/**
 * Constructor
 *
 * @param [in] cmd    The commanding interface, which allows user
 *                    inputs to trigger state changes
 * @param [in] logger For logging state machine activity
 */
StateMachine::StateMachine(CommandInterface& cmd, Logger& logger)
	: _clients(),
	  _cmd(cmd),
	  _current_state(none),
	  _is_init(false),
	  _logger(logger),
	  _logging_enabled(true),
	  _name("StateMachine"),
	  _pending_state(idle),
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
 * Add a new task for the state machine to run while in
 * the specified state
 *
 * @note This releases ownership of \a task
 *
 * @param[in] state The state in which to run this task
 * @param[in] task  The task to run
 *
 * @return True on success
 */
bool StateMachine::add_task(state_t state, Signal::generic* task)
{
	AbortIfNot(state < n_states, false);
	AbortIfNot(task, false);

	AbortIfNot(_states[state]->add_task(task),
		false);

	return true;
}

/**
 * Disable logging :(
 */
void StateMachine::disable_logging()
{
	_logging_enabled = false;
}

/**
 * Enable logging :)
 */
void StateMachine::enable_logging()
{
	_logging_enabled = true;
}

/**
 * Get the state machine's current state
 *
 * @return The current state
 */
auto StateMachine::get_current_state() const -> state_t
{
	return _current_state;
}

/**
 * Initialize. If successful, this will transition
 * us into 'idle'
 *
 * @return True on success
 */
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
	_transitions[idle].push_back(init_search);
	_transitions[idle].push_back(exiting);

	/*
	 * Set the state(s) we can transition to from
	 * 'searching'
	 */
	_transitions[searching].push_back(init_search);
	_transitions[searching].push_back(exiting);
	_transitions[searching].push_back(post_search);

	/*
	 * Set the state(s) we can transition to from
	 * 'init_search'
	 */
	_transitions[init_search].push_back(searching);

	/*
	 * Set the state(s) we can transition to from
	 * 'post_search'
	 */
	_transitions[post_search].push_back(idle);

	AbortIfNot(_logger.register_source(_name),
		false);

	/*
	 * Create the states themselves:
	 */
	_states.resize(n_states);

	_states[none].reset(new State("none"));
	_states[idle].reset(new State("idle"));
	_states[init_search].reset(new State("init_search"));
	_states[searching].reset(new State("searching"));
	_states[post_search].reset(new State("post_search"));
	_states[exiting].reset(new State("exiting"));

	_is_init = true;

	/*
	 * We're done initializing; transition to the
	 * 'idle' state:
	 */
	AbortIfNot(_acknowledge_transition(),
		false);

	return true;
}

/**
 * Register a user with the state machine, allowing it to make
 * state transition requests
 *
 * @param[in] client The subscriber
 *
 * @return True on success
 */
bool StateMachine::register_client(StateMachineClient* client)
{
	AbortIf(client == nullptr, false);

	const std::string name = client->get_name();
	AbortIf(name.size() == 0,
		false);

	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == name)
		{
			char msg[128];
			std::snprintf(msg,128,"duplicate client '%s'\n",
				name.c_str());

			Abort(false, msg);
		}
	}

	int id = _clients.size();

	_clients.push_back(name);
	AbortIfNot(
		client->state_update_sig.attach<StateMachine>(
			*this, &StateMachine::_request_transition), false);

	client->id = id;
	return true;
}

/**
 * Run the state machine. This does two things:
 *
 * 1. Poll the command interface, which will dispatch handlers
 *    to update the current state
 * 2. Make one pass through the list of tasks specific to this
 *    state
 *
 * @return True on success
 */
bool StateMachine::run()
{
	AbortIfNot( _current_state != none,
		false);

	if (!_cmd.poll())
		return false;

	_states[_current_state]->execute();
	return true;
}

/**
 * Acknowledge a pending state transition request
 *
 * @return True on success
 */
bool StateMachine::_acknowledge_transition()
{
	AbortIfNot(_is_init, false);

	const std::string old   =
		_states[_current_state]->get_name();
	const std::string young =
		_states[_pending_state]->get_name();

	if (!_pending_request())
	{
		if (_logging_enabled)
		{
			_logger.write(_name,
				"ignoring transition request %s -> %s\n",
				old.c_str(), young.c_str());
		}

		return true;
	}

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
					"changed states from %s to %s.\n",
					old.c_str(), young.c_str());
			}

			return true;
		}
	}

	if (_logging_enabled)
	{
		_logger.write(_name,
			"unable to change states from %s to %s.\n",
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

/**
 * Check to see if there are any pending state transition requests
 *
 * @return True if a request is pending
 */
bool StateMachine::_pending_request() const
{
	return _pending_state != _current_state;
}

/**
 * Request a state transition
 *
 * @param[in] client    The user making this request
 * @param[in] new_state Transition to this state
 *
 * @return True on success
 */
bool StateMachine::_request_transition(
	const StateMachineClient* client, state_t state)
{
	AbortIfNot(_is_init, false);
	AbortIf(_clients.size() <= client->id,
		false);

	if (_logging_enabled)
	{
		const std::string new_state =
			_states[ state ]->get_name();

		_logger.write(_name,
			"received transition request from %s: %s -> %s\n",
			client->get_name().c_str(),
			_states[_current_state]->get_name().c_str(),
			new_state.c_str());
	}

	_pending_state = state;

	if (!_acknowledge_transition())
	{
		if (_logging_enabled)
		{
			_logger.write(_name,
				"failed to complete a request from %s.\n",
				client->get_name().c_str());
		}

		return false;
	}

	return true;
}

/**
 * Constructor
 *
 * @param[in] name The name of this client (i.e. software
 *                 component)
 */
StateMachineClient::StateMachineClient(const std::string& name)
	: state_update_sig(), id(~0), _name(name)
{
}

/**
 * Destructor
 */
StateMachineClient::~StateMachineClient()
{
}

/**
 * Get the name of this client (for diagnostics)
 *
 * @return The state machine user
 */
std::string StateMachineClient::get_name() const
{
	return _name;
}
