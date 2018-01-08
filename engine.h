#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "protocol2.h"

/**
 * @class ChessEngine
 *
 * The main chess engine class. This runs through initialization and
 * determines what tasks to execute, which depends on the current
 * state machine state
 */
class ChessEngine
{

public:

	ChessEngine(const DataTables& tables);

	~ChessEngine();

	bool init(algorithm_t algorithm, int cmd_fd, int log_fd,
		protocol_t protocol);

	bool run();

private:

	bool _create_state_machine();

	/**
	 * Inputs received by the GUI, which are forwarded to the search
	 * algorithm
	 */
	EngineInputs* _inputs;

	/**
	 * If true, initialization succeeded
	 */
	bool _is_init;

	/**
	 * Used for logging activity
	 */
	Logger _logger;

	/**
	 * Generates captures, non-captures, checks, etc.
	 */
	MoveGen _movegen;

	/**
	 * The name of this software component
	 */
	const std::string _name;

	/**
	 * A communication protocol, used to send/receive outputs/inputs
	 * from the GUI. See \ref Protocol for details
	 */
	Protocol* _protocol;

	/**
	 * The search algorithm being used
	 */
	Search* _search;

	/**
	 * The state machine, which governs execution flow
	 */
	StateMachine* _state_machine;

	/**
	 * The global pre-computed databases used throughout the engine,
	 * such as bitmasks for generating moves
	 */
	const DataTables& _tables;
};

#endif
