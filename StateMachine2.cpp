#include "StateMachine2.h"

/**
 * Constructor
 *
 * @param[in] cmd    The command interface, which allows user inputs
 *                   to drive the state machine
 * @param[in] logger For logging activity
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
 * Acknowledge a pending state transition request. This does not
 * need to be called explicitly unless the requestor asked to
 * defer acknowledgement until later in \ref request_transition()
 *
 * @return True on success
 */
bool StateMachine::acknowledge_transition()
{
	AbortIfNot(_is_init, false);

	AbortIfNot( pending_request(), false );

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
StateMachine::state_t StateMachine::get_current_state() const
{
	return _current_state;
}

/**
 * Initialize. If successful, this will cause a transition to
 * \ref idle
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
	_transitions[idle].push_back(searching);
	_transitions[idle].push_back(exiting);

	/*
	 * Set the state(s) we can transition to from
	 * 'searching'
	 */
	_transitions[searching].push_back(idle);
	_transitions[searching].push_back(exiting);

	AbortIfNot(_logger.register_source(_name),
		false);

	/*
	 * Assign the state names (informational only)
	 */
	_state_names.resize(n_states);

	_state_names[none]      = "none";
	_state_names[idle]      = "idle";
	_state_names[searching] = "searching";
	_state_names[exiting]   = "exiting";

	_is_init = true;

	/*
	 * We're done initializing; transition to the
	 * 'idle' state:
	 */
	AbortIfNot(acknowledge_transition(),
		false);

	return true;
}

/**
 * Check to see if there are any pending state transition requests
 *
 * @return True if a request is pending
 */
bool StateMachine::pending_request() const
{
	return _pending_state != _current_state;
}

/**
 *  Registers a user with this state machine, allowing it to issue
 *  transition requests
 *
 * @param[in] _name  The name of the subscriber
 * @param[in] client The subscriber itself
 *
 * @return True on success
 */
bool StateMachine::register_client(const std::string& _name,
	StateMachineClient* client)
{
	const std::string name = Util::trim(_name);
	AbortIf(name.size() == 0,
		false);

	AbortIf(client == nullptr, false);

	for (auto iter = _clients.begin(), end = _clients.end();
		 iter != end; ++iter)
	{
		if (*iter == name)
		{
			char msg[128];
			std::snprintf(msg,128,"duplicate client '%s'\n",
						name.c_str());
			Abort(false, msg);
		}
	}

	_clients.push_back(name);

	AbortIfNot(client->transition_sig.attach<StateMachine>(*this,
		&StateMachine::request_transition), false);

	return true;
}

/**
 * Request a state transition
 *
 * @param[in] _client The name of the user who is making this request
 * @param[in] state   Transition to this state
 * @param[in] defer   If true, do not transition yet. Instead, wait
 *                    until \ref acknowledge_transition() gets called
 *                    explicitly
 *
 * @return True on success
 */
bool StateMachine::request_transition(const std::string& _client,
	state_t state, bool defer)
{
	AbortIfNot(_is_init, false);

	const std::string client=Util::trim(_client);
	AbortIf(client.empty(), false);

	for ( size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client)
		{
			if (_logging_enabled)
			{
				const std::string new_state = _state_names[state];

				_logger.write(_name,
					"received transition request from '%s': %s -> %s\n",
					client.c_str(),
					_state_names[_current_state].c_str(),
					new_state.c_str());
			}

			_pending_state = state;
			if (!defer)
			{
				AbortIfNot(acknowledge_transition(),
					false);
			}

			return true;
		}
	}

	if (_logging_enabled)
	{
		_logger.write(_name, "unregistered client: '%s'\n",
			client.c_str());
	}

	return false;
}

/**
 * Constructor
 *
 * @param[in] name The name of this client (i.e. software
 *                 component)
 */
StateMachineClient::StateMachineClient(const std::string& name)
	: _name(name), transition_sig()
{
}

/**
 * Destructor
 */
StateMachineClient::~StateMachineClient()
{
}

/**
 * Get the name of this client. This is what makes us known
 * to the state machine
 *
 * @return The state machine user
 */
std::string StateMachineClient::get_name() const
{
	return _name;
}
