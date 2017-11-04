#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include "cmd.h"

/**
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
	 * @param[in] name The name of this task
	 */
	Task(const std::string& name)
		: _name(name)
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
		raise();
	}

private:

	/**
	 * The name of this task
	 */
	std::string _name;
};

/**
 * A single state within a finite state machine
 */
class State
{

public:

	State();
	State(const std::string& name);

	~State();

	/**
	 * Add to the list of tasks to be executed whenever we are
	 * in this state
	 *
	 * @details
	 * A task is essentially a routine that can be run in this
	 * state. Thus, the template parameters determine the
	 * signature of the function that will be invoked when the
	 * task is performed
	 *
	 * @tparam R The task's return type
	 * @tparam T Types of input arguments to the task
	 *
	 * @oaram [in] task A pointer to the task. Note this won't
	 *                  transfer ownership of the pointer
	 *
	 * @return True on success
	 */
	template <typename R, typename... T>
	bool add_task(Task<R,T...>* task)
	{
		AbortIf(task == nullptr, false);

		_tasks.push_back(task);
		return true;
	}

	std::string get_name() const;

	bool run();

private:

	/**
	 * The name of this state
	 */
	std::string _name;

	/**
	 * The set of tasks to perform while in this state
	 */
	std::vector<Signal::generic*>
				_tasks;
};

/**
 * Implements a finite state machine. The chess engine itself can be
 * in any of four states:
 *
 * 1. init
 *    This is the initialization state, which only occurs once upon
 *    program startup
 * 2. search
 *    In this state the engine runs the negamax search algorithm to
 *    compute a best move
 * 3. ponder
 *    This state is similar to the search state, except that it runs
 *    while the user is on move
 * 4. idle
 *    In this state the engine is initialized but isn't doing any
 *    computation; it simply waits for input
 */
class StateMachine
{
	typedef std::vector<state_t> state_v;

public:

	typedef enum
	{
		search,
		ponder,
		init,
		idle,
		num_states,
		undef

	} state_t;

	StateMachine();

	~StateMachine();

	/**
	 * Create a new task that will run while in a particular
	 * state
	 *
	 * @tparam R The task's return type
	 * @tparam T Types of input arguments to the task
	 *
	 * @param[in] state Add a Task to this state
	 * @param[in] task  The Task that will execute while in
	 *                  state \a state
	 *
	 * @return True on success
	 */
	template <typename R, typename... T>
	bool add_task(state_t state, Task<R,T...>* task)
	{
		AbortIfNot(state < num_states, false);

		AbortIfNot(_states[ state ].add_task( task ),
			false);

		return true;
	}

	bool build(int fd);

	static bool get_transitions(state_t state,
		state_v& transitions);

	bool request_transition(
		const std::string& state);

private:

	/**
	 * The CommandInterface used to command state transitions
	 */
	CommandInterface
		_cmd;

	/**
	 * Our current state machine state
	 */
	state_t _current_state;

	/**
	 * A mapping from state name to its enumeration
	 */
	std::map<std::string,state_t>
		_name_to_id;

	/**
	 * All states in this state machine
	 */
	std::vector<State>
		_states;
	
	/**
	 * Mapping from state to a corresponding set of reachable
	 * states
	 */
	std::vector<state_v>
		_transitions;
};

#endif
