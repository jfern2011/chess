#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "position2.h"
#include "protocol2.h"
#include "StateMachine2.h"

class ChessEngine
{

public:

	ChessEngine(const DataTables& tables);

	~ChessEngine();

	bool init(int cmd_fd, int log_fd,
			  protocol_t protocol=console_mode);

	bool run();

private:

	bool _is_init;

	Logger _logger;

	Position _position;

	Protocol* _protocol;

	/**
	 * The state machine, which determines what the program is
	 * doing at any given time
	 */
	StateMachine*
		_state_machine;

	const DataTables&
		_tables;
};

#endif
