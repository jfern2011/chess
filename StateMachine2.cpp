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
 * defer acknowledgement until later; \ref _request_transition()
 * for details
 *
 * @return True on success
 */
bool StateMachine::acknowledge_transition()
{
	AbortIfNot(_is_init, false);

	const std::string old   = _state_names[_current_state];
	const std::string young = _state_names[_pending_state];

	if (!pending_request())
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
	_transitions[idle].push_back(init_search);
	_transitions[idle].push_back(exiting);

	/*
	 * Set the state(s) we can transition to from
	 * 'searching'
	 */
	_transitions[searching].push_back(init_search);
	_transitions[searching].push_back(exiting);
	_transitions[searching].push_back(postsearch);

	/*
	 * Set the state(s) we can transition to from
	 * 'init_search'
	 */
	_transitions[init_search].push_back(searching);

	/*
	 * Set the state(s) we can transition to from
	 * 'postsearch'
	 */
	_transitions[postsearch].push_back(idle);

	AbortIfNot(_logger.register_source(_name),
		false);

	/*
	 * Assign the state names (informational only)
	 */
	_state_names.resize(n_states);

	_state_names[none]        = "none";
	_state_names[idle]        = "idle";
	_state_names[init_search] = "init_search";
	_state_names[searching]   = "searching";
	_state_names[postsearch]  = "postsearch";
	_state_names[exiting]     = "exiting";

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
 * Poll the underlying command interface, which will send state
 * transition requests to this instance
 *
 * @return True on success
 */
bool StateMachine::poll() const
{
	AbortIfNot(_is_init, false);

	return _cmd.poll();
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

			Abort(false,msg);
		}
	}

	_clients.push_back(name);

	AbortIfNot(client->transition_sig.attach<StateMachine>(*this,
		&StateMachine::_request_transition), false);

	return true;
}

/**
 * Return the human-readable equivalent of a \ref state_t
 *
 * @param[in] state The value to convert
 *
 * @return The state name
 */
std::string StateMachine::to_string(state_t state) const
{
	return _state_names[state];
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
bool StateMachine::_request_transition(const std::string& _client,
	state_t state, bool defer)
{
	AbortIfNot(_is_init, false);

	const std::string client=Util::trim(_client);

	if (client.empty())
	{
		if (_logging_enabled)
		{
			_logger.write(_name,
				"unnamed client requested a state change!\n");
		}

		return false;
	}

	for ( size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i] == client)
		{
			if (_logging_enabled)
			{
				const std::string new_state = _state_names[state];

				_logger.write(_name,
					"received transition request from %s: %s -> %s\n",
					client.c_str(),
					_state_names[_current_state].c_str(),
					new_state.c_str());
			}

			_pending_state = state;
			if (!defer)
			{
				if (!acknowledge_transition())
				{
					if (_logging_enabled)
					{
						_logger.write(_name,
							"failed to complete a request from %s.\n",
							client.c_str());
					}

					return false;
				}
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
