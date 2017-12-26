#include "engine.h"
#include "position2.h"
#include "search2.h"

ChessEngine::ChessEngine(const DataTables& tables)
	: _inputs(nullptr),
	  _is_init(false),
	  _logger(),
	  _protocol(nullptr),
	  _state_machine(nullptr),
	  _tables(tables)
{
}

ChessEngine::~ChessEngine()
{
	if (_state_machine) delete _state_machine;
	if (_inputs) delete _inputs;
	if (_protocol) delete _protocol;
}

bool ChessEngine::init(int cmd_fd, int log_fd, protocol_t protocol)
{
	char msg[128];

	AbortIfNot(_logger.assign_fd(log_fd, true), false);

	_inputs = new EngineInputs(_tables, _logger);
	AbortIfNot(_inputs->init(Position(_tables, true )),
		false);

	switch (protocol)
	{
	case console_mode:
		_protocol = new Console(*_inputs, _logger);
		break;
	case uci_protocol:
		_protocol = new UCI(*_inputs, _logger);
		break;
	case xboard_protocol:
		_protocol = new xBoard(*_inputs, _logger);
		break;
	default:
		std::snprintf(msg, 128,
			"Invalid protocol ID: %d\n", protocol);
		Abort(false, msg);
	}

	AbortIfNot(_protocol->init(cmd_fd),
		false);

	_state_machine = new StateMachine(_protocol->get_cmd_interface(),
		_logger);

	AbortIfNot(_state_machine->init(), false);

	/*
	 * Inform the state machine that _protocol may initiate state
	 * transitions:
	 */
	AbortIfNot(_state_machine->register_client(_protocol->get_name(),
		_protocol), false);

	_is_init = true;
	return true;
}

bool ChessEngine::run()
{
	AbortIfNot(_is_init, false);

	MoveGen movegen(_tables);

	PvSearch pvs(movegen, _protocol->get_cmd_interface(), _logger,
		_tables);

	AbortIfNot(pvs.init(), false);

	/*
	 * Check every 100 ms for user input when idle (not searching)
	 */
	const int sleep_time = 100000;

	while (_state_machine->get_current_state()
			!= StateMachine::exiting)
	{
		AbortIfNot(_protocol->sniff(), false);

		::usleep(sleep_time);
	}

	return true;
}
