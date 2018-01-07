#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "cmd.h"

class StateMachineClient;

/**
 * @class StateMachine
 *
 * A simple finite state machine used to determine what the engine is
 * doing at any given time. The list of available states is given
 * in the \ref state_t enumerated type definition. State transitions
 * are generally driven by user inputs, except for the search
 * algorithm, which requests a state transition whenever it begins or
 * finishes a computation
 */
class StateMachine
{

public:

	typedef enum
	{
		/**
		 * The default pre-initialized state
		 */
		none        = 0,

		/**
		 * Indicates the engine isn't doing anything and waiting for
		 * user inputs
		 */
		idle        = 1,

		/**
		 * Initializing for a new search
		 */
		init_search = 2,

		/**
		 * Indicates that a search is in progress. This also applies
		 * when pondering
		 */
		searching   = 3,

		/**
		 * Indicates a search has finished
		 */
		postsearch  = 4,

		/**
		 * Indicates the engine is exiting
		 */
		exiting     = 5,

		/**
		 * The number of states
		 */
		n_states    = 6

	} state_t;

	/**
	 * A container for \ref state_t items
	 */
	typedef std::vector<state_t> state_v;

	StateMachine(CommandInterface& cmd, Logger& logger);

	~StateMachine();

	bool acknowledge_transition();

	void disable_logging();

	void enable_logging();

	state_t get_current_state() const;

	bool init();

	bool pending_request() const;

	bool poll() const;

	bool register_client(const std::string& _name,
		StateMachineClient* client);

	std::string to_string(state_t state) const;

private:

	bool _request_transition(const std::string& _client,
		state_t state, bool defer=false);

	/**
	 * A record of all components registered with
	 * the state machine
	 */
	Util::str_v _clients;

	/**
	 * The command interface used to receive user inputs
	 */
	CommandInterface&
		_cmd;

	/**
	 * The engine's current state
	 */
	state_t _current_state;

	/**
	 * If true, initialization succeeded
	 */
	bool _is_init;

	/**
	 * Utility for logging activity
	 */
	Logger& _logger;

	/**
	 * If true, write activity to the logger
	 */
	bool _logging_enabled;

	/**
	 * The name of this component
	 */
	std::string _name;

	/**
	 * The pending state transition request
	 */
	state_t _pending_state;

	/**
	 * The names of all defined states
	 */
	std::vector<std::string>
		_state_names;

	/**
	 * Mapping from state to a set of reachable
	 * states
	 */
	std::vector<state_v>
		_transitions;
};

/**
 * @class StateMachineClient
 *
 * A StateMachineClient is allowed to make transition requests once
 * it has registered with a StateMachine. This allows derived
 * classes alone to drive the state machine while preventing others
 * from modifying the engine's behavior by forcing it into an
 * undesired state
 */
class StateMachineClient
{

public:

	StateMachineClient(const std::string& name);

	virtual ~StateMachineClient();

	std::string get_name() const;

	/**
	 * Type definition for a callback that triggers state transitions
	 */
	typedef Signal::Signal
		<bool, const std::string&, StateMachine::state_t, bool> sig_t;

	/**
	 * Transition requests can only be made through this
	 */
	sig_t transition_sig;

protected:

	/**
	 * The name of this component
	 */
	const std::string
		_name;
};

#endif
