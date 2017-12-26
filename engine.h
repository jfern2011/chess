#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "protocol2.h"
#include "search2.h"

/**
 * The main chess engine class. This runs through initialization and
 * determines what tasks to execute, which depends on the current
 * state machine state
 */
class ChessEngine
{

public:

	ChessEngine(const DataTables& tables);

	~ChessEngine();

	bool init(int cmd_fd, int log_fd,
			  protocol_t protocol=console_mode);

	bool run(algorithm_t algorithm);

private:

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
	 * Used for logging activity. Mostly for debugging
	 */
	Logger _logger;

	/**
	 * The communication protocol used to send/receive inputs from
	 * the GUI
	 */
	Protocol* _protocol;

	/**
	 * The state machine, which determines execution flow
	 */
	StateMachine* _state_machine;

	/**
	 * The global pre-computed databases used throughout the engine,
	 * such as bitmasks for generating moves
	 */
	const DataTables& _tables;
};

#endif
