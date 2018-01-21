#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "cmd.h"

#include <memory>

/**
 * @class Task
 *
 * Represents a single task to perform while in a particular
 * state
 */
template <class R, class... T>
class Task : public Signal::Signal<R,T...>
{

public:

	/**
	 * Constructor
	 *
	 * @param [in]  name  The name of this task
	 */
	Task(const std::string& name)
		: Signal::Signal<R,T...>(), _name(name)
	{
	}

	/**
	 * Destructor
	 */
	~Task()
	{
	}

	/**
	 * Get the name of this task
	 *
	 * @return The task name
	 */
	inline std::string get_name() const
	{
		return _name;
	}

	/**
	 * Execute the task. This is generally called repeatedly
	 * while in a particular state
	 */
	inline void run()
	{
		this->raise();
	}

private:

	/**
	 * The name of this task
	 */
	std::string _name;
};

/**
 * @class State
 *
 * Represents an individual state within a StateMachine
 */
class State
{

public:

	State(const std::string& name);

	State(const State& other);

	~State();

	State& operator=( const State& rhs );

	bool add_task(Signal::generic* task);

	void execute();

	std::string get_name() const;

private:

	/**
	 * The name of this state
	 */
	std::string _name;

	/**
	 * The task(s) to run while inside of
	 * this state
	 */
	std::vector<
		std::unique_ptr<Signal::generic >
		> _tasks;
};

class StateMachineClient;

/**
 * @class StateMachine
 *
 * A simple, finite state machine that determines what the engine is
 * doing at any given time
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
		post_search = 4,

		/**
		 * Indicates the engine is exiting
		 */
		exiting     = 5,

		/**
		 * The number of states
		 */
		n_states    = 6

	} state_t;

	StateMachine(CommandInterface& cmd, Logger& logger);

	~StateMachine();

	bool add_task(state_t state, Signal::generic* task);

	void disable_logging();

	void  enable_logging();

	state_t get_current_state() const;

	bool init();

	bool register_client( StateMachineClient * client );

	bool run();

	/**
	 * A container for \ref state_t items
	 */
	typedef std::vector<state_t> state_v;

private:

	bool _acknowledge_transition();

	bool _pending_request() const;

	bool _request_transition(
		const StateMachineClient* client,
		state_t state);

	/**
	 * The record of all components registered with this
	 * state machine
	 */
	Util::str_v _clients;

	/**
	 * The command interface used to receive user inputs
	 */
	CommandInterface&
		_cmd;

	/**
	 * The state machine's current state
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
	const std::string _name;

	/**
	 * The pending state transition request
	 */
	state_t _pending_state;

	/**
	 * The states that comprise this state
	 * machine
	 */
	std::vector< std::unique_ptr<State> >
		_states;

	/**
	 * Maps from state to a set of reachable
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
		<bool,const StateMachineClient*, StateMachine::state_t> sig_t;

	/**
	 * Transition requests can only be made through this
	 */
	sig_t state_update_sig;

	/**
	 * The state machine user ID
	 */
	size_t id;

protected:

	/**
	 * The name of this component
	 */
	const std::string
		_name;
};

#endif
